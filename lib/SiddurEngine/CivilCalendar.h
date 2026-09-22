#pragma once

#include "HebrewDate.h"

namespace SiddurEngine {

class CivilCalendar final {
 public:
  [[nodiscard]] static constexpr bool isLeapYear(const int year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
  }

  [[nodiscard]] static constexpr int daysInMonth(const int year, const int month) {
    switch (month) {
      case 2:
        return isLeapYear(year) ? 29 : 28;
      case 4:
      case 6:
      case 9:
      case 11:
        return 30;
      default:
        return 31;
    }
  }

  [[nodiscard]] static constexpr CivilDate nextDate(CivilDate date) {
    if (date.day < daysInMonth(date.year, date.month)) {
      ++date.day;
    } else {
      date.day = 1;
      if (date.month < 12) {
        ++date.month;
      } else {
        date.month = 1;
        ++date.year;
      }
    }
    return date;
  }

  [[nodiscard]] static constexpr CivilDate previousDate(CivilDate date) {
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
    date.day = daysInMonth(date.year, date.month);
    return date;
  }
};

}  // namespace SiddurEngine
