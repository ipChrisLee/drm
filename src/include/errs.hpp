#pragma once

#include <iomanip>
#include <string>
#include <stdint.h>

#include <magic_enum/magic_enum.hpp>

namespace drm {

enum class ErrType : int8_t {
  Unknown_Error = 0,
  Parse_Error,
  Argc_Error,
  Path_Error,
};

struct RunErr {
  ErrType errType;
  std::string prompt;

  friend std::ostream& operator<<(std::ostream& os, const RunErr& runErr) {
    os << "RunErr { errType = " << magic_enum::enum_name(runErr.errType) << ", prompt = " << std::quoted(runErr.prompt)
       << "}";
    return os;
  }
};

[[noreturn]] void exit_on(const RunErr&);


}  // namespace drm
