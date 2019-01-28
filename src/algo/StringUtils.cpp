#include "algo/StringUtils.hpp" // IWYU pragma: associated
#include <boost/lexical_cast.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

namespace boostander {
namespace algo {

std::string trim_whitespace(std::string const& str) {
  const std::string whitespace = " \t\f\v\n\r";
  auto start = str.find_first_not_of(whitespace);
  auto end = str.find_last_not_of(whitespace);
  if (start == std::string::npos || end == std::string::npos) {
    return std::string();
  }
  return str.substr(start, end + 1);
}

boost::uuids::uuid genBoostGuid() {
  // return sm->maxSessionId() + 1; // TODO: collision
  boost::uuids::random_generator generator;
  return generator();
}

std::string genGuid() { return boost::lexical_cast<std::string>(genBoostGuid()); }

std::string currentDateTime(const std::string& format) {
  auto now = std::chrono::system_clock::now();
  return dateToStr(now);
}

std::chrono::system_clock::time_point dateTimeFromStr(const std::string& date,
                                                      const std::string& format) {
  std::tm tm = {};
  std::stringstream ss(date);
  ss >> std::get_time(&tm, format.c_str());
  return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::string dateToStr(const std::chrono::system_clock::time_point& tp, const std::string& format) {
  std::stringstream ss;
  auto in_time_t = std::chrono::system_clock::to_time_t(tp);
  ss << std::put_time(std::localtime(&in_time_t), format.c_str());
  return ss.str();
}

} // namespace algo
} // namespace boostander
