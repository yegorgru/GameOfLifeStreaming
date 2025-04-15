#pragma once

#include "IClient.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <memory>
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>

namespace Streaming::Beast {

class BeastClient : public IClient {
public:
    BeastClient();
    ~BeastClient() override;
public:
    bool connect(const std::string& serverAddress, int port) override;
    void disconnect() override;
    bool isConnected() const override;
    void setOnConnected(ConnectionCallback callback) override;
    void setOnDisconnected(ConnectionCallback callback) override;
    void setOnDataReceived(DataCallback callback) override;
private:
    void doRead();
    void doClose();
    void fail(boost::system::error_code ec, const std::string& what);
private:
    using IoContext = boost::asio::io_context;
    using WorkGuard = boost::asio::executor_work_guard<IoContext::executor_type>;
    using OptionalWorkGuard = std::optional<WorkGuard>;
    using Socket = boost::asio::ip::tcp::socket;
    using Resolver = boost::asio::ip::tcp::resolver;
    using ResolverPtr = std::unique_ptr<Resolver>;
    using WebSocket = boost::beast::websocket::stream<boost::beast::tcp_stream>;
    using WebSocketPtr = std::unique_ptr<WebSocket>;
    using Buffer = boost::beast::flat_buffer;
    using AtomicFlag = std::atomic<bool>;
private:
    std::string mServerAddress;
    int mServerPort;
    IoContext mIoContext;
    OptionalWorkGuard mWork;
    WebSocketPtr mWebSocket;
    ResolverPtr mResolver;
    Buffer mBuffer;
    std::string mFrameBuffer;
    std::jthread mThread;
    AtomicFlag mRunning;
    AtomicFlag mConnected;
    std::mutex mMutex;
    
    ConnectionCallback mOnConnected;
    ConnectionCallback mOnDisconnected;
    DataCallback mOnDataReceived;
};

} // namespace Streaming::Beast
