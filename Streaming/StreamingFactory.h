#pragma once

#include "IClient.h"
#include "IServer.h"
#include <memory>
#include <string>

namespace Streaming {

class StreamingFactory {
public:
    static ClientPtr CreateClient();
    static ServerPtr CreateServer();
};

} // namespace Streaming
