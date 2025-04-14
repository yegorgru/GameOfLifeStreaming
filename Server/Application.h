#pragma once

#include "Config.h"
#include "IServer.h"
#include "GameOfLife.h"
#include <memory>
#include <atomic>

namespace GameOfLife::Server {

class Application {
public:
    Application();
    ~Application();
public:
    bool initialize(int argc, char* argv[]);
    void run();
    void shutdown();
private:
    void setupSignalHandling();
    bool setupServer();
private:
    using AtomicFlag = std::atomic<bool>;
    using ServerPtr = std::shared_ptr<Streaming::IServer>;
    using GameOfLifePtr = std::unique_ptr<GameOfLife>;
private:
    Config mConfig;
    ServerPtr mServer;
    AtomicFlag mRunning;
    GameOfLifePtr mGameOfLife;
};

} // namespace GameOfLife::Server
