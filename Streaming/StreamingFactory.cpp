// #include "StreamingFactory.h"

// // Include the appropriate implementation based on compile-time options
// #if defined(USE_ASIO)
// #include "Asio/Client.h"
// #include "Asio/Server.h"
// #elif defined(USE_BEAST)
// #include "Beast/BeastClient.h"
// #include "Beast/BeastServer.h"
// #elif defined(USE_POCO)
// #include "Poco/PocoClient.h"
// #include "Poco/PocoServer.h"
// #else
// #error "No streaming implementation selected"
// #endif

// namespace Streaming {

// ClientPtr StreamingFactory::CreateClient() {
// #if defined(USE_ASIO)
//     return std::make_shared<Asio::AsioClient>();
// #elif defined(USE_BEAST)
//     return std::make_shared<Beast::BeastClient>();
// #elif defined(USE_POCO)
//     return std::make_shared<Poco::PocoClient>();
// #endif
// }

// ServerPtr StreamingFactory::CreateServer() {
// #if defined(USE_ASIO)
//     return std::make_shared<Asio::AsioServer>();
// #elif defined(USE_BEAST)
//     return std::make_shared<Beast::BeastServer>();
// #elif defined(USE_POCO)
//     return std::make_shared<Poco::PocoServer>();
// #endif
// }

// std::string StreamingFactory::GetImplementationName() {
// #if defined(USE_ASIO)
//     return "Boost.Asio";
// #elif defined(USE_BEAST)
//     return "Boost.Beast";
// #elif defined(USE_POCO)
//     return "POCO";
// #else
//     return "Unknown";
// #endif
// }

// } // namespace Streaming
