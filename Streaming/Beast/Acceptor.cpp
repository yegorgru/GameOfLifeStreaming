#include "Acceptor.h"

namespace Streaming::Beast {

Acceptor::Acceptor(IoContext& ios, Callback callback)
    : mIos(ios)
    , mAcceptor(nullptr)
    , mIsStopped(false)
    , mCallback(callback)
{
}

void Acceptor::start(const std::string& rawIp, unsigned short portNum) {
    Log::Info("Start Acceptor");
    mAcceptor = std::make_unique<TcpAcceptor>(mIos, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(rawIp), portNum));
    mAcceptor->listen();
    initAccept();
}

void Acceptor::stop() {
    Log::Info("Stop Acceptor");
    if (mNextSocket) {
        mNextSocket->close();
    }
    if (mAcceptor) {
        mAcceptor->close();
    }
    mIsStopped = true;
}

void Acceptor::initAccept() {
    Log::Debug("Waiting for new connection");
    mNextSocket = std::make_shared<TcpSocket>(mIos);

    mAcceptor->async_accept(*mNextSocket.get(),
        [this](const boost::system::error_code& ec) {
            if (mIsStopped) {
                Log::Debug("Acceptor stopped, ignoring async_accept result.");
                return;
            }

            if (ec.value() == 0) {
                Log::Debug("New connection accepted");
                mCallback(mNextSocket);
            } else {
                Log::Error(Print::composeMessage("Error occured! Error code = ", ec.value(), ". Message: ", ec.message()));
            }

            if (!mIsStopped) {
                initAccept();
            }
        });
}

} // namespace Streaming::Beast