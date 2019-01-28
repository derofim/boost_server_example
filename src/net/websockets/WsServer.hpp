#pragma once

#include "algo/CallbackManager.hpp"
#include "algo/NetworkOperation.hpp"
#include "net/SessionManagerBase.hpp"
#include <boost/asio.hpp>
#include <functional>
#include <map>
#include <string>
#include <thread>
#include <vector>

namespace boostander {
namespace net {
class WsSession;
class NetworkManager;
class WsListener;
} // namespace net
} // namespace boostander

namespace boostander {
namespace config {
class ServerConfig;
} // namespace config
} // namespace boostander

namespace boostander {
namespace net {

struct WsNetworkOperation : public algo::NetworkOperation<algo::WS_OPCODE> {
  WsNetworkOperation(const algo::WS_OPCODE& operationCode, const std::string& operationName)
      : NetworkOperation(operationCode, operationName) {}

  WsNetworkOperation(const algo::WS_OPCODE& operationCode) : NetworkOperation(operationCode) {}
};

typedef std::function<void(WsSession* clientSession, NetworkManager* nm,
                           std::shared_ptr<std::string> messageBuffer)>
    WsNetworkOperationCallback;

class WSInputCallbacks
    : public algo::CallbackManager<WsNetworkOperation, WsNetworkOperationCallback> {
public:
  WSInputCallbacks();

  ~WSInputCallbacks() override;

  std::map<WsNetworkOperation, WsNetworkOperationCallback> getCallbacks() const override;

  void addCallback(const WsNetworkOperation& op, const WsNetworkOperationCallback& cb) override;
};

/**
 * @brief manages currently valid sessions
 */
class WSServer : public SessionManagerBase<WsSession, WSInputCallbacks> {
public:
  WSServer(NetworkManager* nm, const boostander::config::ServerConfig& serverConfig);

  void sendToAll(const std::string& message) override;

  void sendTo(const std::string& sessionID, const std::string& message) override;

  void handleIncomingMessages() override;

  void unregisterSession(const std::string& id) override;

  void runThreads(const config::ServerConfig& serverConfig) override;

  void finishThreads() override;

  void runIocWsListener(const config::ServerConfig& serverConfig);

  std::shared_ptr<WsListener> getWsListener() const { return iocWsListener_; }

  // The io_context is required for all I/O
  boost::asio::io_context ioc_;

private:
  std::shared_ptr<WsListener> iocWsListener_;

  // Run the I/O service on the requested number of threads
  std::vector<std::thread> wsThreads_;

  NetworkManager* nm_;
};

} // namespace net
} // namespace boostander
