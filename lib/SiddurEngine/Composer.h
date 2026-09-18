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

    std::vector<PrayerBlockId> blocks = {
        PrayerBlockId::NetilatYadayim,
        PrayerBlockId::AsherYatzar,
        PrayerBlockId::ElohaiNeshama,
        PrayerBlockId::RetzehOpening,
    };

    if (context.isRoshHodesh) {
      blocks.push_back(PrayerBlockId::YaalehVeyavoRoshHodesh);
    }

    blocks.push_back(PrayerBlockId::RetzehConclusion);
    blocks.push_back(PrayerBlockId::Modim);
    return blocks;
  }
};

}  // namespace SiddurEngine
