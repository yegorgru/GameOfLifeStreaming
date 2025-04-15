#pragma once

#include <atomic>
#include <memory>
#include <functional>

#include <boost/asio.hpp>

#include "Log.h"

namespace Streaming::Beast {

class Acceptor {
public:
    using IoContext = boost::asio::io_context;
    using TcpSocket = boost::asio::ip::tcp::socket;
    using TcpSocketPtr = std::shared_ptr<TcpSocket>;
    using Callback = std::function<void(TcpSocketPtr)>;
public:
    Acceptor(IoContext& ios, Callback callback);
    ~Acceptor() = default;
public:
    void start(const std::string& rawIp, unsigned short portNum);
    void stop();
private:
    void initAccept();
private:
    using TcpAcceptor = boost::asio::ip::tcp::acceptor;
    using TcpAcceptorPtr = std::unique_ptr<TcpAcceptor>;
    using AtomicFlag = std::atomic<bool>;
private:
    IoContext& mIos;
    TcpAcceptorPtr mAcceptor;
    TcpSocketPtr mNextSocket;
    AtomicFlag mIsStopped;
    Callback mCallback;
};

} // namespace Streaming::Beast