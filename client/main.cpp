/*
 * Copyright (c) 2018 Denis Trofimov (den.a.trofimov@yandex.ru)
 * Distributed under the MIT License.
 * See accompanying file LICENSE.md or copy at http://opensource.org/licenses/MIT
 */

#include "algo/TickManager.hpp"
#include "config/ServerConfig.hpp"
#include "log/Logger.hpp"
#include "net/NetworkManager.hpp"
#include "net/websockets/WsListener.hpp"
#include "net/websockets/WsServer.hpp"
#include "net/websockets/WsSession.hpp"
#include "storage/path.hpp"
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem; // from <filesystem>

using namespace std::chrono_literals;

static void printNumOfCores() {
  unsigned int c = std::thread::hardware_concurrency();
  LOG(INFO) << "Number of cores: " << c;
  const size_t minCores = 4;
  if (c < minCores) {
    LOG(INFO) << "Too low number of cores! Prefer servers with at least " << minCores << " cores";
  }
}

int main() {
  LOG(INFO) << "Starting client..";

  using namespace boostander::algo;
  using namespace boostander::net;

  size_t WSTickFreq = 100; // 1/Freq
  size_t WSTickNum = 0;

  boostander::log::Logger::instance(); // inits Logger

  printNumOfCores();

  const fs::path workdir = boostander::storage::getThisBinaryDirectoryPath();

  // TODO: support async file read, use futures or std::async
  // NOTE: future/promise Should Not Be Coupled to std::thread Execution Agents
  boostander::config::ServerConfig serverConfig(
      fs::path{workdir / boostander::config::ASSETS_DIR / boostander::config::CONFIGS_DIR /
               boostander::config::CONFIG_NAME},
      workdir);

  // NOTE Tell the socket to bind to port 0, then query the socket for the actual port it bound to
  // if successful.
  serverConfig.wsPort_ = static_cast<unsigned short>(0);

  auto nm = std::make_shared<boostander::net::NetworkManager>(serverConfig);

  // TODO: print active sessions

  // TODO: destroy inactive wrtc sessions (by timer)

  nm->run(serverConfig);

  LOG(INFO) << "Starting server loop for event queue";

  // process recieved messages with some period
  TickManager<std::chrono::milliseconds> tm(50ms);

  // Create the session and run it
  const auto newSessId = "clientSideServerId";
  // boost::asio::ip::tcp::socket& socket_ = nm->getWS()->getWsListener()->socket_;
  /*auto newWsSession =
      std::make_shared<WsSession>(std::move(nm->getWS()->getWsListener()->socket_), nm, newSessId);
  nm->getWS()->addSession(newSessId, newWsSession);*/
  auto newWsSession = nm->getWS()->getWsListener()->addClientSession(newSessId);
  if (!newWsSession || !newWsSession.get()) {
    LOG(WARNING) << "addClientSession failed ";
    nm->finish();
    LOG(WARNING) << "exiting...";
    return EXIT_SUCCESS;
  }

  const std::string hostToConnect = "127.0.0.1";
  const std::string portToConnect = "8080";
  newWsSession->connectAsClient(hostToConnect, portToConnect,
                                tcp::endpoint::protocol_type::tcp::v6());

  bool isConnected = newWsSession->waitForConnect(/* maxWait_ms */ 1000);
  if (!isConnected) {
    LOG(WARNING) << "waitForConnect: Can`t connect to " << hostToConnect << ":" << portToConnect;
    nm->finish();
    LOG(WARNING) << "exiting...";
    return EXIT_SUCCESS;
  }
  newWsSession->runAsClient();

  std::string welcomeMsg = "welcome, ";
  welcomeMsg += newSessId;
  LOG(INFO) << "1111new ws session " << newSessId;
  newWsSession->send(std::make_shared<std::string>(welcomeMsg)); // NOTE: need wait connected state

  tm.addTickHandler(TickHandler("handleAllPlayerMessages", [&nm, &newSessId, &newWsSession]() {
    // TODO: merge responses for same Player (NOTE: packet size limited!)

    // TODO: move game logic to separete thread or service

    // Handle queued incoming messages
    nm->handleIncomingMessages();
    std::string periodicMsg = "0welcome, ";
    periodicMsg += newSessId;
    newWsSession->send(std::make_shared<std::string>(periodicMsg));
  }));

  {
    tm.addTickHandler(TickHandler("WSTick", [&nm, &WSTickFreq, &WSTickNum]() {
      WSTickNum++;
      if (WSTickNum < WSTickFreq) {
        return;
      } else {
        WSTickNum = 0;
      }
      LOG(WARNING) << "WSTick! " << nm->getWS()->getSessionsCount();
      // send test data to all players
      std::chrono::system_clock::time_point nowTp = std::chrono::system_clock::now();
      std::time_t t = std::chrono::system_clock::to_time_t(nowTp);
      std::string msg = "WS server_time: ";
      msg += std::ctime(&t);
      msg += ";Total WS connections:";
      msg += std::to_string(nm->getWS()->getSessionsCount());
      const std::unordered_map<std::string, std::shared_ptr<boostander::net::WsSession>>& sessions =
          nm->getWS()->getSessions();
      msg += ";SESSIONS:[";
      for (auto& it : sessions) {
        std::shared_ptr<boostander::net::WsSession> wss = it.second;
        msg += it.first;
        msg += "=";
        if (!wss || !wss.get()) {
          msg += "EMPTY";
        } else {
          msg += wss->getId();
        }
      }
      msg += "]SESSIONS";

      /*nm->getWS()->sendToAll(msg);
      nm->getWS()->doToAllSessions(
          [&](const std::string& sessId, std::shared_ptr<boostander::net::WsSession> session) {
            if (!session || !session.get()) {
              LOG(WARNING) << "WSTick: Invalid WsSession ";
              return;
            }
            session->send("Your WS id: " + session->getId());
          });*/
    }));
  }

  while (tm.needServerRun()) {
    tm.tick();
  }

  // TODO: sendProcessedMsgs in separate thread

  // (If we get here, it means we got a SIGINT or SIGTERM)
  LOG(WARNING) << "If we get here, it means we got a SIGINT or SIGTERM";

  nm->finish();

  return EXIT_SUCCESS;
}
