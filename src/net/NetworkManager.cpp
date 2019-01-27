#include "net/NetworkManager.hpp" // IWYU pragma: associated
#include "config/ServerConfig.hpp"
#include "log/Logger.hpp"
#include "net/websockets/WsListener.hpp"
#include "net/websockets/WsServer.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <filesystem>
#include <string>
#include <thread>

namespace boostander {
namespace net {

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

NetworkManager::NetworkManager(const boostander::config::ServerConfig& serverConfig) {}

std::shared_ptr<WSServer> NetworkManager::getWS() const { return wsServer_; }

void NetworkManager::handleIncomingMessages() {
  if (!wsServer_->getWsListener()->isAccepting()) {
    LOG(WARNING) << "WsListener not accepting incoming messages";
  }
  wsServer_->handleIncomingMessages();
}

/*
 * TODO
#include <csignal>
/// Block until SIGINT or SIGTERM is received.
void sigWait(net::io_context& ioc) {
  // Capture SIGINT and SIGTERM to perform a clean shutdown
  boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
  signals.async_wait([&](boost::system::error_code const&, int) {
    // Stop the `io_context`. This will cause `run()`
    // to return immediately, eventually destroying the
    // `io_context` and all of the sockets in it.
    LOG(WARNING) << "Called ioc.stop() on SIGINT or SIGTERM";
    ioc.stop();
    doServerRun = false;
  });
}
*/

void NetworkManager::run(const boostander::config::ServerConfig& serverConfig) {
  // NOTE: no 'this' in constructor
  wsServer_ = std::make_shared<WSServer>(this, serverConfig);

  wsServer_->runIocWsListener(serverConfig);

  // TODO int max_thread_num = std::thread::hardware_concurrency();

  wsServer_->runThreads(serverConfig);
}

void NetworkManager::finish() { wsServer_->finishThreads(); }

} // namespace net
} // namespace boostander
