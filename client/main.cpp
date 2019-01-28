/*
 * Copyright (c) 2018 Denis Trofimov (den.a.trofimov@yandex.ru)
 * Distributed under the MIT License.
 * See accompanying file LICENSE.md or copy at http://opensource.org/licenses/MIT
 */

#include "algo/CSV.hpp"
#include "algo/StringUtils.hpp"
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

int main(int argc, char** argv) {
  using namespace boostander::algo;
  using namespace boostander::net;
  using namespace boostander::storage;

  std::locale::global(std::locale::classic()); // https://stackoverflow.com/a/18981514/10904212

  boostander::log::Logger::instance(); // inits Logger

  LOG(INFO) << "Starting client..";

  // Check command line arguments.
  if (argc < 2) {
    LOG(WARNING) << "Got argc = " << std::to_string(argc) << " \n"
                 << "Got argv[1] = " << (argc > 1 ? argv[1] : "NULL") << " \n"
                 << "Usage: websocket-client-async <csvFilePath>\n"
                 << "Example:\n"
                 << "    boostander_client data/test_data_28.01.2019.csv\n";
    return EXIT_FAILURE;
  }

  auto const csvFilePath = argv[1];

  std::filesystem::path path(csvFilePath); // Construct the path from a string.
  if (path.is_relative()) {
    LOG(INFO) << "Detected relative filepath..";
    const fs::path workdir = boostander::storage::getThisBinaryDirectoryPath();
    path = std::filesystem::path(workdir / csvFilePath);
    LOG(INFO) << "New filepath = " << path.c_str();
  }

  const std::string csvContents = getFileContents(path);

  // check csv format
  {
    CSV csv;
    csv.loadFromMemory(csvContents);
    if (csv.getRowsCount() <= 0 || csv.getColsCount() <= 0) {
      LOG(WARNING) << "Provided invalid csv file\n"
                   << "See example .csv files in assets folder\n";
    }
  }

  size_t WSTickFreq = 100; // 1/Freq
  size_t WSTickNum = 0;

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

  // NOTE: need wait connected state
  LOG(WARNING) << "connecting to " << hostToConnect << ":" << portToConnect << "...";
  bool isConnected = newWsSession->waitForConnect(/* maxWait_ms */ 1000);
  if (!isConnected) {
    LOG(WARNING) << "waitForConnect: Can`t connect to " << hostToConnect << ":" << portToConnect;
    nm->finish();
    LOG(WARNING) << "exiting...";
    return EXIT_SUCCESS;
  }

  newWsSession->runAsClient();

  // const std::string msg = static_cast<char>(WS_OPCODE::CSV_ANALIZE) + csvContents;
  const std::string msg = Opcodes::opcodeToStr(WS_OPCODE::CSV_ANALIZE) + csvContents;
  newWsSession->send(std::make_shared<std::string>(msg));

  tm.addTickHandler(TickHandler("handleAllPlayerMessages", [&nm, &newSessId, &newWsSession]() {
    // TODO: merge responses for same Player (NOTE: packet size limited!)

    // TODO: move game logic to separete thread or service

    // Handle queued incoming messages
    nm->handleIncomingMessages();
    /*std::string periodicMsg = "0welcome, ";
    periodicMsg += newSessId;
    newWsSession->send(std::make_shared<std::string>(periodicMsg));*/
  }));

  while (tm.needServerRun()) {
    tm.tick();
  }

  // TODO: sendProcessedMsgs in separate thread

  // (If we get here, it means we got a SIGINT or SIGTERM)
  LOG(WARNING) << "If we get here, it means we got a SIGINT or SIGTERM";

  nm->finish();

  return EXIT_SUCCESS;
}
