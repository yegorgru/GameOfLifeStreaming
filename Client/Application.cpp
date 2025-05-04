#include "Application.h"
#include "StreamingFactory.h"
#include "Print.h"
#include "Log.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <numeric>  // для std::accumulate

#include "Log.h"

namespace GameOfLife::Client {

Application::Application()
    : mRunning(false)
    , mConnected(false)
    , mNewFrameReceived(false)
    , mGridWidth(20)
    , mGridHeight(20)
    , mCellSize(20) {}

Application::~Application() {
    shutdown();
}

bool Application::initialize(int argc, char* argv[]) {
    Print::initConsolePrinter();

    if (!mConfig.parseCommandLine(argc, argv)) {
        return false;
    }

    const auto logFilename = mConfig.getLogFilename();
    const auto logLevel = mConfig.getLogLevel();
    if (logFilename.empty()) {
        Log::initConsoleLogger(logLevel);
    } else {
        Log::initFileLogger(logLevel, logFilename);
    }
    Print::initConsolePrinter(); 

    Log::Debug("Logger and Printer initialized.");

    mCellSize = mConfig.getCellSize();

    Print::PrintLine("Game of Life Client initializing...");
    Log::Info("Game of Life Client initializing...");

    if (!setupClient()) {
        Print::PrintLine("Failed to initialize streaming client", std::cerr);
        Log::Throw("Failed to initialize streaming client");
        return false;
    }

    if (!initWindow()) {
        Print::PrintLine("Failed to initialize Raylib window", std::cerr);
        Log::Throw("Failed to initialize Raylib window");
        return false;
    }

    mRunning = true;
    Log::Info("Client initialization successful.");
    return true;
}

void Application::run() {
    if (!mRunning) {
        Print::PrintLine("Cannot run client - not properly initialized", std::cerr);
        Log::Error("Attempted to run client when not initialized.");
        return;
    }

    Print::PrintLine("Client running...");
    Log::Info("Client main loop started.");
    gameLoop();
    Print::PrintLine("Client main loop exited.");
    Log::Info("Client main loop finished.");
}

void Application::shutdown() {
    if (mRunning) {
        Print::PrintLine("Shutting down client...");
        mRunning = false;

        for (auto& client : mClients) {
            if (client) {
                client->disconnect();
                client.reset();
            }
        }

        if (IsWindowReady()) {
            CloseWindow();
        }
        Print::PrintLine("Client shutdown complete.");
    }
}

bool Application::setupClient() {
    try {
        for (uint64_t i = 0; i < clientsNumber; ++i) {
            mClients.push_back(Streaming::StreamingFactory::CreateClient());
        }
        setupCallbacks();

        Print::PrintLine(Print::composeMessage("Connecting via multicast group ", mConfig.getMulticastAddress(), " on port ", mConfig.getServerPort()));
        for (size_t i = 0; i < mClients.size(); ++i) {
            Print::PrintLine(Print::composeMessage("Connecting client ", i + 1, " of ", mClients.size(), "..."));
            if (!mClients[i]->connect(mConfig.getMulticastAddress(), mConfig.getServerPort())) {
                Print::PrintLine("Failed to join multicast group!", std::cerr);
                return false;
            }
        }

        for (size_t i = 0; i < mClients.size(); ++i) {
            Print::PrintLine(Print::composeMessage("Start Receiving ", i + 1, " of ", mClients.size(), "..."));
            mClients[i]->startReceive();
        }

        return true;
    }
    catch (const std::exception& e) {
        Print::PrintLine(Print::composeMessage("Exception during client setup: ", e.what()), std::cerr);
        return false;
    }
}

void Application::setupCallbacks() {
    mLatencyHistory.resize(MAX_LATENCY_HISTORY);
    auto dataReceived = [this](const std::string& data) {
        try {
            long long receivedTimestampMs = std::stoll(data);
            
            auto now = std::chrono::system_clock::now();
            auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
            long long latency = nowMs - receivedTimestampMs;

            uint64_t currentCountCopy = ++mCurrentCount;
            mLatencyHistory[currentCountCopy % MAX_LATENCY_HISTORY] = latency;
            if (currentCountCopy % 1000 == 0) {
                Print::PrintLine(Print::composeMessage("Latency history size: ", currentCountCopy));
            }
            if (currentCountCopy % MAX_LATENCY_HISTORY == 0) {
                Print::PrintLine(Print::composeMessage("Latency history size exceeded ", MAX_LATENCY_HISTORY, ", removing oldest entry."));

                long long sum = std::accumulate(mLatencyHistory.begin(), mLatencyHistory.end(), 0LL);
                mAverageLatency = static_cast<double>(sum) / MAX_LATENCY_HISTORY;
                
                Print::PrintLine(Print::composeMessage("Average latency: ", mAverageLatency, " ms (", MAX_LATENCY_HISTORY, " samples)"));
            }
            
        } catch (const std::exception& e) {
            Print::PrintLine(Print::composeMessage("Error parsing timestamp: ", e.what()), std::cerr);
        }
    };
    auto connectedCallback = [this]() {
        Print::PrintLine("Connected to server!");
        mConnected = true;
    };

    auto disconnectedCallback = [this]() {
        Print::PrintLine("Disconnected from server!");
        mConnected = false;
    };

    for (const auto& client : mClients) {
        client->setOnDataReceived(dataReceived);
        client->setOnConnected(connectedCallback);
        client->setOnDisconnected(disconnectedCallback);
    }
}

bool Application::initWindow() {
    InitWindow(mGridWidth * mCellSize, mGridHeight * mCellSize, "Game of Life Client");
    if (!IsWindowReady()) {
        return false;
    }
    SetTargetFPS(mConfig.getTargetFps());
    return true;
}

void Application::gameLoop() {
    while (mRunning && !WindowShouldClose()) {
        updateWindowState();
        renderCurrentFrame();
    }
    if (WindowShouldClose()) {
        Log::Error("Window closed by user.");
    }
    shutdown();
}

void Application::updateWindowState() {
    bool updateFrame = false;
    std::string currentFrameCopy;

    if (mNewFrameReceived.exchange(false)) {
        std::lock_guard<std::mutex> lock(mFrameMutex);
        currentFrameCopy = mLatestFrame;
        updateFrame = true;
    }

    if (updateFrame && !currentFrameCopy.empty()) {
        auto [width, height] = getGridDimensions(currentFrameCopy);
        if (width > 0 && height > 0 && (width != mGridWidth || height != mGridHeight)) {
            mGridWidth = width;
            mGridHeight = height;
            SetWindowSize(mGridWidth * mCellSize, mGridHeight * mCellSize);
        }
    }
}

void Application::renderCurrentFrame() {
    BeginDrawing();
    ClearBackground(BLACK);

    std::string frameToRender;
    {
        std::lock_guard<std::mutex> lock(mFrameMutex);
        frameToRender = mLatestFrame;
    }

    if (!frameToRender.empty()) {
        renderFrame(frameToRender);
    } else if (!mConnected) {
        DrawText("Connecting to server...", 20, 20, 20, GRAY);
    } else {
        DrawText("Waiting for data from server...", 20, 20, 20, GRAY);
    }

    DrawFPS(10, 10);
    EndDrawing();
}

Application::GridSize Application::getGridDimensions(const std::string& frame) {
    if (frame.length() < 7 || frame[3] != 'x') {
        return {0, 0};
    }

    std::string header = frame.substr(0, 7);
    try {
        int parsedWidth = std::stoi(header.substr(0, 3));
        int parsedHeight = std::stoi(header.substr(4));
        if (parsedWidth > 0 && parsedHeight > 0 && parsedWidth < 2000 && parsedHeight < 2000) {
            return {parsedWidth, parsedHeight};
        }
    } catch (const std::invalid_argument&) {
        Print::PrintLine(Print::composeMessage("Invalid number in header: ", header), std::cerr);
    }

    Print::PrintLine(Print::composeMessage("Failed to parse header: ", header), std::cerr);
    return {0, 0};
}

void Application::renderFrame(const std::string& frame) {
    const size_t headerLength = 7;
    
    if (mGridWidth <= 0 || mGridHeight <= 0) {
         Print::PrintLine("Cannot render frame: Invalid grid dimensions.", std::cerr);
         return;
    }

    size_t expectedDataLength = static_cast<size_t>(mGridWidth) * mGridHeight;
    size_t expectedTotalLength = headerLength + expectedDataLength;

    if (frame.length() < expectedTotalLength) {
        Print::PrintLine(Print::composeMessage("Frame data too short. Expected: ", expectedTotalLength, ", Got: ", frame.length()), std::cerr);
        return; 
    }

    for (size_t i = 0; i < expectedDataLength; ++i) {
        size_t frameIndex = headerLength + i;
        char c = frame[frameIndex];

        int x = static_cast<int>(i % mGridWidth);
        int y = static_cast<int>(i / mGridWidth);

        bool isAlive = (c == '#');

        Rectangle cellRect = {
            static_cast<float>(x * mCellSize),
            static_cast<float>(y * mCellSize),
            static_cast<float>(mCellSize),
            static_cast<float>(mCellSize)
        };

        if (isAlive) {
            DrawRectangleRec(cellRect, GREEN);
            DrawRectangleLinesEx(cellRect, 1.0f, DARKGREEN);
        } else {
            DrawRectangleLinesEx(cellRect, 0.5f, DARKGRAY);
        }
    }
}

} // namespace GameOfLife::Client
