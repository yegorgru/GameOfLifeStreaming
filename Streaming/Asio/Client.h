#pragma once

#include "IClient.h"
#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include <optional>

namespace Streaming::Asio {

class AsioClient : public IClient {
public:
    AsioClient();
    ~AsioClient() override;
public:
    bool connect(const std::string& multicastAddress, int port) override;
    void disconnect() override;
    void setOnConnected(std::function<void()> callback) override;
    void setOnDisconnected(std::function<void()> callback) override;
    void setOnDataReceived(std::function<void(const std::string&)> callback) override;
    bool isConnected() const override;
private:
    void startReceive();
    void handleReceive(const boost::system::error_code& error, std::size_t bytesReceived);
private:
    using udp = boost::asio::ip::udp;
    using IoContext = boost::asio::io_context;
    using WorkGuard = boost::asio::executor_work_guard<IoContext::executor_type>;
    using WorkGuardOptional = std::optional<WorkGuard>;
    using Socket = udp::socket;
    using AtomicFlag = std::atomic<bool>;
    using Buffer = std::string;
private:
    IoContext mIoContext;
    Socket mSocket;
    udp::endpoint mSenderEndpoint;
    Buffer mReceiveBuffer;
    Buffer mFrameBuffer;
    WorkGuardOptional mWork;
    std::jthread mThread;
    AtomicFlag mRunning;
    AtomicFlag mConnected;
    std::string mMulticastAddress;
private:
    ConnectionCallback mOnConnected;
    ConnectionCallback mOnDisconnected;
    DataCallback mOnDataReceived;
};

} // namespace Streaming::Asio
