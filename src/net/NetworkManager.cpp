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

NetworkManager::NetworkManager() {}

std::shared_ptr<WSServer> NetworkManager::getWS() const { return wsServer_; }

void NetworkManager::handleIncomingMessages() {
  if (!wsServer_->getWsListener()->isAccepting()) {
    LOG(WARNING) << "WsListener not accepting incoming messages";
  }
  wsServer_->handleIncomingMessages();
}

void NetworkManager::run(const boostander::config::ServerConfig& serverConfig) {
  // NOTE: no 'this' in constructor
  wsServer_ = std::make_shared<WSServer>(this, serverConfig);

  wsServer_->runIocWsListener(serverConfig);

  wsServer_->runThreads(serverConfig);
}

void NetworkManager::finish() {
  wsServer_->getWsListener()->stop();
  wsServer_->finishThreads();
}

} // namespace net
} // namespace boostander
