#pragma once

#include "IStreamingConnection.h"
#include <string>
#include <functional>
#include <memory>

namespace Streaming {

// Interface for the streaming client
class IStreamingClient {
public:
    virtual ~IStreamingClient() = default;

    // Connect to a server at the specified host and port
    virtual bool Connect(const std::string& host, int port) = 0;

    // Disconnect from the server
    virtual void Disconnect() = 0;

    // Send data to the server
    virtual void SendData(const std::string& data) = 0;

    // Register callback for when connected to server
    virtual void SetOnConnected(std::function<void()> callback) = 0;

    // Register callback for when disconnected from server
    virtual void SetOnDisconnected(std::function<void()> callback) = 0;

    // Register callback for when data is received from server
    virtual void SetOnDataReceived(std::function<void(const std::string&)> callback) = 0;

    // Check if connected to server
    virtual bool IsConnected() const = 0;

    // Get the underlying connection if connected
    virtual ConnectionPtr GetConnection() const = 0;
};

// Alias for shared client pointer
using ClientPtr = std::shared_ptr<IStreamingClient>;

} // namespace Streaming
