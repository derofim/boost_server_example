#pragma once

#include <chrono>
#include <string>

namespace boostander {
namespace algo {

std::string trim_whitespace(std::string const& str);

std::string genGuid();

// supported formatting: https://howardhinnant.github.io/date/date.html#to_stream_formatting
std::string currentDateTime();

// supported formatting: https://howardhinnant.github.io/date/date.html#to_stream_formatting
std::chrono::system_clock::time_point
dateTimeFromStr(const std::string& date, const std::string& format = "%d.%m.%Y %H:%M:%S");

// supported formatting: https://howardhinnant.github.io/date/date.html#to_stream_formatting
std::string dateToStr(const std::chrono::system_clock::time_point& tp,
                      const std::string& format = "%d.%m.%Y %H:%M:%S");

} // namespace algo
} // namespace boostander
