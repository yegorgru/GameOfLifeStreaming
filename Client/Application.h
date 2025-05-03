#pragma once

#include "Config.h"
#include "IClient.h"
#include <raylib.h>
#include <memory>
#include <string>
#include <mutex>
#include <atomic>
#include <deque>  // для збереження історії латентності

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
    std::vector<ClientPtr> mClients;
    AtomicFlag mRunning;
    AtomicFlag mConnected;
    AtomicFlag mNewFrameReceived;
    std::mutex mFrameMutex;
    std::string mLatestFrame;
    int mGridWidth;
    int mGridHeight;
    int mCellSize;
      // Для вимірювання латентності мережі
    std::deque<long long> mLatencyHistory;
    double mAverageLatency = 0.0;
    uint64_t clientsNumber = 15000;
    uint64_t MAX_LATENCY_HISTORY = 15000 * 1000;
    std::atomic<uint64_t> mCurrentCount = 0;
};

} // namespace GameOfLife::Client
