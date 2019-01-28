#pragma once

#include <boost/log/sinks.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/formatting_ostream.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <ios>
#include <memory>
#include <sstream>
#include <string>

#define LOG(lvl) BOOST_LOG_TRIVIAL(lvl)
#define WARNING warning
#define INFO info

namespace boostander {
namespace log {

// typedef boost::log::sinks::asynchronous_sink<boost::log::sinks::text_ostream_backend> text_sink;

typedef boost::log::sinks::asynchronous_sink<boost::log::sinks::text_ostream_backend> ostream_sink;

typedef boost::log::sinks::synchronous_sink<boost::log::sinks::syslog_backend> sink_t;

class Logger {
public:
  Logger();

  ~Logger();

  static Logger& instance();

private:
  void log(std::string text, boost::log::trivial::severity_level sevLevel);

  boost::log::sources::severity_logger<boost::log::trivial::severity_level> severityLogger;

  boost::shared_ptr<ostream_sink> outputstream;
  boost::shared_ptr<sink_t> logfile;
};

} // namespace log
} // namespace boostander
