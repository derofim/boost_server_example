#pragma once

#include "net/SessionBase.hpp"
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <cstddef>
#include <string>
#include <vector>

namespace boostander {
namespace algo {
class DispatchQueue;
} // namespace algo
} // namespace boostander

namespace boostander {
namespace net {

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// NOTE: ProducerConsumerQueue must be created with a fixed maximum size
// We use Queue per connection, so it is for 1 client
constexpr size_t maxSendQueueElems = 16;

class NetworkManager;
class PCO;

enum class PING_STATE : uint32_t { ALIVE, SENDING, SENT, TOTAL };

// Echoes back all received WebSocket messages
class WsSession : public SessionBase, public std::enable_shared_from_this<WsSession> {
public:
  // Take ownership of the socket
  explicit WsSession(boost::asio::ip::tcp::socket socket, NetworkManager* nm,
                     const std::string& id);

  ~WsSession();

  // Start the asynchronous operation
  void runAsServer();

  void on_session_fail(boost::beast::error_code ec, char const* what);

  void on_control_callback(boost::beast::websocket::frame_type kind,
                           boost::string_view payload); // TODO boost::string_view or
                                                        // std::string_view

  // Called to indicate activity from the remote peer
  void onRemoteActivity();

  void on_accept(boost::beast::error_code ec);

  // Called when the timer expires.
  void on_timer(boost::beast::error_code ec);

  void do_read();

  bool handleIncomingData(std::shared_ptr<std::string> message) override;

  void send(std::shared_ptr<std::string> ss) override;

  void send(const std::string& ss) override;

  void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);

  void on_write(boost::beast::error_code ec, std::size_t bytes_transferred);

  void on_ping(boost::beast::error_code ec);

  bool isOpen() const;

  bool fullyCreated() const { return isFullyCreated_; }

  void connectAsClient(const std::string& host, const std::string& port);

  void onResolve(beast::error_code ec, tcp::resolver::results_type results);

  void onConnect(beast::error_code ec);

  void onHandshake(beast::error_code ec);

  bool waitForConnect(std::size_t max_wait_ms) const;

  void runAsClient();

private:
  bool isFullyCreated_{false};

  /**
   * 16 Kbyte for the highest throughput, while also being the most portable one
   * @see https://viblast.com/blog/2015/2/5/webrtc-data-channel-message-size/
   **/
  static constexpr size_t maxReceiveMsgSizebyte = 1024 * 1024 * 1024;
  static constexpr size_t maxSendMsgSizebyte = 1024 * 1024 * 1024;

  /**
   * The websocket::stream class template provides asynchronous and blocking message-oriented
   * functionality necessary for clients and servers to utilize the WebSocket protocol.
   * @note all asynchronous operations are performed within the same implicit or explicit strand.
   **/
  boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;

  /**
   * I/O objects such as sockets and streams are not thread-safe. For efficiency, networking adopts
   * a model of using threads without explicit locking by requiring all access to I/O objects to be
   * performed within a strand.
   */
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;

  boost::asio::ip::tcp::resolver resolver_;

  boost::beast::multi_buffer recievedBuffer_;

  boost::asio::steady_timer timer_;

  bool isSendBusy_;

  std::vector<std::shared_ptr<const std::string>> sendQueue_;

  NetworkManager* nm_;

  PING_STATE pingState_ = PING_STATE::ALIVE;
};

} // namespace net
} // namespace boostander
