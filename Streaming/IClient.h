#pragma once

#include <string>
#include <functional>
#include <memory>

namespace Streaming {

class IClient {
public:
    using ConnectionCallback = std::function<void()>;
    using DataCallback = std::function<void(const std::string&)>;
public:
    virtual ~IClient() = default;
public:
    virtual bool connect(const std::string& multicastAddress, int port) = 0;
    virtual void disconnect() = 0;
    virtual void setOnConnected(ConnectionCallback callback) = 0;
    virtual void setOnDisconnected(ConnectionCallback callback) = 0;
    virtual void setOnDataReceived(DataCallback callback) = 0;
    virtual bool isConnected() const = 0;
};

using ClientPtr = std::unique_ptr<IClient>;

} // namespace Streaming
