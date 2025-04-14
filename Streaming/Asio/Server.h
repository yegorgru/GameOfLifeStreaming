#pragma once

#include "IServer.h"
#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>

namespace Streaming::Asio {

class AsioServer : public IServer {
public:    
    AsioServer();
    ~AsioServer() override;
public:
    bool start(const std::string& multicastAddress, int port, int threadCount = 1) override;
    void stop() override;
    bool isRunning() const override;
    void broadcastData(const std::string& data) override;
private:
    void setupMulticast(const std::string& multicastAddress, int port);
private:
    using WorkGuard = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    using WorkGuardOptional = std::optional<WorkGuard>;
    using ThreadPool = std::vector<std::jthread>;
    using AtomicFlag = std::atomic<bool>;
    using MulticastEndpoint = boost::asio::ip::udp::endpoint;
    using SocketPtr = std::unique_ptr<boost::asio::ip::udp::socket>;
    using IoContext = boost::asio::io_context;
private:
    IoContext mIoContext;
    SocketPtr mSocket;
    MulticastEndpoint mMulticastEndpoint;
    WorkGuardOptional mWork;
    ThreadPool mThreadPool;
    AtomicFlag mRunning;
};

} // namespace Streaming::Asio
