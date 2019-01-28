#include "algo/NetworkOperation.hpp" // IWYU pragma: associated
#include <cinttypes>
#include <cstdint>

namespace boostander {
namespace algo {

std::string Opcodes::opcodeToStr(const WS_OPCODE& code) {
  // return std::to_string(static_cast<uint32_t>(code));
  return std::string(1, static_cast<char>(code)); // std::to_string(static_cast<char>(code));
}

WS_OPCODE Opcodes::wsOpcodeFromStr(const std::string& str) {
  // TODO str -> uint32_t: to separate UTILS file
  // see https://stackoverflow.com/a/5745454/10904212
  /*uint32_t type; // NOTE: on change: don`t forget about UINT32_FIELD_MAX_LEN
  sscanf(str.c_str(), "%" SCNu32, &type);
  return static_cast<WS_OPCODE>(type);*/

  char c = str[0];
  return static_cast<WS_OPCODE>(c);
}

} // namespace algo
} // namespace boostander
