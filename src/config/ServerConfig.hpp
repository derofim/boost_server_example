#pragma once

#include <boost/asio.hpp>
#include <cstdint>
#include <filesystem>

namespace boostander {
namespace config {

struct ServerConfig {
  ServerConfig(const std::filesystem::path& workdir);

  void print() const;

  void loadConf();

  const std::filesystem::path workdir_;

  // TODO private:
  boost::asio::ip::address address_;

  // port for WebSockets connection
  // 0 is for random port
  unsigned short wsPort_;

  int32_t threads_;
};

} // namespace config
} // namespace boostander
