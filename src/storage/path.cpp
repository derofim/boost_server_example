#include "storage/path.hpp" // IWYU pragma: associated
#include "log/Logger.hpp"
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/dll.hpp>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>

namespace fs = std::filesystem;

namespace boostander {
namespace storage {

/**
 * Get absolute path of currently running binary.
 *
 * @return absolute path
 */
fs::path getThisBinaryPath() { return boost::dll::program_location().c_str(); }

/**
 * Get absolute path to directory of currently running binary.
 *
 * @return absolute path
 */
fs::path getThisBinaryDirectoryPath() {
  return boost::dll::program_location().parent_path().c_str();
}

std::string getFileContents(const fs::path& path) {
  if (path.empty()) {
    // TODO: log
    LOG(WARNING) << "getFileContents: path.empty for " << path.c_str();
    return "";
  }

  if (!fs::exists(path)) {
    LOG(WARNING) << "getFileContents: !fs::exists(path) for " << path.c_str();
    // TODO: log
    return "";
  }

  // Open the stream to 'lock' the file.
  std::ifstream f{path};

  // Obtain the size of the file.
  const auto sz = fs::file_size(path);

  // Create a buffer.
  std::string result(sz, ' ');

  // Read the whole file into the buffer.
  f.read(result.data(), sz);

  return result;
}

} // namespace storage
} // namespace boostander
