#include "net/websockets/WsServer.hpp" // IWYU pragma: associated
#include "algo/CSV.hpp"
#include "algo/DispatchQueue.hpp"
#include "algo/NetworkOperation.hpp"
#include "algo/StringUtils.hpp"
#include "config/ServerConfig.hpp"
#include "log/Logger.hpp"
#include "net/websockets/WsListener.hpp"
#include "net/websockets/WsSession.hpp"
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace {

using namespace ::boostander::net;

static void pingCallback(WsSession* clientSession, NetworkManager* nm,
                         std::shared_ptr<std::string> messageBuffer) {
  using boostander::algo::Opcodes;
  using boostander::algo::WS_OPCODE;

  if (!messageBuffer || !messageBuffer.get()) {
    LOG(WARNING) << "WsServer: Invalid messageBuffer";
    return;
  }

  if (!clientSession) {
    LOG(WARNING) << "WSServer invalid clientSession!";
    return;
  }

  /*const std::string payload = messageBuffer->substr(1, messageBuffer->size() - 1);
  const std::string pingResponse = Opcodes::opcodeToStr(WS_OPCODE::PING) + payload;*/

  // const std::string incomingStr = beast::buffers_to_string(messageBuffer->data());
  LOG(INFO) << std::this_thread::get_id() << ":"
            << "pingCallback incomingMsg=" << messageBuffer->substr(0, 50).c_str();

  // send same message back (ping-pong)
  clientSession->send(messageBuffer);
}

static void csvAnalizeCallback(WsSession* clientSession, NetworkManager* nm,
                               std::shared_ptr<std::string> messageBuffer) {
  using boostander::algo::Opcodes;
  using boostander::algo::WS_OPCODE;

  using namespace boostander::algo;
  using namespace boostander::net;

  if (!messageBuffer || !messageBuffer.get()) {
    LOG(WARNING) << "WsServer: Invalid messageBuffer";
    return;
  }

  if (!clientSession) {
    LOG(WARNING) << "WSServer invalid clientSession!";
    return;
  }

  const std::string payload = messageBuffer->substr(1, messageBuffer->size() - 1);

  LOG(INFO) << std::this_thread::get_id() << ":"
            << "testCSVCallback incomingMsg=" << messageBuffer->substr(0, 50).c_str();

  CSV csv;
  std::chrono::system_clock::time_point maxDate;
  double delim = 0.0;
  {
    csv.loadFromMemory(payload);
    if (csv.getRowsCount() <= 0 || csv.getColsCount() <= 0) {
      LOG(WARNING) << "csvAnalizeCallback: Provided invalid csv file\n"
                   << "See example .csv files in assets folder\n";
    }
    for (int i = 0; i < csv.getRowsCount(); i++) {
      auto dt = dateTimeFromStr(csv.getAt(i, 0));
      if (dt > maxDate) {
        maxDate = dt;
        auto d1 = boost::lexical_cast<double>(csv.getAt(i, 1));
        auto d2 = boost::lexical_cast<double>(csv.getAt(i, 2));
        if (!std::isinf(d2) && d2 != 0) {
          delim = d1 / d2;
        }
      }
    }
  }

  LOG(WARNING) << "csvAnalizeCallback: maxDate = " << dateToStr(maxDate) << " delim = " << delim;

  // send analize result
  const std::string CSVResponse =
      Opcodes::opcodeToStr(WS_OPCODE::CSV_ANSWER) + std::to_string(csv.getRowsCount());
  clientSession->send(CSVResponse);
}

static void csvAnswerCallback(WsSession* clientSession, NetworkManager* nm,
                              std::shared_ptr<std::string> messageBuffer) {
  using boostander::algo::Opcodes;
  using boostander::algo::WS_OPCODE;

  if (!messageBuffer || !messageBuffer.get()) {
    LOG(WARNING) << "WsServer: Invalid messageBuffer";
    return;
  }

  if (!clientSession) {
    LOG(WARNING) << "WSServer invalid clientSession!";
    return;
  }

  const std::string payload = messageBuffer->substr(1, messageBuffer->size() - 1);
  LOG(WARNING) << "csvAnswerCallback: " << payload;

  // const std::string CSVResponse = Opcodes::opcodeToStr(WS_OPCODE::CSV_ANSWER) + payload;

  // const std::string incomingStr = beast::buffers_to_string(messageBuffer->data());
  /*LOG(INFO) << std::this_thread::get_id() << ":"
            << "testCSVCallback incomingMsg=" << messageBuffer->substr(0, 50).c_str();*/

  // send same message back
  // clientSession->send(messageBuffer);
}

} // namespace

namespace boostander {
namespace net {

WSInputCallbacks::WSInputCallbacks() {}

WSInputCallbacks::~WSInputCallbacks() {}

std::map<WsNetworkOperation, WsNetworkOperationCallback> WSInputCallbacks::getCallbacks() const {
  return operationCallbacks_;
}

void WSInputCallbacks::addCallback(const WsNetworkOperation& op,
                                   const WsNetworkOperationCallback& cb) {
  operationCallbacks_[op] = cb;
}

// TODO: add webrtc callbacks (similar to websockets)

WSServer::WSServer(NetworkManager* nm, const boostander::config::ServerConfig& serverConfig)
    : nm_(nm), ioc_(serverConfig.threads_) {

  {
    const WsNetworkOperation op = WsNetworkOperation(
        algo::WS_OPCODE::PING, algo::Opcodes::opcodeToStr(algo::WS_OPCODE::PING));
    operationCallbacks_.addCallback(op, &pingCallback);
  }

  {
    const WsNetworkOperation op = WsNetworkOperation(
        algo::WS_OPCODE::CSV_ANALIZE, algo::Opcodes::opcodeToStr(algo::WS_OPCODE::CSV_ANALIZE));
    operationCallbacks_.addCallback(op, &csvAnalizeCallback);
  }

  {
    const WsNetworkOperation op = WsNetworkOperation(
        algo::WS_OPCODE::CSV_ANSWER, algo::Opcodes::opcodeToStr(algo::WS_OPCODE::CSV_ANSWER));
    operationCallbacks_.addCallback(op, &csvAnswerCallback);
  }
}

/**
 * @brief removes session from list of valid sessions
 *
 * @param id id of session to be removed
 */
void WSServer::unregisterSession(const std::string& id) {
  const std::string idCopy = id; // unknown lifetime, use idCopy
  {
    std::shared_ptr<WsSession> sess = getSessById(idCopy);
    if (!removeSessById(idCopy)) {
      LOG(WARNING) << "WsServer::unregisterSession: trying to unregister non-existing session";
      // NOTE: continue cleanup with saved shared_ptr
    }
    if (!sess) {
      // throw std::runtime_error(
      LOG(WARNING) << "WsServer::unregisterSession: session already deleted";
      return;
    }
  }
  LOG(WARNING) << "WsServer: unregistered " << idCopy;
}

/**
 * @example:
 * std::time_t t = std::chrono::system_clock::to_time_t(p);
 * std::string msg = "server_time: ";
 * msg += std::ctime(&t);
 * sm->sendToAll(msg);
 **/
void WSServer::sendToAll(const std::string& message) {
  LOG(WARNING) << "WSServer::sendToAll:" << message;
  {
    for (auto& sessionkv : getSessions()) {
      if (!sessionkv.second || !sessionkv.second.get()) {
        LOG(WARNING) << "WSServer::sendToAll: Invalid session ";
        continue;
      }
      if (auto session = sessionkv.second.get()) {
        session->send(message);
      }
    }
  }
}

void WSServer::sendTo(const std::string& sessionID, const std::string& message) {
  {
    auto sessionsCopy = getSessions();
    auto it = sessionsCopy.find(sessionID);
    if (it != sessionsCopy.end()) {
      if (!it->second || !it->second.get()) {
        LOG(WARNING) << "WSServer::sendTo: Invalid session ";
        return;
      }
      it->second->send(message);
    }
  }
}

void WSServer::handleIncomingMessages() {
  // LOG(INFO) << "WSServer::handleIncomingMessages getSessionsCount " << getSessionsCount();
  doToAllSessions([&](const std::string& sessId, std::shared_ptr<WsSession> session) {
    if (!session || !session.get()) {
      LOG(WARNING) << "WsServer::handleAllPlayerMessages: trying to "
                      "use non-existing session";
      // NOTE: unregisterSession must be automatic!
      unregisterSession(sessId);
      return;
    }
    /*if (!session->isOpen() && session->fullyCreated()) {
      LOG(WARNING) << "WsServer::handleAllPlayerMessages: !session->isOpen()";
      // NOTE: unregisterSession must be automatic!
      unregisterSession(session->getId());
      return;
    }*/
    // LOG(INFO) << "doToAllSessions for " << session->getId();
    auto msgs = session->getReceivedMessages();
    if (!msgs || !msgs.get()) {
      LOG(WARNING) << "WsServer::handleAllPlayerMessages: invalid session->getReceivedMessages()";
      return;
    }
    msgs->DispatchQueued();
  });
}

void WSServer::runThreads(const config::ServerConfig& serverConfig) {
  wsThreads_.reserve(serverConfig.threads_);
  for (auto i = serverConfig.threads_; i > 0; --i) {
    wsThreads_.emplace_back([this] { ioc_.run(); });
  }
  // TODO sigWait(ioc);
  // TODO ioc.run();
}

void WSServer::finishThreads() {
  // Block until all the threads exit
  for (auto& t : wsThreads_) {
    if (t.joinable()) {
      t.join();
    }
  }
}

void WSServer::runIocWsListener(const config::ServerConfig& serverConfig) {

  tcp::endpoint tcpEndpoint = tcp::endpoint{serverConfig.address_, serverConfig.wsPort_};

  std::shared_ptr<std::string const> workdirPtr =
      std::make_shared<std::string>(serverConfig.workdir_.string());

  if (!workdirPtr || !workdirPtr.get()) {
    LOG(WARNING) << "WSServer::runIocWsListener: Invalid workdirPtr";
    return;
  }

  // Create and launch a listening port
  iocWsListener_ = std::make_shared<WsListener>(ioc_, tcpEndpoint, workdirPtr, nm_);
  if (!iocWsListener_ || !iocWsListener_.get()) {
    LOG(WARNING) << "WSServer::runIocWsListener: Invalid iocWsListener_";
    return;
  }

  iocWsListener_->run();
}

} // namespace net
} // namespace boostander
