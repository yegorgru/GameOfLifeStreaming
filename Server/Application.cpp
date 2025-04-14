#include "Application.h"
#include "Log.h"
#include "../Streaming/StreamingFactory.h"
#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>

namespace GameOfLife::Server {

namespace {
    std::atomic<bool> gShutdownRequested(false);
}

static void signalHandler(int signal) {
    Log::Info("Received signal " + std::to_string(signal) + ", initiating shutdown...");
    gShutdownRequested = true;
}

Application::Application()
    : mRunning(false) {
}

Application::~Application() {
    shutdown();
}

bool Application::initialize(int argc, char* argv[]) {
    if (!mConfig.parseCommandLine(argc, argv)) {
        return false;
    }

    if (!mConfig.getLogFilename().empty()) {
        Log::initFileLogger(mConfig.getLogLevel(), mConfig.getLogFilename());
    }
    else {
        Log::initConsoleLogger(mConfig.getLogLevel());
    }

    Log::Info("Game of Life Server initializing...");
    setupSignalHandling();

    if (!setupServer()) {
        Log::Error("Failed to initialize streaming server");
        return false;
    }

    mRunning = true;
    return true;
}

void Application::run() {
    if (!mRunning || !mServer) {
        Log::Error("Cannot run server - not properly initialized");
        return;
    }

    Log::Info("Server running on port " + std::to_string(mConfig.getPort()));
    Log::Info("Press Ctrl+C to stop the server");

    auto [width, height] = mConfig.getGridSize();
    int fps = mConfig.getFps();
    int frameDelayMs = 1000 / fps;
    Log::Info("Game of Life grid size: " + std::to_string(width) + "x" + std::to_string(height) + ", " + std::to_string(fps) + " FPS");

    mGameOfLife = std::make_unique<GameOfLife>(width, height);
    mGameOfLife->initializeRandom(mConfig.getFillRatio());

    while (mRunning && !gShutdownRequested) {
        mGameOfLife->update();        
        std::string asciiFrame = mGameOfLife->toString();
        mServer->broadcastData(asciiFrame);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(frameDelayMs));
    }

    Log::Info("Server main loop exited");
}

void Application::shutdown() {
    if (mRunning) {
        Log::Info("Shutting down server...");        
        mRunning = false;

        if (mServer) {
            mServer->stop();
            mServer.reset();
        }

        Log::Info("Server shutdown complete");
    }
}

void Application::setupSignalHandling() {
    std::signal(SIGINT, signalHandler);  // Handle Ctrl+C
    std::signal(SIGTERM, signalHandler); // Handle termination request
}

bool Application::setupServer() {    
    try {
        mServer = Streaming::StreamingFactory::CreateServer();
        
        Log::Info("Starting server on port " + std::to_string(mConfig.getPort()) + " with multicast " + mConfig.getMulticastAddress());
        
        if (!mServer->start(mConfig.getMulticastAddress(), mConfig.getPort())) {
            Log::Error("Failed to start server on port " + std::to_string(mConfig.getPort()));
            return false;
        }
        
        return true;
    }
    catch (const std::exception& e) {
        Log::Error("Exception during server setup: " + std::string(e.what()));
        return false;
    }
}

} // namespace GameOfLife::Server
