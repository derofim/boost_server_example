/*
 * Copyright (c) 2019 Denis Trofimov (den.a.trofimov@yandex.ru)
 * Distributed under the MIT License.
 * See accompanying file LICENSE.md or copy at http://opensource.org/licenses/MIT
 */
#include "algo/CSV.hpp"
#include "algo/NetworkOperation.hpp"
#include "algo/StringUtils.hpp"
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

#include "testsCommon.h"

SCENARIO("loadCSV", "[CSV]") {
  using boostander::algo::CSV;
  CSV csv;
  csv.loadFromMemory("1,2,3,4,5\n"
                     "2,4,6,8,10\n"
                     "3,6,9,12,15\n"
                     "4,8,12,16,20\n"
                     "5,10,15,20,25\n"
                     "6,12,18,24,30\n"
                     "7,14,21,28,35\n"
                     "8,16,24,32,40\n"
                     "9,18,27,36,45\n"
                     "10,20,30,40,50\n");

  CHECK(csv.getRowsCount() == 10);
  CHECK(csv.getColsCount() == 5);

  CHECK(csv.getRow(0).at(0) == "1");
  CHECK(csv.getRow(0).at(1) == "2");
  CHECK(csv.getRow(0).at(2) == "3");
  CHECK(csv.getRow(0).at(3) == "4");
  CHECK(csv.getRow(0).at(4) == "5");

  CHECK(csv.getAt(0, 0) == "1");
  CHECK(csv.getAt(1, 1) == "4");

  CHECK(csv.getAt(0, 4) == "5");

  CHECK(csv.getAt(9, 0) == "10");
  CHECK(csv.getAt(9, 4) == "50");

  CHECK(csv.setAt(9, 0, "G") == "G");
  CHECK(csv.setAt(9, 4, "ABRA") == "ABRA");

  CHECK(csv.getAt(9, 0) == "G");
  CHECK(csv.getAt(9, 1) == "20");
  CHECK(csv.getAt(9, 2) == "30");
  CHECK(csv.getAt(9, 3) == "40");
  CHECK(csv.getAt(9, 4) == "ABRA");

  csv.appendRow(std::vector<std::string>{"9", "8", "7", "6", "5\n"});
  CHECK(csv.getRowsCount() == 11);
  CHECK(csv.getAt(10, 0) == "9");
  CHECK(csv.getAt(10, 1) == "8");
  CHECK(csv.getAt(10, 2) == "7");
  CHECK(csv.getAt(10, 3) == "6");
  CHECK(csv.getAt(10, 4) == "5\n");
}

SCENARIO("chrono", "[stringUtils]") {
  using namespace boostander::algo;

  CHECK(dateToStr(dateTimeFromStr("20.10.1995 22:15:14")) == "20.10.1995 22:15:14");

  {
    auto start_time = dateTimeFromStr("20.10.1995 22:15:14");
    auto end_time = dateTimeFromStr("20.10.1995 22:15:15");

    CHECK(chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count() == 1000);

    CHECK(end_time > start_time);

    end_time += std::chrono::hours(5);

    CHECK(chrono::duration_cast<chrono::hours>(end_time - start_time).count() == 5);
  }
}

SCENARIO("Opcodes", "[Opcodes]") {
  using namespace boostander::algo;
  using namespace std;

  CHECK(Opcodes::opcodeToStr(WS_OPCODE::PING) == "0");
  CHECK(Opcodes::wsOpcodeFromStr("0") == WS_OPCODE::PING);

  CHECK(Opcodes::opcodeToStr(WS_OPCODE::CSV_ANALIZE) == "1");
  CHECK(Opcodes::wsOpcodeFromStr("1") == WS_OPCODE::CSV_ANALIZE);

  CHECK(Opcodes::opcodeToStr(WS_OPCODE::CSV_ANSWER) == "2");
  CHECK(Opcodes::wsOpcodeFromStr("2") == WS_OPCODE::CSV_ANSWER);
}

SCENARIO("randomCSV", "[CSV]") {
  using namespace boostander::algo;
  using namespace std;
  using namespace Catch;
  using namespace Catch::literals;

  std::locale::global(std::locale::classic()); // https://stackoverflow.com/a/18981514/10904212

  CSV csv;
  csv.setRowNum(0);
  csv.setColNum(3);

  mt19937 randomGenerator(time(0));
  uniform_int_distribution<int> iRoll(1, 1000);
  uniform_real_distribution<double> dRoll(1, 1000);

  uniform_int_distribution<int> nRoll(1, 100);
  const auto rowNum = nRoll(randomGenerator);

  auto now = std::chrono::system_clock::now();

  for (int i = 0; i < rowNum; i++) {
    now += std::chrono::hours(iRoll(randomGenerator));
    double r0 = dRoll(randomGenerator);
    double r1 = dRoll(randomGenerator);
    auto d0 = boost::str(boost::format("%.9f") % r0);
    auto d1 = boost::str(boost::format("%.9f") % r1);
    CHECK(boost::lexical_cast<double>(d0) == Approx(r0));
    CHECK(boost::lexical_cast<double>(d1) == Approx(r1));
    CHECK(dateToStr(now) == dateToStr(dateTimeFromStr(dateToStr(now))));
    csv.appendRow(std::vector<std::string>{dateToStr(now), d0, d1 + "\n"});
  }

  // may be used to generate test .csv files
  // format: test_data_28.01.2019 01:21:58
  // csv.saveToFile("test_data_" + currentDateTime("%d_%m_%Y") + ".csv");
}
