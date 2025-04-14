#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace Streaming {

class IServer 
{
public:    
    virtual ~IServer() = default;
public:
    virtual bool start(const std::string& multicastAddress, int port, int threadCount = 1) = 0;
    virtual void stop() = 0;
    virtual void broadcastData(const std::string& data) = 0;
    virtual bool isRunning() const = 0;
};

using ServerPtr = std::unique_ptr<IServer>;

} // namespace Streaming
