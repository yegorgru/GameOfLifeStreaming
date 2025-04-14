#pragma once

#include "IServer.h"
#include <Poco/Net/MulticastSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <string>
#include <memory>

namespace Streaming::Poco {

class PocoServer : public IServer {
public:
    PocoServer();
    ~PocoServer() override;
public:
    bool start(const std::string& multicastAddress, int port, int threadCount) override;
    void stop() override;
    bool isRunning() const override;
    void broadcastData(const std::string& data) override;
private:
    using MulticastSocket = ::Poco::Net::MulticastSocket;
    using SocketPtr = std::shared_ptr<MulticastSocket>;
    using SocketAddress = ::Poco::Net::SocketAddress;
    using AtomicFlag = std::atomic<bool>;
private:
    SocketPtr mSocket; 
    SocketAddress mMulticastAddress;
    AtomicFlag mRunning;
};

} // namespace Streaming::Poco
