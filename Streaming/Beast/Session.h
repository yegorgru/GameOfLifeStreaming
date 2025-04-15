#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <memory>

namespace Streaming::Beast {

class BeastServer; 
class Session;
using SessionPtr = std::shared_ptr<Session>;

class Session : public std::enable_shared_from_this<Session> {
public:
    using TcpSocket = boost::asio::ip::tcp::socket;
public:
    Session(BeastServer& server, TcpSocket&& socket);
    ~Session();
public:
    void run();
    void send(const std::string& message);
    void close();
private:
    void read();
    void write();
    void fail(boost::beast::error_code ec, const std::string& message);
private:
    using FlatBuffer = boost::beast::flat_buffer;
    using WebSocketStream = boost::beast::websocket::stream<boost::beast::tcp_stream>;
    using WriteQueue = std::queue<std::string>;
    using AtomicFlag = std::atomic<bool>;
private:
    BeastServer& mServer;
    WebSocketStream mStream;
    FlatBuffer mBuffer;
    AtomicFlag mIsWriting;
    WriteQueue mWriteQueue;
    std::mutex mQueueMutex;
    SessionPtr mSelfPtr;
    AtomicFlag mIsClosing;
};

} // namespace Streaming::Beast
