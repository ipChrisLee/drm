#include <iostream>
#include <optional>

#include <cxxopts.hpp>
#include <yaml-cpp/yaml.h>
#include <absl/strings/str_split.h>
#include <absl/time/time.h>

void parse_argv() {
  auto options = cxxopts::Options("drm", "Rm file by date adn time.");
}

struct DelRule {
  std::optional<absl::Time> tSt, tEd;
  std::optional<int64_t> cSt, cEd;

  static DelRule parse_from_string() {
    auto res = DelRule{};
    return res;
  }
};

int main(int argc, char ** argv) {
  return 0;
}
