#pragma once

#include "IStreamingClient.h"
#include "IStreamingServer.h"
#include <memory>
#include <string>

namespace Streaming {

// Factory class to create the appropriate network implementation
class StreamingFactory {
public:
    // Create a client implementation based on the selected library
    static ClientPtr CreateClient();
    
    // Create a server implementation based on the selected library
    static ServerPtr CreateServer();
    
    // Get the name of the currently selected network library
    static std::string GetImplementationName();
};

} // namespace Streaming
