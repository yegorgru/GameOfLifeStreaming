#include "Client.h"
#include "Log.h"
#include "Print.h"
#include <stop_token>

namespace Streaming::Poco {

namespace {
    const size_t MAX_BUFFER_SIZE = 4096;
    const char FRAME_DELIMITER = '\n';
}

PocoClient::PocoClient()
    : mRunning(false)
    , mConnected(false)
    , mReceiveBuffer(MAX_BUFFER_SIZE, ' ')
{
    Log::Debug("PocoClient created.");
}

PocoClient::~PocoClient() {
    Log::Debug("PocoClient destroying...");
    disconnect();
    Log::Debug("PocoClient destroyed.");
}

bool PocoClient::connect(const std::string& multicastAddress, int port) {
    if (mConnected) {
        Log::Warning("PocoClient::connect called but client is already connected.");
        return true;
    }

    Log::Info(Print::composeMessage("Attempting to connect PocoClient to multicast ", multicastAddress, " on port ", port));

    namespace PocoNet = ::Poco::Net;
    try {
        PocoNet::IPAddress ipAddr = PocoNet::IPAddress::parse(multicastAddress);
        if (!ipAddr.isMulticast()) {
            Log::Error(Print::composeMessage("Provided address is not a multicast address: ", multicastAddress));
            return false;
        }
        mMulticastGroupAddress = PocoNet::SocketAddress(ipAddr, port);

        mSocket = std::make_unique<PocoNet::MulticastSocket>(PocoNet::SocketAddress::IPv4);

        mSocket->setReuseAddress(true);
        mSocket->bind(PocoNet::SocketAddress(PocoNet::IPAddress(), port), true); 

        mSocket->joinGroup(mMulticastGroupAddress.host());

        mRunning = true;
        mConnected = true;

        mReceiveThread = std::jthread([this](std::stop_token stoken) {
            receiveLoop(stoken); 
        });

        Log::Info(Print::composeMessage("PocoClient connected. Joined group: ", mMulticastGroupAddress.host().toString(), " on port ", port));

        if (mOnConnected) {
            try {
                mOnConnected();
            } catch (const std::exception& e) {
                Log::Error(Print::composeMessage("Exception in OnConnected callback: ", e.what()));
            }
        }

        return true;

    } catch (const PocoNet::NetException& e) {
        Log::Error(Print::composeMessage("Poco NetException during client connect: ", e.displayText()));
        disconnect();
        return false;
    } catch (const ::Poco::Exception& e) {
        Log::Error(Print::composeMessage("Poco Exception during client connect: ", e.displayText()));
        disconnect();
        return false;
    } catch (const std::exception& e) {
        Log::Error(Print::composeMessage("Standard exception during client connect: ", e.what()));
        disconnect();
        return false;
    }
}

void PocoClient::disconnect() {
    if (!mRunning) {
        return;
    }
    Log::Info("Disconnecting PocoClient...");

    mRunning = false;
    mConnected = false;

    if (mReceiveThread.joinable()) {
        mReceiveThread.request_stop();
        if (mSocket) {
            try {
                mSocket->close();
            } catch (const ::Poco::Exception& e) {
                Log::Warning(Print::composeMessage("Exception closing socket during disconnect: ", e.displayText()));
            }
        }
        mReceiveThread.join();
        Log::Debug("PocoClient receive thread joined.");
    }

    try {
        if (mSocket) {
            try {
                if (mMulticastGroupAddress.host().isMulticast()) {
                    mSocket->leaveGroup(mMulticastGroupAddress.host());
                    Log::Debug(Print::composeMessage("Left multicast group: ", mMulticastGroupAddress.host().toString()));
                }
            } catch (const ::Poco::Exception& e) {
                 Log::Warning(Print::composeMessage("Exception leaving multicast group: ", e.displayText()));
            }
            
            if (mSocket->impl() && mSocket->impl()->initialized()) {
                mSocket->close();
            }
            mSocket.reset();
            Log::Debug("PocoClient socket closed and reset.");
        }
    } catch (const ::Poco::Exception& e) {
        Log::Error(Print::composeMessage("Poco Exception during client disconnect cleanup: ", e.displayText()));
    } catch (const std::exception& e) {
        Log::Error(Print::composeMessage("Standard exception during client disconnect cleanup: ", e.what()));
    }

    if (mOnDisconnected) {
        try {
            mOnDisconnected();
        } catch (const std::exception& e) {
            Log::Error(Print::composeMessage("Exception in OnDisconnected callback: ", e.what()));
        }
    }
    Log::Info("PocoClient disconnected.");
}

bool PocoClient::isConnected() const {
    return mConnected;
}

void PocoClient::setOnConnected(std::function<void()> callback) {
    mOnConnected = std::move(callback);
}

void PocoClient::setOnDisconnected(std::function<void()> callback) {
    mOnDisconnected = std::move(callback);
}

void PocoClient::setOnDataReceived(std::function<void(const std::string&)> callback) {
    mOnDataReceived = std::move(callback);
}

void PocoClient::receiveLoop(std::stop_token stopToken) {
    Log::Debug("PocoClient receive loop started.");
    ::Poco::Timespan timeout(10000);

    while (!stopToken.stop_requested() && mRunning) {
        namespace PocoNet = ::Poco::Net;
        try {
            if (mSocket && mSocket->poll(timeout, PocoNet::Socket::SELECT_READ)) {
                int bytesReceived = mSocket->receiveFrom(mReceiveBuffer.data(), static_cast<int>(mReceiveBuffer.size()), mSenderAddress);
                if (bytesReceived > 0) {
                    handleReceivedData(bytesReceived);
                } else if (bytesReceived == 0) {
                    Log::Debug("ReceiveFrom returned 0 bytes.");
                } else {
                    Log::Warning("ReceiveFrom returned negative value after poll indicated readability.");
                }
            } else {
                if (stopToken.stop_requested() || !mRunning) {
                    break;
                }
            }
        } 
        catch (const ::Poco::TimeoutException&) {
            if (stopToken.stop_requested() || !mRunning) {
                break;
            }
        } 
        catch (const PocoNet::NetException& e) {
            if (mRunning && e.code() != POCO_EINTR && e.code() != POCO_EAGAIN && e.code() != POCO_EWOULDBLOCK) { // Ignore interrupt/retry errors if still running
                Log::Error(Print::composeMessage("Poco NetException in receive loop: ", e.displayText()));
            }
        } 
        catch (const ::Poco::Exception& e) {
            if (mRunning) {
                Log::Error(Print::composeMessage("Poco Exception in receive loop: ", e.displayText()));
            }
        } 
        catch (const std::exception& e) {
            if (mRunning) {
                Log::Error(Print::composeMessage("Standard exception in receive loop: ", e.what()));
            }
        }
    }
    Log::Debug("PocoClient receive loop finished.");
}

void PocoClient::handleReceivedData(int length) {
    std::string receivedPart(mReceiveBuffer.data(), length);
    size_t startPos = 0;

    mFrameBuffer += receivedPart;

    size_t delimiterPos = mFrameBuffer.find(FRAME_DELIMITER);
    while (delimiterPos != std::string::npos) {
        std::string completeFrame = mFrameBuffer.substr(0, delimiterPos);
        mFrameBuffer.erase(0, delimiterPos + 1);

        if (mOnDataReceived && !completeFrame.empty()) {
            try {
                mOnDataReceived(completeFrame);
            } catch (const std::exception& e) {
                Log::Error(Print::composeMessage("Exception in OnDataReceived callback: ", e.what()));
            }
        }
        delimiterPos = mFrameBuffer.find(FRAME_DELIMITER);
    }
}

} // namespace Streaming::Poco
