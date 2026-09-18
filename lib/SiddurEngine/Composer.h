#pragma once

#include <vector>

#include "PrayerBlockId.h"
#include "PrayerContext.h"

namespace SiddurEngine {

class Composer final {
 public:
  [[nodiscard]] static std::vector<PrayerBlockId> compose(const PrayerContext& context) {
    if (context.service != PrayerService::Shaharit) {
      return {};
    }

    return {
        PrayerBlockId::NetilatYadayim,
        PrayerBlockId::AsherYatzar,
        PrayerBlockId::ElohaiNeshama,
    };
  }
};

}  // namespace SiddurEngine
