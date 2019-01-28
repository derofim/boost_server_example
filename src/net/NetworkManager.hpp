#pragma once

#include <memory>

namespace boostander {
namespace config {
class ServerConfig;
} // namespace config
} // namespace boostander

namespace boostander {
namespace net {

/**
 * Type field stores uint32_t -> [0-4294967295] -> max 10 digits
 **/
constexpr unsigned long UINT32_FIELD_MAX_LEN = 10;

class WSServer;

class NetworkManager {
public:
  NetworkManager();

  void handleIncomingMessages();

  void run(const boostander::config::ServerConfig& serverConfig);

  void finish();

  std::shared_ptr<WSServer> getWS() const;

private:
  std::shared_ptr<WSServer> wsServer_;
};

} // namespace net
} // namespace boostander
