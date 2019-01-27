#pragma once

#include <boost/asio.hpp>
#include <cstdint>
#include <filesystem>
#include <string>

namespace sol {
class state;
} // namespace sol

namespace boostander {
namespace config {

const std::string ASSETS_DIR = "assets";
const std::string CONFIGS_DIR = "configuration_files";
const std::string CONFIG_NAME = "server_conf.txt"; // TODO

struct ServerConfig {
  ServerConfig(const std::filesystem::path& workdir);

  ServerConfig(const std::filesystem::path& configPath, const std::filesystem::path& workdir);

  void print() const;

  void loadConf();

  const std::filesystem::path workdir_;

  // TODO private:
  boost::asio::ip::address address_;

  // port for WebSockets connection
  unsigned short wsPort_;

  int32_t threads_;
};

} // namespace config
} // namespace boostander
