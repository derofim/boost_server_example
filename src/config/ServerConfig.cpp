#include "config/ServerConfig.hpp" // IWYU pragma: associated
#include "log/Logger.hpp"
#include "storage/path.hpp"
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>

namespace boostander {
namespace config {

namespace fs = std::filesystem;         // from <filesystem>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

ServerConfig::ServerConfig(const fs::path& workdir) : workdir_(workdir) { loadConf(); }

void ServerConfig::print() const {
  LOG(INFO) << "address: " << address_.to_string() << '\n'
            << "port: " << wsPort_ << '\n'
            << "threads: " << threads_;
}

void ServerConfig::loadConf() {
  address_ = net::ip::make_address("127.0.0.1");
  wsPort_ = static_cast<unsigned short>(8080);
  threads_ = 1;
}

} // namespace config
} // namespace boostander
