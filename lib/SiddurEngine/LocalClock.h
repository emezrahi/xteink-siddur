#pragma once

#include "CivilDateTime.h"

namespace SiddurEngine {

class LocalClock final {
 public:
  [[nodiscard]] static constexpr CivilDateTime applyUtcOffset(CivilDateTime utc, uint8_t utcOffsetQuarterHoursBiased) {
    if (utcOffsetQuarterHoursBiased > 104) {
      utcOffsetQuarterHoursBiased = 104;
    }

    const int offsetQuarterHours = static_cast<int>(utcOffsetQuarterHoursBiased) - 48;
    int totalMinutes = utc.hour * 60 + utc.minute + offsetQuarterHours * 15;

    while (totalMinutes < 0) {
      totalMinutes += 1440;
      utc.date = previousCivilDate(utc.date);
    }
    while (totalMinutes >= 1440) {
      totalMinutes -= 1440;
      utc.date = nextCivilDate(utc.date);
    }

    utc.hour = totalMinutes / 60;
    utc.minute = totalMinutes % 60;
    return utc;
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

  [[nodiscard]] static constexpr CivilDate previousCivilDate(CivilDate date) {
    if (date.day > 1) {
      --date.day;
      return date;
    }

    if (date.month > 1) {
      --date.month;
    } else {
      date.month = 12;
      --date.year;
    }

    date.day = daysInGregorianMonth(date.year, date.month);
    return date;
  }
};

}  // namespace SiddurEngine
