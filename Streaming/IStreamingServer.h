#pragma once

#include "IStreamingConnection.h"
#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace Streaming {

// Interface for the streaming server
class IStreamingServer {
public:
    virtual ~IStreamingServer() = default;

    // Start the server on the specified port
    virtual bool Start(int port) = 0;

    // Stop the server
    virtual void Stop() = 0;

    // Send data to all connected clients
    virtual void BroadcastData(const std::string& data) = 0;

    // Register callback for when a client connects
    virtual void SetOnClientConnected(std::function<void(ConnectionPtr)> callback) = 0;

    // Register callback for when a client disconnects
    virtual void SetOnClientDisconnected(std::function<void(ConnectionPtr)> callback) = 0;

    // Register callback for when data is received from a client
    virtual void SetOnDataReceived(std::function<void(ConnectionPtr, const std::string&)> callback) = 0;

    // Get all currently connected clients
    virtual std::vector<ConnectionPtr> GetConnectedClients() const = 0;

    // Check if the server is running
    virtual bool IsRunning() const = 0;
};

// Alias for shared server pointer
using ServerPtr = std::shared_ptr<IStreamingServer>;

} // namespace Streaming
