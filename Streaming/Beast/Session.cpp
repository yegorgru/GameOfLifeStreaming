#include "Session.h"
#include "Server.h"
#include <boost/asio.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <boost/beast/http.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Log.h"

namespace Streaming::Beast {

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;

Session::Session(BeastServer& server, TcpSocket&& socket)
    : mServer(server)
    , mStream(std::move(socket))
    , mIsWriting(false)
    , mIsClosing(false) 
{
    Log::Debug("Beast Session created.");
}

Session::~Session() {
    Log::Debug("Beast Session destroyed.");
}

void Session::run() {
    mSelfPtr = shared_from_this();

    mStream.set_option(
        websocket::stream_base::timeout::suggested(
            beast::role_type::server
        )
    );

    mStream.set_option(websocket::stream_base::decorator(
        [](websocket::response_type& res)
        {
            res.set(http::field::server,
                std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-async");
        }));

    mStream.async_accept(
        [this](beast::error_code ec) {
            if (mIsClosing) {
                return;
            }
            if (ec) {
                fail(ec, "accept");
                return;
            }
            Log::Info("Beast WebSocket connection accepted.");
            mServer.addSession(mSelfPtr);
            read();
        }
    );
}

void Session::read() {
    if (mIsClosing) {
        return;
    }
    
    mStream.async_read(
        mBuffer,
        [this](beast::error_code ec, size_t) {
            if (mIsClosing) {
                return;
            }
            if (ec == websocket::error::closed) {
                Log::Info("Beast WebSocket connection closed by peer.");
                close();
                return;
            }
            if (ec) {
                fail(ec, "read");
                return;
            }
            mBuffer.consume(mBuffer.size());
            read();
        }
    );
}

void Session::send(const std::string& message) {
    if (mIsClosing) {
        return;
    }
    boost::asio::post(
        mStream.get_executor(),
        [this, msg = message]() {
            if (mIsClosing) {
                return;
            }

            bool startWriting;
            {
                std::lock_guard<std::mutex> lock(mQueueMutex);
                startWriting = mWriteQueue.empty();
                mWriteQueue.push(std::move(msg));
            }

            if (startWriting) {
                write();
            }
        }
    );
}

void Session::write() {
    if (mIsClosing || mIsWriting) {
        return;
    }

    std::shared_ptr<std::string> messageToSend;
    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        if (mWriteQueue.empty()) {
            return;
        }
        mIsWriting = true;
        messageToSend = std::make_shared<std::string>(std::move(mWriteQueue.front()));
        mWriteQueue.pop();
    }

    try {
        mStream.text(true);
        mStream.async_write(
            boost::asio::buffer(*messageToSend),
            [this, messageToSend](beast::error_code ec, std::size_t) {
                if (mIsClosing) {
                    return;
                }

                if (ec) {
                    if (ec != boost::asio::error::operation_aborted) {
                        fail(ec, "write");
                    }
                    close();
                    return;
                }

                bool queueEmpty = true;
                {
                    std::lock_guard<std::mutex> lock(mQueueMutex);
                    queueEmpty = mWriteQueue.empty();
                }

                if (!queueEmpty) {
                    write();
                } else {
                    mIsWriting = false;
                }
            }
        );
    } catch (const std::exception& e) {
        Log::Error(Print::composeMessage("Beast Session Exception in write(): ", e.what()));
        mIsWriting = false;
        close();
    } catch (...) {
        Log::Error("Beast Session Unknown exception in write()");
        mIsWriting = false;
        close();
    }
}

void Session::fail(beast::error_code ec, const std::string& message) {
    if (mIsClosing && (ec == boost::asio::error::operation_aborted || ec == websocket::error::closed)) {
        return;
    }

    Log::Error(Print::composeMessage("Beast Session Error (", message, "): ", ec.message()));

    close();
}

void Session::close() {
    if (mIsClosing.exchange(true)) {
        return;
    }

    Log::Debug("Initiating Beast Session close...");
    
    if (mSelfPtr) {
        mServer.removeSession(mSelfPtr);
    }

    boost::asio::post(mStream.get_executor(), 
        [this]() {
            if (mStream.is_open()) {
                mStream.async_close(websocket::close_code::normal, 
                    [this](beast::error_code) {
                        // not interested
                    });
            }
            mSelfPtr.reset();
        });
}

} // namespace Streaming::Beast