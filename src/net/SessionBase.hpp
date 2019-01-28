#pragma once

#include <memory>
#include <string>

namespace boostander {
namespace config {
class ServerConfig;
} // namespace config
} // namespace boostander

namespace boostander {
namespace algo {
class DispatchQueue;
} // namespace algo
} // namespace boostander

namespace boostander {
namespace net {

class SessionBase {
public:
  SessionBase(const std::string& id);

  virtual ~SessionBase() {}

  virtual void send(std::shared_ptr<std::string> ss) = 0;

  virtual void send(const std::string& ss) = 0;

  virtual std::shared_ptr<algo::DispatchQueue> getReceivedMessages() const;

  virtual bool handleIncomingData(const std::shared_ptr<std::string> message) = 0;

  virtual std::string getId() const { return id_; }

  virtual bool hasReceivedMessages() const;

  virtual void clearReceivedMessages() const;

protected:
  const std::string id_;

  std::shared_ptr<algo::DispatchQueue> receivedMessagesQueue_;
};

} // namespace net
} // namespace boostander
