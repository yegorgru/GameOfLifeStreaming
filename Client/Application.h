#pragma once

#include "Config.h"
#include "IClient.h"
#include <raylib.h>
#include <memory>
#include <string>
#include <mutex>
#include <atomic>

namespace GameOfLife::Client {

class Application {
public:
    Application();
    ~Application();
public:
    bool initialize(int argc, char* argv[]);
    void run();
    void shutdown();
private:
    bool setupClient();
    void setupCallbacks();
    bool initWindow();
    void gameLoop();
    void updateWindowState();
    void renderCurrentFrame();
    void renderFrame(const std::string& frame);
private:
    using GridSize = std::pair<int, int>;
    GridSize getGridDimensions(const std::string& frame);
private:
    using ClientPtr = std::unique_ptr<Streaming::IClient>;
    using AtomicFlag = std::atomic<bool>;
private:
    Config mConfig;
    ClientPtr mClient;
    AtomicFlag mRunning;
    AtomicFlag mConnected;
    AtomicFlag mNewFrameReceived;
    std::mutex mFrameMutex;
    std::string mLatestFrame;
    int mGridWidth;
    int mGridHeight;
    int mCellSize;
};

} // namespace GameOfLife::Client
