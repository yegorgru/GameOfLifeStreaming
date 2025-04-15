#include "Server.h"
#include "Session.h"
#include "Acceptor.h"
#include "Log.h"
#include <boost/beast/websocket/stream.hpp>
#include <memory>
#include <vector>
#include <boost/asio/post.hpp>
#include <boost/asio/strand.hpp>

namespace Streaming::Beast {

BeastServer::BeastServer()
    : mRunning(false)
{
    Log::Debug("BeastServer creating...");
}

BeastServer::~BeastServer() {
    Log::Debug("BeastServer destroying...");
    if (mRunning) {
        stop();
    }
}

bool BeastServer::start(const std::string& address, int port, int threadCount) {
    if (mRunning) {
        Log::Warning("BeastServer::start called but server is already running.");
        return true;
    }
    Log::Info(Print::composeMessage("Starting BeastServer on port ", port, " with ", threadCount, " threads..."));

    mRunning = true;
    mWork.emplace(boost::asio::make_work_guard(mIoContext));

    auto acceptorCallback = [this](Acceptor::TcpSocketPtr socket_ptr) {
        boost::asio::post(mIoContext, 
            [this, socketPtr = std::move(socket_ptr)]() {
            if (!mRunning) {
                 Log::Debug("Server stopped before processing accepted socket in post.");
                 if (socketPtr && socketPtr->is_open()) {
                    boost::beast::error_code ignored_ec;
                    socketPtr->shutdown(Acceptor::TcpSocket::shutdown_both, ignored_ec);
                    socketPtr->close(ignored_ec);
                 }
                 return;
            }
            onAccept(std::move(socketPtr));
        });
    };
    mAcceptor = std::make_unique<Acceptor>(mIoContext, acceptorCallback);
    mAcceptor->start(address, port);

    mThreadPool.reserve(threadCount);
    for (int i = 0; i < threadCount; i++) {
        mThreadPool.emplace_back(
            [this, i]() {
                try {
                    Log::Debug(Print::composeMessage("BeastServer thread started: ", i, " (ID: ", std::this_thread::get_id(), ")"));
                    mIoContext.run();
                    Log::Debug(Print::composeMessage("BeastServer thread exiting: ", i, " (ID: ", std::this_thread::get_id(), ")"));
                } catch (const std::exception& e) {
                    Log::Error(Print::composeMessage("BeastServer thread exception: ", e.what()));
                } catch (...) {
                    Log::Error("BeastServer thread unknown exception.");
                }
            }
        );
    }

    Log::Info(Print::composeMessage("BeastServer started successfully on ", address, ":", port));
    return true;
}

void BeastServer::stop() {
    if (!mRunning.exchange(false)) {
        Log::Warning("BeastServer::stop called but server is not running.");
        return;
    }
    Log::Info("Stopping BeastServer...");

    if (mAcceptor) {
        mAcceptor->stop();
        mAcceptor.reset();
    }

    Log::Debug("Closing active sessions...");
    SessionStorage sessionsCopy;
    {
        std::lock_guard<std::mutex> lg(mSessionsMutex);
        sessionsCopy = mSessions;
    }
    for (const auto& sessionPtr : sessionsCopy) {
        if (sessionPtr) {
            sessionPtr->close();
        }
    }
    mSessions.clear();

    if (mWork) {
        mWork->reset();
        mWork.reset();
    }

    Log::Debug("Joining thread pool...");
    for (size_t i = 0; i < mThreadPool.size(); ++i) {
        if (mThreadPool[i].joinable()) {
            mThreadPool[i].join();
        }
    }
    mThreadPool.clear();

    mIoContext.stop();

    Log::Info("BeastServer stopped.");
}

void BeastServer::broadcastData(const std::string& data) {
    if (!mRunning) {
        return;
    }

    SessionStorage sessionsCopy;
    {
        std::lock_guard<std::mutex> lg(mSessionsMutex);
        sessionsCopy = mSessions;
    }
    for (const auto& session_ptr : sessionsCopy) {
        session_ptr->send(data);
    }
}

bool BeastServer::isRunning() const {
    return mRunning;
}

void BeastServer::addSession(SessionPtr session) {
    if (!session) {
        return;
    }
    std::lock_guard<std::mutex> lock(mSessionsMutex);
    mSessions.insert(session);
    Log::Debug(Print::composeMessage("Session added. Total sessions: ", mSessions.size()));
}

void BeastServer::removeSession(SessionPtr session) {
    if (!session) {
        return;
    }
    std::lock_guard<std::mutex> lock(mSessionsMutex);
    mSessions.erase(session);
    Log::Debug(Print::composeMessage("Session removed. Total sessions: ", mSessions.size()));
}

void BeastServer::onAccept(Acceptor::TcpSocketPtr socketPtr) {
    if (!mRunning) {
        Log::Debug("Server stopped before processing accepted socket in onAccept.");
        if (socketPtr && socketPtr->is_open()) {
            boost::beast::error_code ignored;
            socketPtr->shutdown(Acceptor::TcpSocket::shutdown_both, ignored);
            socketPtr->close(ignored);
        }
        return;
    }

    if (!socketPtr) {
        Log::Error("BeastServer::onAccept received null socket pointer.");
        return;
    }

    Log::Debug("BeastServer::onAccept - Creating session for new connection.");
    auto session = std::make_shared<Session>(*this, std::move(*socketPtr));
    session->run();
}

} // namespace Streaming::Beast

