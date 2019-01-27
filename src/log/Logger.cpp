#include "log/Logger.hpp" // IWYU pragma: associated
#include "config/ServerConfig.hpp"
#include "storage/path.hpp"
#include <filesystem>
#include <iostream>
#include <memory>

// IWYU pragma: begin_exports
#include <boost/bind.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <boost/log/core/record.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/make_shared.hpp>
#include <boost/phoenix/bind.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/ref.hpp>
#include <boost/shared_ptr.hpp>
// IWYU pragma: end_exports

namespace fs = std::filesystem;

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

namespace boostander {
namespace log {

Logger::Logger() {
  logfile = boost::make_shared<text_sink>();

  const std::string logFileName = "app.log";
  logfile->locked_backend()->add_stream(boost::make_shared<std::ofstream>(logFileName));

  logfile->set_formatter(expr::stream

                         << "["
                         // line id will be 5-digits, zero-filled
                         << std::setw(5) << std::setfill('0') << expr::attr<unsigned int>("LineID")
                         << "|"
                         << expr::format_date_time<boost::posix_time::ptime>("TimeStamp",
                                                                             "%Y-%m-%d %H:%M:%S")
                         << "|" << std::setw(7) << std::setfill(' ') << logging::trivial::severity
                         << "] : " << expr::smessage);

  // Register the logfile in the logging core
  logging::core::get()->add_sink(logfile);

  outputstream = boost::make_shared<ostream_sink>();
  boost::shared_ptr<std::ostream> clog{&std::clog, boost::null_deleter{}};

  outputstream->locked_backend()->add_stream(clog);

  outputstream->set_filter(logging::trivial::severity >= logging::trivial::info

  );

  //		ostream->set_formatter(&logger::custom_formatter);
  outputstream->set_formatter(expr::stream

                              << "[" << std::setw(7) << std::setfill(' ')
                              << logging::trivial::severity << "] : " << expr::smessage);

  // Register the ostream in the logging core
  logging::core::get()->add_sink(outputstream);

  // Add TimeStamp, LineID to log records
  logging::add_common_attributes();
}

Logger::~Logger() {
  boost::shared_ptr<logging::core> core = logging::core::get();

  // Remove the sink from the core, so that no records are passed to it
  core->remove_sink(outputstream);
  core->remove_sink(logfile);

  // Break the feeding loop
  outputstream->stop();
  logfile->stop();

  // Flush all log records that may have left buffered
  outputstream->flush();
  logfile->flush();

  // Delete shared_ptr
  outputstream.reset();
  logfile.reset();
}

Logger& Logger::instance() {
  static Logger instance;
  return instance;
}

} // namespace log
} // namespace boostander
