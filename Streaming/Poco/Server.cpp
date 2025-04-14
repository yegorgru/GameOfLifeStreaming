#include "Server.h"
#include "Log.h"
#include "Print.h"
#include "ThreadPoolManager.h"

#include <Poco/Net/NetException.h>

namespace Streaming::Poco {

PocoServer::PocoServer()
    : mRunning(false)
{
    ThreadPoolManager::Init();
    Log::Debug("PocoServer created.");
}

PocoServer::~PocoServer() {
    Log::Debug("PocoServer destroying...");
    stop();
    Log::Debug("PocoServer destroyed.");
}

bool PocoServer::start(const std::string& multicastAddress, int port, int threadCount) {
    if (mRunning) {
        Log::Warning("PocoServer::start called but server is already running.");
        return true;
    }

    Log::Info(Print::composeMessage("Attempting to start PocoServer on port ", port, " with multicast ", multicastAddress));

    namespace PocoNet = ::Poco::Net;
    try {
        PocoNet::IPAddress ipAddr = PocoNet::IPAddress::parse(multicastAddress);
        if (!ipAddr.isMulticast()) {
            Log::Error(Print::composeMessage("Provided address is not a multicast address: ", multicastAddress));
            return false;
        }
        mMulticastAddress = PocoNet::SocketAddress(ipAddr, port);
        mSocket = std::make_shared<PocoNet::MulticastSocket>(PocoNet::SocketAddress::IPv4);
        mSocket->setReuseAddress(true);
        // mSocket->setLoopback(false);
        // mSocket->setTimeToLive(1);

        ThreadPoolManager::Get().start(1, threadCount > 0 ? threadCount : 4);

        mRunning = true;
        Log::Info(Print::composeMessage("PocoServer started successfully. Multicast target: ", mMulticastAddress.toString()));
        return true;

    } catch (const PocoNet::NetException& e) {
        Log::Error(Print::composeMessage("Poco NetException during server start: ", e.displayText()));
        stop();
        return false;
    } catch (const ::Poco::Exception& e) {
        Log::Error(Print::composeMessage("Poco Exception during server start: ", e.displayText()));
        stop();
        return false;
    } catch (const std::exception& e) {
        Log::Error(Print::composeMessage("Standard exception during server start: ", e.what()));
        stop();
        return false;
    }
}

void PocoServer::stop() {
    if (!mRunning) {
        return;
    }
    Log::Info("Stopping PocoServer...");
    mRunning = 0;

    ThreadPoolManager::Destroy();

    try {
        if (mSocket) {
            mSocket->close(); 
            mSocket.reset();
            Log::Debug("PocoServer socket closed and reset.");
        }
    } catch (const ::Poco::Exception& e) {
        Log::Error(Print::composeMessage("Poco Exception during server stop: ", e.displayText()));
    } catch (const std::exception& e) {
        Log::Error(Print::composeMessage("Standard exception during server stop: ", e.what()));
    }
    Log::Info("PocoServer stopped.");
}

bool PocoServer::isRunning() const {
    return mRunning;
}

void PocoServer::broadcastData(const std::string& data) {
    if (!mRunning || !mSocket) {
        return;
    }

    ThreadPoolManager::Get().enqueue([socket = mSocket, targetAddress = mMulticastAddress, dataCopy = data]() {
        try {
            if (!socket || !socket->impl() || !socket->impl()->initialized()) {
                 Log::Warning("Broadcast task skipped: Socket is closed or invalid.");
                 return;
            }

            int bytesSent = socket->sendTo(dataCopy.data(), static_cast<int>(dataCopy.length()), targetAddress);
            if (bytesSent != static_cast<int>(dataCopy.length())) {
                Log::Warning(Print::composeMessage("Could not send complete UDP packet async. Expected: ", dataCopy.length(), ", Sent: ", bytesSent));
            }
        } catch (const ::Poco::Net::NetException& e) {
            if (e.code() != POCO_ENETRESET && e.code() != POCO_ESHUTDOWN && e.code() != POCO_ECONNABORTED) {
                Log::Error(Print::composeMessage("Poco NetException during async broadcast: ", e.displayText()));
            }
        } catch (const ::Poco::Exception& e) {
            Log::Error(Print::composeMessage("Poco Exception during async broadcast: ", e.displayText()));
        } catch (const std::exception& e) {
            Log::Error(Print::composeMessage("Standard exception during async broadcast: ", e.what()));
        }
    });
}

} // namespace Streaming::Poco
