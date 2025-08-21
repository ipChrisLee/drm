#include <chrono>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <algorithm>

#include <cxxopts.hpp>
#include <fmt/format.h>
#include <tl/expected.hpp>
#include <absl/time/time.h>
#include <absl/time/clock.h>
#include <range/v3/all.hpp>

#include "delrule.hpp"
#include "errs.hpp"
#include "range/v3/view/filter.hpp"

namespace fs = std::filesystem;
namespace rv = ranges::views;

struct DrmOptions {
  std::string range = "";
  std::string dir = "";
  bool dry = false;
  bool reverse = false;
};

tl::expected<DrmOptions, drm::RunErr> parse_argv(int argc, char** argv) {
  auto drmOptions = DrmOptions{};
  auto optionsParser = cxxopts::Options("drm", "Rm file by date and time.");
  // clang-format off
  optionsParser.add_options()
    ("h,help", "Print help.")
    ("R,reverse", "Reverse behavior, to keep selected items not to remove items.")
    ("r,range", "Date range. Supported format '(:)[:]'.\n"
                "\t'(:)' is for date and time, for now we only support relative time.\n"
                "\t\tExample: (-1Y:) from 1 year before to now; (:-6M): before 6 months ago.\n"
                "\t'[:]' is for index of folder and files, it is like numpy index rule.\n",
     cxxopts::value<std::string>())
    ("d,dir", "Directory path to rm.", cxxopts::value<std::string>())
    ("dry-run", "Just print what to do, but don't rm files.");
  // clang-format on
  auto options = optionsParser.parse(argc, argv);
  if (options.count("help")) {
    std::cout << optionsParser.help() << std::endl;
    exit(1);
  }
  if (options.count("reverse")) { drmOptions.reverse = true; }
  if (!options.count("range")) {
    return tl::unexpected(drm::RunErr{.errType = drm::ErrType::Argc_Error, .prompt = "-r/--range is required."});
  }
  drmOptions.range = options["range"].as<std::string>();
  if (!options.count("dir")) {
    return tl::unexpected(drm::RunErr{.errType = drm::ErrType::Argc_Error, .prompt = "-d/--dir is required."});
  }
  drmOptions.dir = options["dir"].as<std::string>();
  if (options.count("dry-run")) { drmOptions.dry = options["dry-run"].as<bool>(); }
  return drmOptions;
}

int main(int argc, char** argv) {
  auto drmOptions = ({
    auto eDrmOptions = parse_argv(argc, argv);
    if (!eDrmOptions.has_value()) { drm::exit_on(eDrmOptions.error()); }
    *eDrmOptions;
  });
  auto delRule = ({
    auto eDelRule = drm::DelRule::parse_from_string(drmOptions.range);
    if (!eDelRule.has_value()) { drm::exit_on(eDelRule.error()); }
    *eDelRule;
  });
  dbg(delRule);
  auto dir = fs::path(drmOptions.dir);
  if (!fs::exists(dir)) {
    drm::exit_on(drm::RunErr{.errType = drm::ErrType::Path_Error,
                             .prompt = absl::StrCat("Path ", drmOptions.dir, " not exists.")});
  }
  auto subEntries = std::vector<fs::path>();
  for (const auto& entry: fs::directory_iterator(dir)) { subEntries.emplace_back(entry.path()); }
  std::sort(subEntries.begin(), subEntries.end(), [](const fs::path& lhs, const fs::path& rhs) {
    auto lhsModifyTime = absl::FromTimeT(
      std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        fs::last_write_time(lhs) - fs::file_time_type::clock::now() + std::chrono::system_clock::now())));
    auto rhsModifyTime = absl::FromTimeT(
      std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        fs::last_write_time(rhs) - fs::file_time_type::clock::now() + std::chrono::system_clock::now())));
    return lhsModifyTime < rhsModifyTime;
  });
  dbg(subEntries);
  auto timeNow = absl::Now();
  auto stTime = delRule.tSt + timeNow;
  auto edTime = delRule.tEd + timeNow;
  auto selectedEntries = std::vector<fs::path>();
  for (const auto& entry: subEntries) {
    auto modifyTime = absl::FromTimeT(
      std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        fs::last_write_time(entry) - fs::file_time_type::clock::now() + std::chrono::system_clock::now())));
    if (stTime < modifyTime && modifyTime < edTime) { selectedEntries.emplace_back(entry); }
  }
  const auto L = int64_t(selectedEntries.size());
  auto stIt = ({
    auto st = (delRule.iSt.has_value() ? *delRule.iSt : 0);
    st = (st >= 0 ? st : L + st);
    st = std::max(int64_t(0), st);
    selectedEntries.begin() + st;
  });
  auto edIt = ({
    auto ed = (delRule.iEd.has_value() ? *delRule.iEd : L);
    ed = (ed >= 0 ? ed : L + ed);
    ed = std::max(int64_t(0), ed);
    selectedEntries.begin() + ed;
  });
  dbg((stIt - selectedEntries.begin()));
  dbg((edIt - selectedEntries.begin()));
  if (drmOptions.reverse) {
    if (drmOptions.dry) {
      std::cout << "Dry run. Rm list: ";
      for (auto it = selectedEntries.begin(); it != stIt; ++it) { std::cout << *it << " "; }
      for (auto it = edIt; it != selectedEntries.end(); ++it) { std::cout << *it << " "; }
      std::cout << "\n";
      std::cout << "Dry run. Keep list: ";
      for (auto it = stIt; it != edIt; ++it) { std::cout << *it << " "; }
      std::cout << std::endl;
    } else {
      selectedEntries.erase(selectedEntries.begin(), stIt);
      selectedEntries.erase(edIt, selectedEntries.end());
    }
  } else {
    if (drmOptions.dry) {
      std::cout << "Dry run. Rm list: ";
      for (auto it = stIt; it != edIt; ++it) { std::cout << *it << " "; }
      std::cout << std::endl;
    } else {
      selectedEntries.erase(stIt, edIt);
    }
  }
  return 0;
}
