#pragma once

#include <functional>
#include <string>
#include <thread>
#include <vector>

namespace boostander {
namespace algo {

using namespace std::chrono_literals;

class TickHandler {
public:
  TickHandler(const std::string& id, std::function<void()> fn) : id_(id), fn_(fn) {}
  const std::string id_;
  const std::function<void()> fn_;
};

template <typename PeriodType> class TickManager {
public:
  TickManager(const PeriodType& serverNetworkUpdatePeriod)
      : serverNetworkUpdatePeriod_(serverNetworkUpdatePeriod) {}

  void tick() {
    std::this_thread::sleep_for(serverNetworkUpdatePeriod_);
    for (const TickHandler& it : tickHandlers_) {
      // LOG(INFO) << "tick() for " << it.id_;
      it.fn_();
    }
  }

  bool needServerRun() const { return needServerRun_; }

  void stop() { needServerRun_ = false; }

  void addTickHandler(const TickHandler& tickHandler) { tickHandlers_.push_back(tickHandler); }

  std::vector<TickHandler> getTickHandlers() const { return tickHandlers_; }

private:
  PeriodType serverNetworkUpdatePeriod_;

  std::vector<TickHandler> tickHandlers_;

  bool needServerRun_ = true;
};
} // namespace algo
} // namespace boostander
