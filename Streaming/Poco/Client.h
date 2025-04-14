#pragma once

#include "IClient.h"
#include <Poco/Net/MulticastSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/NetException.h>
#include <Poco/Timespan.h>

#include <string>
#include <memory>
#include <thread>
#include <stop_token>

namespace Streaming::Poco {

class PocoClient : public IClient {
public:
    PocoClient();
    ~PocoClient() override;
public:
    bool connect(const std::string& multicastAddress, int port) override;
    void disconnect() override;
    void setOnConnected(std::function<void()> callback) override;
    void setOnDisconnected(std::function<void()> callback) override;
    void setOnDataReceived(std::function<void(const std::string&)> callback) override;
    bool isConnected() const override;
private:
    void receiveLoop(std::stop_token stopToken);
    void handleReceivedData(int length);
private:
    using MulticastSocket = ::Poco::Net::MulticastSocket;
    using SocketPtr = std::unique_ptr<MulticastSocket>;
    using SocketAddress = ::Poco::Net::SocketAddress;
    using AtomicFlag = std::atomic<bool>;
private:
    SocketPtr mSocket;
    SocketAddress mMulticastGroupAddress;
    SocketAddress mSenderAddress;

    std::string mReceiveBuffer;
    std::string mFrameBuffer;

    AtomicFlag mRunning;
    AtomicFlag mConnected;
    std::jthread mReceiveThread;

private:
    std::function<void()> mOnConnected;
    std::function<void()> mOnDisconnected;
    std::function<void(const std::string&)> mOnDataReceived;
};

} // namespace Streaming::Poco
