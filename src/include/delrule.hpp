#pragma once

#include <iomanip>
#include <ostream>

#include <cxxopts.hpp>
#include <yaml-cpp/yaml.h>
#include <absl/strings/str_split.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/charconv.h>
#include <absl/time/time.h>
#include <tl/expected.hpp>
#include <dbg.h>
#include <libassert/assert.hpp>

#include "errs.hpp"

namespace drm {

struct DelRule {
  absl::Duration tSt, tEd;
  std::optional<int64_t> iSt, iEd;

  /**
   * @brief
   * @example "(-3d:)[-5:]" select only last 5 items in 3 days before.
   * @example "(:-1m)[:10]" select only first 10 items before 1 month before.
   * @example "(:-1y)[:]" select all items before 1 year.
   * @example "(:)[10:]" select first 10 items in all time.
   * @example "(:)[:]" select all items.
   */
  static tl::expected<DelRule, RunErr> parse_from_string(std::string_view str) {
    auto res = DelRule{};
    if (str.empty()) {
      return tl::unexpected(RunErr{.errType = ErrType::Parse_Error, .prompt = "Failed to parse empty range."});
    }
    if (str.front() != '(' || str.back() != ']') {
      return tl::unexpected(
        RunErr{.errType = ErrType::Parse_Error, .prompt = "DelRule should start with '(' and end with ']'."});
    }
    auto parts = std::vector<std::string_view>(absl::StrSplit(str.substr(1, str.length() - 2), ")["));
    if (parts.size() != 2) {
      return tl::unexpected(RunErr{.errType = ErrType::Parse_Error,
                                   .prompt = absl::StrCat("Failed to split '", str, "' into two parts by ')['.")});
    }
    auto dateRangeStr = parts.front(), selRangeStr = parts.back();
    {
      auto toAbslDurationFormat = [](std::string_view buf) -> tl::expected<std::string, RunErr> {
        auto durStrList = std::vector<std::string_view>(absl::StrSplit(buf, '/'));
        auto durRes = std::map<char, int64_t>();
        for (const auto& durStr: durStrList) {
          auto dur = int64_t{};
          auto durType = durStr.back();
          auto pureDurStr = durStr.substr(0, durStr.size() - 1);
          if (absl::SimpleAtoi(pureDurStr, &dur)) {
            durRes[durType] += dur;
          } else {
            return tl::unexpected(
              RunErr{.errType = ErrType::Parse_Error,
                     .prompt = absl::StrCat("Failed to parse time range '", durStr, "' since parsing duration int '",
                                            pureDurStr, "' failed")});
          }
        }
        auto hDur = int64_t(0), mDur = int64_t(0), sDur = int64_t(0);
        for (const auto& [durType, dur]: durRes) {
          switch (durType) {
            case 'Y': {
              hDur += dur * 12 * 30 * 24;
              break;
            }
            case 'M': {
              hDur += dur * 24 * 30;
              break;
            }
            case 'd':
            case 'D': {
              hDur += dur * 24;
              break;
            }
            case 'h': {
              hDur += dur;
              break;
            }
            case 'm': {
              mDur += dur;
              break;
            }
            case 's': {
              sDur += dur;
              break;
            }
            default: {
              return tl::unexpected(
                tl::unexpected(RunErr{.errType = ErrType::Parse_Error,
                                      .prompt = absl::StrCat("Failed to parse time range, unexpected durType '",
                                                             std::string(1, durType), "' in '", buf, "'.")}));
            }
          }
        }
        return absl::StrCat(hDur, "h", mDur, "m", sDur, "s");
      };
      auto tBuffers = std::vector<std::string_view>(absl::StrSplit(dateRangeStr, ':'));
      if (tBuffers.size() != 2) {
        return tl::unexpected(RunErr{.errType = ErrType::Parse_Error, .prompt = "Failed to split time range."});
      }
      auto tStStr = tBuffers.front(), tEdStr = tBuffers.back();
      if (tStStr.empty()) {
        res.tSt = -absl::InfiniteDuration();
      } else {
        auto e = toAbslDurationFormat(tStStr);
        if (e.has_value()) {
          tStStr = e.value();
        } else {
          return tl::unexpected(e.error());
        }
        auto _x = absl::ParseDuration(tStStr, &res.tSt);
        ASSERT(_x, "This duration is constructed by yourself, so should not parse failed.");
      }
      if (tEdStr.empty()) {
        res.tEd = absl::InfiniteDuration();
      } else {
        auto e = toAbslDurationFormat(tEdStr);
        if (e.has_value()) {
          tEdStr = e.value();
        } else {
          return tl::unexpected(e.error());
        }
        auto _x = absl::ParseDuration(tEdStr, &res.tEd);
        ASSERT(_x, "This duration is constructed by yourself, so should not parse failed.");
      }
    }
    {
      auto tBuffers = std::vector<std::string_view>(absl::StrSplit(selRangeStr, ':'));
      if (tBuffers.size() != 2) {
        return tl::unexpected(RunErr{.errType = ErrType::Parse_Error, .prompt = "Failed to split index range."});
      }
      auto iStStr = tBuffers.front(), iEdStr = tBuffers.back();
      if (iStStr.empty()) {
        res.iSt = std::nullopt;
      } else {
        res.iSt = int64_t{};
        auto e = absl::SimpleAtoi(iStStr, &*res.iSt);
        if (!e) {
          return tl::unexpected(RunErr{.errType = ErrType::Parse_Error,
                                       .prompt = absl::StrCat("Failed to parse index str '", iStStr, "'.")});
        }
      }
      if (iEdStr.empty()) {
        res.iEd = std::nullopt;
      } else {
        res.iEd = int64_t{};
        auto e = absl::SimpleAtoi(iEdStr, &*res.iEd);
        if (!e) {
          return tl::unexpected(RunErr{.errType = ErrType::Parse_Error,
                                       .prompt = absl::StrCat("Failed to parse index str '", iEdStr, "'.")});
        }
      }
    }
    return res;
  }

  friend std::ostream& operator<<(std::ostream& os, const DelRule& delRule) {
    os << "DelRule { tSt = ";
    os << delRule.tSt;
    os << ", tEd = ";
    os << delRule.tEd;
    os << ", iSt = ";
    if (delRule.iSt.has_value()) {
      os << *delRule.iSt;
    } else {
      os << "std::nullopt";
    }
    os << ", iEd = ";
    if (delRule.iEd.has_value()) {
      os << *delRule.iEd;
    } else {
      os << "std::nullopt";
    }
    os << "}";
    return os;
  }
};

}  // namespace drm
