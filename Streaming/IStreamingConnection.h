#pragma once

#include <string>
#include <functional>
#include <memory>

namespace Streaming {

// Forward declarations
class IStreamingClient;
class IStreamingServer;

// Interface for a network connection
class IStreamingConnection {
public:
    virtual ~IStreamingConnection() = default;

    // Send data through the connection
    virtual void SendData(const std::string& data) = 0;

    // Close the connection
    virtual void Close() = 0;

    // Check if connection is still active
    virtual bool IsConnected() const = 0;

    // Get remote endpoint information (address:port)
    virtual std::string GetRemoteEndpoint() const = 0;
};

// Alias for connection handler callback
using ConnectionHandler = std::function<void(const std::string&)>;

// Alias for shared connection pointer
using ConnectionPtr = std::shared_ptr<IStreamingConnection>;

} // namespace Streaming
