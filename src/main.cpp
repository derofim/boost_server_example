/*
 * Copyright (c) 2019 Denis Trofimov (den.a.trofimov@yandex.ru)
 * Distributed under the MIT License.
 * See accompanying file LICENSE.md or copy at http://opensource.org/licenses/MIT
 */

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <algorithm>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem; // from <filesystem>

using namespace std::chrono_literals;

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Report a failure
void fail(beast::error_code ec, char const* what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

// Echoes back all received WebSocket messages
class session : public std::enable_shared_from_this<session> {
  websocket::stream<tcp::socket> ws_;
  net::strand<net::io_context::executor_type> strand_;
  beast::multi_buffer buffer_;

public:
  // Take ownership of the socket
  explicit session(tcp::socket socket) : ws_(std::move(socket)), strand_(ws_.get_executor()) {}

  // Start the asynchronous operation
  void run() {
    // Accept the websocket handshake
    ws_.async_accept(net::bind_executor(
        strand_, std::bind(&session::on_accept, shared_from_this(), std::placeholders::_1)));
  }

  void on_accept(beast::error_code ec) {
    if (ec)
      return fail(ec, "accept");

    // Read a message
    do_read();
  }

  void do_read() {
    // Read a message into our buffer
    ws_.async_read(buffer_, net::bind_executor(
                                strand_, std::bind(&session::on_read, shared_from_this(),
                                                   std::placeholders::_1, std::placeholders::_2)));
  }

  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    // This indicates that the session was closed
    if (ec == websocket::error::closed)
      return;

    if (ec)
      fail(ec, "read");

    // Echo the message
    ws_.text(ws_.got_text());
    ws_.async_write(
        buffer_.data(),
        net::bind_executor(strand_, std::bind(&session::on_write, shared_from_this(),
                                              std::placeholders::_1, std::placeholders::_2)));
  }

  void on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
      return fail(ec, "write");

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Do another read
    do_read();
  }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener> {
  tcp::acceptor acceptor_;
  tcp::socket socket_;

public:
  listener(net::io_context& ioc, tcp::endpoint endpoint) : acceptor_(ioc), socket_(ioc) {
    beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
      fail(ec, "open");
      return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
      fail(ec, "set_option");
      return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if (ec) {
      fail(ec, "bind");
      return;
    }

    // Start listening for connections
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
      fail(ec, "listen");
      return;
    }
  }

  // Start accepting incoming connections
  void run() {
    if (!acceptor_.is_open())
      return;
    do_accept();
  }

  void do_accept() {
    acceptor_.async_accept(
        socket_, std::bind(&listener::on_accept, shared_from_this(), std::placeholders::_1));
  }

  void on_accept(beast::error_code ec) {
    if (ec) {
      fail(ec, "accept");
    } else {
      // Create the session and run it
      std::make_shared<session>(std::move(socket_))->run();
    }

    // Accept another connection
    do_accept();
  }
};

//------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
  // Check command line arguments.
  if (argc != 4) {
    std::cerr << "Usage: websocket-server-async <address> <port> <threads>\n"
              << "Example:\n"
              << "    websocket-server-async 0.0.0.0 8080 1\n";
    return EXIT_FAILURE;
  }
  auto const address = net::ip::make_address(argv[1]);
  auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
  auto const threads = std::max<int>(1, std::atoi(argv[3]));

  // The io_context is required for all I/O
  net::io_context ioc{threads};

  // Create and launch a listening port
  std::make_shared<listener>(ioc, tcp::endpoint{address, port})->run();

  // Run the I/O service on the requested number of threads
  std::vector<std::thread> v;
  v.reserve(threads - 1);
  for (auto i = threads - 1; i > 0; --i)
    v.emplace_back([&ioc] { ioc.run(); });
  ioc.run();

  return EXIT_SUCCESS;
}