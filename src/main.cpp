/*
 * Copyright (c) 2018 Denis Trofimov (den.a.trofimov@yandex.ru)
 * Distributed under the MIT License.
 * See accompanying file LICENSE.md or copy at http://opensource.org/licenses/MIT
 */

#include "algo/TickManager.hpp"
#include "config/ServerConfig.hpp"
#include "log/Logger.hpp"
#include "net/NetworkManager.hpp"
#include "storage/path.hpp"
#include <boost/log/core/record.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <chrono>
#include <cstdlib>
#include <filesystem>
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
  using namespace boostander::algo;

  boostander::log::Logger::instance(); // inits Logger

  LOG(INFO) << "Starting server..";

  printNumOfCores();

  const fs::path workdir = boostander::storage::getThisBinaryDirectoryPath();

  const boostander::config::ServerConfig serverConfig(workdir);

  auto nm = std::make_shared<boostander::net::NetworkManager>();

  nm->run(serverConfig);

  // process recieved messages with some period
  TickManager<std::chrono::milliseconds> tm(50ms);

  tm.addTickHandler(TickHandler("handleAllPlayerMessages", [&nm]() {
    // Handle queued incoming messages
    nm->handleIncomingMessages();
  }));

  while (tm.needServerRun()) {
    tm.tick();
  }

  // (If we get here, it means we got a SIGINT or SIGTERM)
  LOG(WARNING) << "If we get here, it means we got a SIGINT or SIGTERM";

  nm->finish();

  return EXIT_SUCCESS;
}
