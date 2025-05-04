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
    void setOnConnected(ConnectionCallback callback) override;
    void setOnDisconnected(ConnectionCallback callback) override;
    void setOnDataReceived(DataCallback callback) override;
    bool isConnected() const override;
    void startReceive() override;
private:
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
    inline static AtomicFlag mIsFirstTime = true;
    static IoContext mIoContext;
    std::vector<std::jthread> mThreads;
    static WorkGuardOptional mWork;
    Socket mSocket;
    udp::endpoint mSenderEndpoint;
    Buffer mReceiveBuffer;
    Buffer mFrameBuffer;
    AtomicFlag mRunning;
    AtomicFlag mConnected;
    std::string mMulticastAddress;
private:
    ConnectionCallback mOnConnected;
    ConnectionCallback mOnDisconnected;
    DataCallback mOnDataReceived;
};

} // namespace Streaming::Asio
