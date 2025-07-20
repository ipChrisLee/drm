#include "errs.hpp"

#include <iomanip>
#include <iostream>

#include <fmt/color.h>
#include <absl/strings/str_cat.h>

namespace drm {

void exit_on(const RunErr& r) {
  std::cerr << fmt::format(fmt::fg(fmt::color::red),
                           absl::StrCat("Program execution failed, err type ", magic_enum::enum_name(r.errType),
                                        ", detailed reason: '", r.prompt, "'. Run --help for help."))
            << std::endl;
  exit(1);
}

}  // namespace drm
