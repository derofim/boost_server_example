#pragma once

#include "log/Logger.hpp"
#include <string>

namespace boostander {
namespace algo {

enum class WS_OPCODE_ENUM : uint32_t { PING = 48, TOTAL };

class Opcodes {
public:
  using WS_OPCODE = WS_OPCODE_ENUM;

  static std::string opcodeToStr(const WS_OPCODE& code);

  static WS_OPCODE wsOpcodeFromStr(const std::string& str);
};

using WS_OPCODE = Opcodes::WS_OPCODE;

template <typename T> struct NetworkOperation {
  NetworkOperation(const T& operationCode, const std::string& operationName)
      : operationCode_(operationCode), operationCodeStr_(Opcodes::opcodeToStr(operationCode)),
        operationName_(operationName) {}

  NetworkOperation(const T& operationCode)
      : operationCode_(operationCode), operationCodeStr_(Opcodes::opcodeToStr(operationCode)),
        operationName_("") {}

  bool operator<(const NetworkOperation& rhs) const { return operationCode_ < rhs.operationCode_; }

  const T operationCode_;

  const std::string operationCodeStr_;

  /**
   * operationName usefull for logging
   * NOTE: operationName may be empty
   **/
  const std::string operationName_;
};

} // namespace algo
} // namespace boostander
