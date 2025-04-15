#pragma once

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <set>
#include <mutex>
#include <atomic>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include "IServer.h"
#include "Session.h"
#include "Acceptor.h"

namespace Streaming::Beast {

class BeastServer : public IServer {
public:
    using SessionPtr = std::shared_ptr<Session>;
public:
    BeastServer();
    ~BeastServer() override;
public:
    bool start(const std::string& address, int port, int threadCount) override;
    void stop() override;
    void broadcastData(const std::string& data) override;
    bool isRunning() const override;
public:
    void addSession(SessionPtr session);
    void removeSession(SessionPtr session);
private:
    void onAccept(Acceptor::TcpSocketPtr socketPtr);
private:
    using IoContext = boost::asio::io_context;
    using SessionStorage = std::set<SessionPtr>;
    using AcceptorPtr = std::unique_ptr<Acceptor>;
    using ThreadPool = std::vector<std::jthread>;
    using AtomicFlag = std::atomic<bool>;
    using WorkOptional = std::optional<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>;
private:
    IoContext mIoContext;
    WorkOptional mWork;
    SessionStorage mSessions;
    std::mutex mSessionsMutex;
    AtomicFlag mRunning;
    AcceptorPtr mAcceptor;
    ThreadPool mThreadPool;
};

} // namespace Streaming::Beast
