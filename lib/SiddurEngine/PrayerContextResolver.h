#pragma once

#include "HebrewCalendar.h"
#include "PrayerContext.h"

namespace SiddurEngine {

struct PrayerContextResolution {
  PrayerContext context;
  HebrewDate hebrewDate;
};

class PrayerContextResolver final {
 public:
  [[nodiscard]] static PrayerContextResolution resolve(const PrayerService service, CivilDate civilDate,
                                                       const bool afterSunset) {
    if (afterSunset) {
      civilDate = nextCivilDate(civilDate);
    }

    const HebrewDate hebrewDate = HebrewCalendar::fromGregorian(civilDate);

    PrayerContext context;
    context.service = service;
    context.isRoshHodesh = HebrewCalendar::isRoshHodesh(hebrewDate);

    return {context, hebrewDate};
  }

 private:
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
