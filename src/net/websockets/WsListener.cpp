#include "net/websockets/WsListener.hpp" // IWYU pragma: associated
#include "algo/StringUtils.hpp"
#include "log/Logger.hpp"
#include "net/NetworkManager.hpp"
#include "net/websockets/WsServer.hpp"
#include "net/websockets/WsSession.hpp"
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace boostander {
namespace net {

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace {

// TODO: prevent collision? respond ERROR to client if collided?
static std::string nextWsSessionId() { return boostander::algo::genGuid(); }

} // namespace

WsListener::WsListener(boost::asio::io_context& ioc, const boost::asio::ip::tcp::endpoint& endpoint,
                       std::shared_ptr<std::string const> doc_root, NetworkManager* nm)
    : socket_(ioc), acceptor_(ioc), doc_root_(doc_root), nm_(nm), endpoint_(endpoint),
      strand_(ioc.get_executor()) {
  configureAcceptor();
}

// Report a failure
void WsListener::on_WsListener_fail(beast::error_code ec, char const* what) {
  LOG(WARNING) << "on_WsListener_fail: " << what << ": " << ec.message();
}

void WsListener::configureAcceptor() {
  beast::error_code ec;

  // Open the acceptor
  acceptor_.open(endpoint_.protocol(), ec);
  if (ec) {
    on_WsListener_fail(ec, "open");
    return;
  }

  // Allow address reuse
  acceptor_.set_option(net::socket_base::reuse_address(true), ec);
  if (ec) {
    on_WsListener_fail(ec, "set_option");
    return;
  }

  // Bind to the server address
  acceptor_.bind(endpoint_, ec);
  if (ec) {
    on_WsListener_fail(ec, "bind");
    return;
  }

  // Start listening for connections
  acceptor_.listen(net::socket_base::max_listen_connections, ec);
  if (ec) {
    on_WsListener_fail(ec, "listen");
    return;
  }
}

// Start accepting incoming connections
void WsListener::run() {
  if (!isAccepting() || needClose_) {
    LOG(INFO) << "WsListener::run: not accepting";
    return;
  }
  do_accept();
}

// Stop accepting incoming connections
void WsListener::stop() {
  /**
   * @see https://github.com/boostorg/beast/issues/940
   * Calls to cancel() will always fail with boost::asio::error::operation_not_supported when run on
   * Windows XP, Windows Server 2003, and earlier versions of Windows, unless
   * BOOST_ASIO_ENABLE_CANCELIO is defined. However, the CancelIo function has two issues
   * https://www.boost.org/doc/libs/1_47_0/doc/html/boost_asio/reference/basic_stream_socket/cancel/overload1.html
   **/
  boost::asio::post(socket_.get_executor(), boost::asio::bind_executor(strand_, [&]() {
                      try {
                        beast::error_code ec;
                        // For portable behaviour with respect to graceful closure of a connected
                        // socket, call shutdown() before closing the socket.
                        if (socket_.is_open()) {
                          socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
                          LOG(INFO) << "shutdown socket...";
                          if (ec) {
                            on_WsListener_fail(ec, "socket_shutdown");
                          }
                          socket_.close(ec);
                          LOG(INFO) << "close socket...";
                          if (ec) {
                            on_WsListener_fail(ec, "socket_close");
                          }
                        }
                        if (acceptor_.is_open()) {
                          LOG(INFO) << "close acceptor...";
                          acceptor_.close(ec);
                          if (ec) {
                            on_WsListener_fail(ec, "acceptor_close");
                          }
                        }
                        needClose_ = true;
                      } catch (const boost::system::system_error& ex) {
                        LOG(WARNING) << "WsListener::stop: exception: " << ex.what();
                      }
                    }));
}

void WsListener::do_accept() {
  if (needClose_) {
    LOG(WARNING) << "WsListener::do_accept: need close";
    return;
  }
  acceptor_.async_accept(socket_, boost::asio::bind_executor(
                                      strand_, std::bind(&WsListener::on_accept, shared_from_this(),
                                                         std::placeholders::_1)));
}

std::shared_ptr<WsSession> WsListener::addClientSession(const std::string& newSessId) {
  if (needClose_) {
    LOG(INFO) << "WsListener::addClientSession: need close";
    return nullptr;
  }
  auto newWsSession = std::make_shared<WsSession>(std::move(socket_), nm_, newSessId);
  nm_->getWS()->addSession(newSessId, newWsSession);
  return newWsSession;
}

/**
 * @brief handles new connections and starts sessions
 */
void WsListener::on_accept(beast::error_code ec) {

  if (!isAccepting() || !socket_.is_open() || needClose_) {
    LOG(WARNING) << "WsListener::on_accept: not accepting";
    return;
  }

  if (ec) {
    on_WsListener_fail(ec, "accept");
  } else {
    // Create the session and run it
    const auto newSessId = nextWsSessionId();
    auto newWsSession = std::make_shared<WsSession>(std::move(socket_), nm_, newSessId);
    nm_->getWS()->addSession(newSessId, newWsSession);
    newWsSession->runAsServer();
  }

  // Accept another connection
  do_accept();
}

} // namespace net
} // namespace boostander
