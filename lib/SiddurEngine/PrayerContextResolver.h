#pragma once

#include "HebrewCalendar.h"
#include "LiturgicalRules.h"
#include "PrayerContext.h"
#include "ServiceSelector.h"

namespace SiddurEngine {

struct PrayerContextResolution {
  PrayerContext context;
  HebrewDate hebrewDate;
};

class PrayerContextResolver final {
 public:
  [[nodiscard]] static PrayerContextResolution resolve(const PrayerService service, CivilDate civilDate,
                                                       const bool afterSunset, const bool diaspora = true) {
    const CivilDate prayerCivilDate = civilDate;
    if (afterSunset) {
      civilDate = nextCivilDate(civilDate);
    }

    const HebrewDate hebrewDate = HebrewCalendar::fromGregorian(civilDate);

    PrayerContext context;
    context.service = service;
    context.weekday = weekdayFromCivilDate(civilDate);
    LiturgicalRules::apply(context, hebrewDate, prayerCivilDate, diaspora);

    return {context, hebrewDate};
  }

  [[nodiscard]] static PrayerContextResolution resolve(const CivilDateTime& local, const LocationConfig& location) {
    const DailyZmanim zmanim = Zmanim::calculate(local.date, location);
    const PrayerService service = ServiceSelector::select(local, zmanim);
    const bool afterSunset = zmanim.valid && local.hour * 60 + local.minute >= zmanim.sunsetMinutes;
    return resolve(service, local.date, afterSunset, location.diaspora);
  }

 private:
  [[nodiscard]] static constexpr Weekday weekdayFromCivilDate(CivilDate date) {
    // Gregorian weekday using Sakamoto's algorithm. 0=Sunday.
    constexpr int monthOffsets[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    int year = date.year;
    if (date.month < 3) {
      --year;
    }

    const int dayIndex = (year + year / 4 - year / 100 + year / 400 + monthOffsets[date.month - 1] + date.day) % 7;
    return static_cast<Weekday>(dayIndex);
  }

  [[nodiscard]] static constexpr bool isGregorianLeapYear(const int year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
  }

  [[nodiscard]] static constexpr int daysInGregorianMonth(const int year, const int month) {
    switch (month) {
      case 2:
        return isGregorianLeapYear(year) ? 29 : 28;
      case 4:
      case 6:
      case 9:
      case 11:
        return 30;
      default:
        return 31;
    }
  }

  [[nodiscard]] static constexpr CivilDate nextCivilDate(CivilDate date) {
    if (date.day < daysInGregorianMonth(date.year, date.month)) {
      ++date.day;
      return date;
    }

    date.day = 1;
    if (date.month < 12) {
      ++date.month;
      return date;
    }

    date.month = 1;
    ++date.year;
    return date;
  }
};

}  // namespace SiddurEngine
