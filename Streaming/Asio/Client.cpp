#include "Client.h"
#include "Log.h"

namespace Streaming::Asio {

namespace {
    const size_t MAX_BUFFER_SIZE = 4096;
    const char FRAME_DELIMITER = '\n';
}

AsioClient::AsioClient()
    : mIoContext()
    , mSocket(mIoContext)
    , mReceiveBuffer(MAX_BUFFER_SIZE, ' ')
    , mFrameBuffer("")
    , mRunning(false)
    , mConnected(false) {
}

AsioClient::~AsioClient() {
    disconnect();
}

bool AsioClient::connect(const std::string& multicastAddress, int port) {
    if (mConnected) {
        return true;
    }

    try {
        namespace ip = boost::asio::ip;
        using udp = ip::udp;

        boost::system::error_code ec;
        ip::address multicastIp = ip::make_address(multicastAddress, ec);
        if (ec) {
             throw std::runtime_error("Invalid multicast address provided: " + multicastAddress);
        }
        mMulticastAddress = multicastAddress;

        mSocket.open(udp::v4());
        mSocket.set_option(udp::socket::reuse_address(true));
        mSocket.bind(udp::endpoint(ip::address_v4::any(), port));

        mSocket.set_option(ip::multicast::join_group(multicastIp));

        mWork.emplace(boost::asio::make_work_guard(mIoContext));
        mRunning = true;
        
        mThread = std::jthread([this]() {
            try {
                mIoContext.run();
            }
            catch (const std::exception& e) {
                std::cerr << "Error in IO context thread: " << e.what() << std::endl;
                mRunning = false;
            }
        });

        startReceive();
        
        mConnected = true;
        
        if (mOnConnected) {
            mOnConnected();
        }
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error connecting to server: " << e.what() << std::endl;
        disconnect();
        return false;
    }
}

void AsioClient::disconnect() {
    if (!mRunning) {
        return;
    }
    
    mConnected = false;
    mRunning = false;
    
    try {
        namespace ip = boost::asio::ip;
        if (mSocket.is_open()) {
            boost::system::error_code ec;
            ip::address multicastIp = ip::make_address(mMulticastAddress, ec);
            if (!ec) {
                mSocket.set_option(ip::multicast::leave_group(multicastIp));
            } else {
                Log::Warning(Print::composeMessage("Warning: Could not parse stored multicast address on disconnect: ", mMulticastAddress));
            }
            mSocket.close();
        }
        
        if (mWork) {
            mWork.reset();
        }
        
        mIoContext.stop();
        
        if (mThread.joinable()) {
            mThread.join();
        }
        
        if (mOnDisconnected) {
            mOnDisconnected();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error disconnecting: " << e.what() << std::endl;
    }
}

void AsioClient::setOnConnected(std::function<void()> callback) {
    mOnConnected = std::move(callback);
}

void AsioClient::setOnDisconnected(std::function<void()> callback) {
    mOnDisconnected = std::move(callback);
}

void AsioClient::setOnDataReceived(std::function<void(const std::string&)> callback) {
    mOnDataReceived = std::move(callback);
}

bool AsioClient::isConnected() const {
    return mConnected;
}

void AsioClient::startReceive() {
    if (!mRunning || !mSocket.is_open()) {
        return;
    }
    
    mSocket.async_receive_from(
        boost::asio::buffer(mReceiveBuffer), mSenderEndpoint,
        [this](const boost::system::error_code& error, std::size_t bytesReceived) {
            handleReceive(error, bytesReceived);
        }
    );
}

void AsioClient::handleReceive(const boost::system::error_code& error, std::size_t bytesReceived) {
    if (!error && bytesReceived > 0) {
        std::string receivedData(mReceiveBuffer.data(), bytesReceived);
        mFrameBuffer += receivedData;
        size_t delimiterPos = mFrameBuffer.find(FRAME_DELIMITER);
        while (delimiterPos != std::string::npos) {
            std::string completeFrame = mFrameBuffer.substr(0, delimiterPos);
            mFrameBuffer.erase(0, delimiterPos + 1);
            if (mOnDataReceived && !completeFrame.empty()) {
                mOnDataReceived(completeFrame);
            }
            delimiterPos = mFrameBuffer.find(FRAME_DELIMITER);
        }
    }
    else if (error != boost::asio::error::operation_aborted) {
        std::cerr << "Error receiving data: " << error.message() << std::endl;
    }
    if (mRunning) {
        startReceive();
    }
}

} // namespace Streaming::Asio
