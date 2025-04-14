#include "Server.h"
#include <iostream>

namespace Streaming::Asio {

AsioServer::AsioServer()
    : mRunning(false) {
}

AsioServer::~AsioServer() {
    stop();
}

bool AsioServer::start(const std::string& multicastAddress, int port, int threadCount) {
    if (mRunning) {
        return true;
    }
    
    try {
        setupMulticast(multicastAddress, port);
        mWork.emplace(boost::asio::make_work_guard(mIoContext));
        
        if (threadCount < 1) {
            threadCount = 1;
        }
        
        mThreadPool.reserve(threadCount);
        for (int i = 0; i < threadCount; ++i) {
            mThreadPool.emplace_back([this]() {
                try {
                    mIoContext.run();
                } catch (const std::exception& e) {
                    std::cerr << "Exception in thread pool: " << e.what() << std::endl;
                }
            });
        }
        
        mRunning = true;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error starting server: " << e.what() << std::endl;
        stop();
        return false;
    }
}

void AsioServer::stop() {
    if (!mRunning) {
        return;
    }
    
    mRunning = false;
    
    try {
        if (mWork) {
            mWork.reset();
        }
        
        mIoContext.stop();
        
        if (mSocket && mSocket->is_open()) {
            mSocket->close();
        }
        
        for (auto& thread : mThreadPool) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        mThreadPool.clear();
    }
    catch (const std::exception& e) {
        std::cerr << "Error stopping server: " << e.what() << std::endl;
    }
}

bool AsioServer::isRunning() const {
    return mRunning;
}

void AsioServer::broadcastData(const std::string& data) {
    if (!mRunning || !mSocket || !mSocket->is_open()) {
        return;
    }
    
    try {
        mSocket->send_to(boost::asio::buffer(data), mMulticastEndpoint);
    }
    catch (const std::exception& e) {
        std::cerr << "Error broadcasting data: " << e.what() << std::endl;
    }
}

void AsioServer::setupMulticast(const std::string& multicastAddress, int port) {
    using namespace boost::asio::ip;
    boost::system::error_code ec;
    address multicast_ip = make_address(multicastAddress, ec);
    if (ec) {
        throw std::runtime_error("Invalid multicast address: " + multicastAddress + " - " + ec.message());
    }
    mMulticastEndpoint = udp::endpoint(multicast_ip, port);

    mSocket = std::make_unique<udp::socket>(mIoContext, mMulticastEndpoint.protocol());
    mSocket->set_option(udp::socket::reuse_address(true));
}

} // namespace Streaming::Asio
