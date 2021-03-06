﻿#pragma once

#include "log/Logger.hpp"
#include <boost/log/core/record.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <functional>
#include <mutex>
#include <stddef.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace boostander {
namespace config {
class ServerConfig;
} // namespace config
} // namespace boostander

namespace boostander {
namespace net {

template <typename sessType, typename callbacksType> class SessionManagerBase {
public:
  SessionManagerBase() {}

  virtual ~SessionManagerBase() {}

  /**
   * @example:
   * std::time_t t = std::chrono::system_clock::to_time_t(p);
   * std::string msg = "server_time: ";
   * msg += std::ctime(&t);
   * sm->sendToAll(msg);
   **/
  virtual void sendToAll(const std::string& message) = 0;

  virtual void sendTo(const std::string& sessionID, const std::string& message) = 0;

  virtual void handleIncomingMessages() = 0;

  virtual void
  doToAllSessions(std::function<void(const std::string& sessId, std::shared_ptr<sessType>)> func);

  /**
   * @brief returns the number of connected clients
   *
   * @return number of valid sessions
   */
  virtual size_t getSessionsCount() const {
    std::scoped_lock lock(sessionsMutex_);
    return sessions_.size();
  }

  virtual std::unordered_map<std::string, std::shared_ptr<sessType>> getSessions() const {
    std::scoped_lock lock(sessionsMutex_);
    return sessions_;
  }

  virtual std::shared_ptr<sessType> getSessById(const std::string& sessionID);

  virtual bool addSession(const std::string& sessionID, std::shared_ptr<sessType> sess);

  virtual void unregisterSession(const std::string& id) = 0;

  virtual callbacksType getOperationCallbacks() const { return operationCallbacks_; }

  virtual void runThreads(const boostander::config::ServerConfig& serverConfig) = 0;

  virtual void finishThreads() = 0;

  virtual bool removeSessById(const std::string& sessionID);

protected:
  callbacksType operationCallbacks_;

  // @see https://stackoverflow.com/a/25521702/10904212
  mutable std::mutex sessionsMutex_;

  // Used to map SessionId to Session
  std::unordered_map<std::string, std::shared_ptr<sessType>> sessions_ = {};
};

template <typename sessType, typename callbacksType>
bool SessionManagerBase<sessType, callbacksType>::removeSessById(const std::string& sessionID) {
  {
    std::scoped_lock lock(sessionsMutex_);
    if (!sessions_.erase(sessionID)) {
      LOG(WARNING) << "unregisterSession: trying to unregister non-existing session";
      // NOTE: continue cleanup with saved shared_ptr
      return false;
    }
  }
  return true;
}

template <typename sessType, typename callbacksType>
std::shared_ptr<sessType>
SessionManagerBase<sessType, callbacksType>::getSessById(const std::string& sessionID) {
  {
    std::scoped_lock lock(sessionsMutex_);
    auto it = sessions_.find(sessionID);
    if (it != sessions_.end()) {
      return it->second;
    }
  }

  LOG(WARNING) << "WSServer::getSessById: unknown session with id = " << sessionID;
  return nullptr;
}

/**
 * @brief adds a session to list of valid sessions
 *
 * @param session session to be registered
 */
template <typename sessType, typename callbacksType>
bool SessionManagerBase<sessType, callbacksType>::addSession(const std::string& sessionID,
                                                             std::shared_ptr<sessType> sess) {
  if (!sess || !sess.get()) {
    LOG(WARNING) << "addSession: Invalid session ";
    return false;
  }
  {
    std::scoped_lock lock(sessionsMutex_);
    sessions_[sessionID] = sess;
  }
  return true;
}

/**
 * @example:
 * sm->doToAll([&](std::shared_ptr<WsSession> session) {
 *   session.get()->send("Your id: " + session.get()->getId());
 * });
 **/
template <typename sessType, typename callbacksType>
void SessionManagerBase<sessType, callbacksType>::doToAllSessions(
    std::function<void(const std::string& sessId, std::shared_ptr<sessType>)> func) {
  {
    for (auto& sessionkv : getSessions()) {
      std::shared_ptr<sessType> session = sessionkv.second;
      {
        if (!session || !session.get()) {
          LOG(WARNING) << "doToAllSessions: Invalid session ";
          continue;
        }
        func(sessionkv.first, session);
      }
    }
  }
}

} // namespace net
} // namespace boostander
