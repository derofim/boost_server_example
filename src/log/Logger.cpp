#include "log/Logger.hpp" // IWYU pragma: associated
#include "config/ServerConfig.hpp"
#include "storage/path.hpp"
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#if !defined(BOOST_WINDOWS)
#define BOOST_LOG_USE_NATIVE_SYSLOG
#endif

// IWYU pragma: begin_exports
#include <boost/bind.hpp>
#include <boost/config.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <boost/log/core/record.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/syslog_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/ipc/object_name.hpp>
#include <boost/log/utility/ipc/reliable_message_queue.hpp>
#include <boost/log/utility/open_mode.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/make_shared.hpp>
#include <boost/phoenix/bind.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/ref.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
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
  boost::shared_ptr<logging::core> core = logging::core::get();

  /*The syslog API (and protocol) doesn't allow applications to specify the way how logs are
   * processed by the log server. For that you have to configure your syslog server. See the
   * documentation for your server (e.g. rsyslog, syslog-ng or journald for logging through the
   * syslog API).*/
  boost::shared_ptr<sinks::syslog_backend> backend(new sinks::syslog_backend(
      keywords::facility = sinks::syslog::local7, /*< the logging facility >*/
      keywords::use_impl = sinks::syslog::native  /*< the native syslog API should be used >*/
      ));

  // Set the straightforward level translator for the "Severity" attribute of type int
  backend->set_severity_mapper(sinks::syslog::direct_severity_mapping<int>("Severity"));

  logfile = boost::make_shared<sink_t>(backend);

  // Wrap it into the frontend and register in the core.
  // The backend requires synchronization in the frontend.
  core->add_sink(logfile);

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
  core->add_sink(logfile);

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
  core->add_sink(outputstream);

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
  // logfile->stop();

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
