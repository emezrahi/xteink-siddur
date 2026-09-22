#pragma once

#include "PrayerContext.h"
#include "Zmanim.h"

namespace SiddurEngine {

class ServiceSelector final {
 public:
  [[nodiscard]] static PrayerService select(const CivilDateTime& local, const DailyZmanim& zmanim) {
    if (!zmanim.valid) return fallback(local.hour * 60 + local.minute);
    const int now = local.hour * 60 + local.minute;
    if (now < zmanim.sunriseMinutes) return PrayerService::Arvit;
    if (now < zmanim.solarNoonMinutes + 30) return PrayerService::Shaharit;
    if (now < zmanim.sunsetMinutes) return PrayerService::Minha;
    return PrayerService::Arvit;
  }

 private:
  [[nodiscard]] static constexpr PrayerService fallback(const int minutes) {
    if (minutes < 6 * 60) return PrayerService::Arvit;
    if (minutes < 13 * 60) return PrayerService::Shaharit;
    if (minutes < 19 * 60) return PrayerService::Minha;
    return PrayerService::Arvit;
  }
};

}  // namespace SiddurEngine
