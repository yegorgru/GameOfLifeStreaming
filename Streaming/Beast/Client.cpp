#include "Client.h"
#include "Log.h"

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/post.hpp>

#include <iostream>
#include <string>
#include <thread>
#include <functional>

namespace Streaming::Beast {

namespace {
    const char FRAME_DELIMITER = '\n';
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace websocket = beast::websocket;
    namespace net = boost::asio;
    using tcp = boost::asio::ip::tcp;
}

BeastClient::BeastClient()
    : mServerPort(0)
    , mRunning(false)
    , mConnected(false)
{
    Log::Debug("Beast client created.");
}

BeastClient::~BeastClient() {
    Log::Debug("Beast client destroying...");
    if (mConnected || mRunning) {
        disconnect();
    }
}

bool BeastClient::connect(const std::string& serverAddress, int port) {
    if (mConnected) {
        Log::Warning("Already connected, disconnect first.");
        return false;
    }
    
    if (mRunning) {
        Log::Warning("Client is already running, disconnect first.");
        return false;
    }
    
    mServerAddress = serverAddress;
    mServerPort = port;
    
    Log::Info(
        Print::composeMessage(
            "Attempting to connect to WebSocket server at ", mServerAddress, ":", mServerPort
        )
    );
    
    try {
        mIoContext.restart();
        
        mResolver = std::make_unique<Resolver>(mIoContext);
        auto const results = mResolver->resolve(mServerAddress, std::to_string(mServerPort));

        mWebSocket = std::make_unique<WebSocket>(mIoContext);

        beast::get_lowest_layer(*mWebSocket).expires_after(std::chrono::seconds(30));
        auto ep = beast::get_lowest_layer(*mWebSocket).connect(results);

        std::string host = mServerAddress + ":" + std::to_string(ep.port());
        
        beast::get_lowest_layer(*mWebSocket).expires_never();
        
        mWebSocket->set_option(
            websocket::stream_base::timeout::suggested(beast::role_type::client)
        );
        
        mWebSocket->set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req) {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " Beast.WebSocket.Client");
            }));
        
        mWebSocket->handshake(host, "/");
        mConnected = true;
        
        Log::Info(
            Print::composeMessage("Connected to WebSocket server at ", mServerAddress, ":", mServerPort)
        );
        
        mWork.emplace(mIoContext.get_executor());
        mRunning = true;
        mThread = std::jthread([this]() {
            try {
                mIoContext.run();
            }
            catch  (const std::exception& e) {
                Log::Error(Print::composeMessage("Exception in io_context thread: ", e.what()));
            }
        });
        
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if (mOnConnected) {
                mOnConnected();
            }
        }

        doRead();
        
        return true;
    } catch (const std::exception& e) {
        Log::Error(Print::composeMessage("Exception during connect: ", e.what()));
        
        if (mWebSocket) {
            try {
                if (mWebSocket->is_open()) {
                    beast::error_code ec;
                    mWebSocket->close(websocket::close_code::normal, ec);
                }
            } catch (...) {
                Log::Error("Failed to close WebSocket connection during error handling.");
            }
            mWebSocket.reset();
        }
        
        mWork.reset();
        mResolver.reset();
        mConnected = false;
        mRunning = false;
        return false;
    }
}

void BeastClient::disconnect() {
    if (!mRunning.exchange(false)) {
        Log::Warning("Client is already stopped.");
        return;
    }
    
    Log::Info("Disconnecting from WebSocket server...");
    
    try {
        if (mWebSocket && mConnected) {
            net::post(mIoContext, [this]() {
                doClose();
            });
        }
        
        mWork.reset();
        
        mIoContext.stop();
        
        if (mThread.joinable()) {
            mThread.join();
        }
        
        mWebSocket.reset();
        mResolver.reset();
        
        mConnected = false;
        
        Log::Info("Disconnected from WebSocket server.");
    } catch (const std::exception& e) {
        Log::Error(Print::composeMessage("Exception during disconnect: ", e.what()));
    }
}

bool BeastClient::isConnected() const {
    return mConnected;
}

void BeastClient::setOnConnected(ConnectionCallback callback) {
    std::lock_guard<std::mutex> lock(mMutex);
    mOnConnected = std::move(callback);
}

void BeastClient::setOnDisconnected(ConnectionCallback callback) {
    std::lock_guard<std::mutex> lock(mMutex);
    mOnDisconnected = std::move(callback);
}

void BeastClient::setOnDataReceived(DataCallback callback) {
    std::lock_guard<std::mutex> lock(mMutex);
    mOnDataReceived = std::move(callback);
}

void BeastClient::doRead() {
    if (!mRunning || !mConnected) {
        return;
    }

    mWebSocket->async_read(
        mBuffer,
        [this](beast::error_code ec, std::size_t bytesTransferred) {
            if (!mRunning || !mConnected) {
                return;
            }
            
            if (ec == websocket::error::closed) {
                Log::Info("WebSocket server closed the connection.");
                doClose();
                return;
            }
            if (ec) {
                fail(ec, "read");
                return;
            }
            
            std::string receivedData = beast::buffers_to_string(mBuffer.data());
            mFrameBuffer += receivedData;
            
            mBuffer.consume(bytesTransferred);
            
            size_t delimiterPos = mFrameBuffer.find(FRAME_DELIMITER);
            while (delimiterPos != std::string::npos) {
                std::string completeFrame = mFrameBuffer.substr(0, delimiterPos);
                mFrameBuffer.erase(0, delimiterPos + 1);
                if (!completeFrame.empty()) {
                    std::lock_guard<std::mutex> lock(mMutex);
                    if (mOnDataReceived) {
                        mOnDataReceived(completeFrame);
                    }
                }
                delimiterPos = mFrameBuffer.find(FRAME_DELIMITER);
            }
            doRead();
        });
}

void BeastClient::doClose() {
    if (!mConnected) {
        return;
    }
    mWebSocket->async_close(
        websocket::close_code::normal,
        [this](beast::error_code ec) {
            if (ec) {
                Log::Warning(Print::composeMessage("Error closing WebSocket: ", ec.message()));
            }
            
            if (mConnected.exchange(false)) {
                std::lock_guard<std::mutex> lock(mMutex);
                if (mOnDisconnected) {
                    mOnDisconnected();
                }
            }
            
            Log::Info("WebSocket connection closed.");
        }
    );
}

void BeastClient::fail(boost::system::error_code ec, const std::string& what) {
    if (ec == net::error::operation_aborted || ec == websocket::error::closed || !mRunning) {
        return;
    }
    
    Log::Error(Print::composeMessage("Beast WebSocket client error (", what, "): ", ec.message()));
    
    if (mConnected) {
        try {
            doClose();
        } catch (const std::exception& e) {
            Log::Error(Print::composeMessage("Exception during close after failure: ", e.what()));
        }
    }
}

} // namespace Streaming::Beast