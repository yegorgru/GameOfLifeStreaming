#include "StreamingFactory.h"

#if defined(USE_ASIO)
#include "Asio/Client.h"
#include "Asio/Server.h"
#elif defined(USE_BEAST)
#include "Beast/Client.h"
#include "Beast/Server.h"
#elif defined(USE_POCO)
#include "Poco/Client.h"
#include "Poco/Server.h"
#else
#error "No streaming implementation selected"
#endif

namespace Streaming {

ClientPtr StreamingFactory::CreateClient() {
#if defined(USE_ASIO)
    return std::make_unique<Asio::AsioClient>();
#elif defined(USE_BEAST)
    return std::make_unique<Beast::BeastClient>();
#elif defined(USE_POCO)
    return std::make_unique<Poco::PocoClient>();
#endif
}

ServerPtr StreamingFactory::CreateServer() {
#if defined(USE_ASIO)
    return std::make_unique<Asio::AsioServer>();
#elif defined(USE_BEAST)
    return std::make_unique<Beast::BeastServer>();
#elif defined(USE_POCO)
    return std::make_unique<Poco::PocoServer>();
#endif
}

} // namespace Streaming
