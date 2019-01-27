#pragma once

#include <boost/asio.hpp>
#include <thread>
#include <vector>

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
  NetworkManager(const boostander::config::ServerConfig& serverConfig);

  void handleIncomingMessages();

  void run(const boostander::config::ServerConfig& serverConfig);

  void finish();

  std::shared_ptr<WSServer> getWS() const;

private:
  std::shared_ptr<WSServer> wsServer_;

  // TODO
  //  std::vector<Player> players_;
};

} // namespace net
} // namespace boostander
