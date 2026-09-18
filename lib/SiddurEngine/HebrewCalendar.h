#pragma once

#include <cstdint>

#include "HebrewDate.h"

namespace SiddurEngine {

class HebrewCalendar final {
 public:
  [[nodiscard]] static constexpr bool isLeapYear(const int year) { return ((7 * year + 1) % 19) < 7; }

  [[nodiscard]] static HebrewDate fromGregorian(const CivilDate& civilDate) {
    const int64_t rd = gregorianToRataDie(civilDate);

    int year = static_cast<int>(((rd - kHebrewEpoch) * 98496) / 35975351) + 1;
    while (rd >= hebrewNewYearRataDie(year + 1)) {
      ++year;
    }
    while (rd < hebrewNewYearRataDie(year)) {
      --year;
    }

    int month = rd >= hebrewDateToRataDie(year, 1, 1) ? 1 : 7;
    while (rd > hebrewDateToRataDie(year, month, daysInMonth(year, static_cast<HebrewMonth>(month)))) {
      ++month;
    }

    const int day = static_cast<int>(rd - hebrewDateToRataDie(year, month, 1) + 1);
    return {year, static_cast<HebrewMonth>(month), day};
  }

  [[nodiscard]] static constexpr bool isRoshHodesh(const HebrewDate& date) { return date.day == 1 || date.day == 30; }

  [[nodiscard]] static int daysInYear(const int year) {
    return static_cast<int>(hebrewNewYearRataDie(year + 1) - hebrewNewYearRataDie(year));
  }

  [[nodiscard]] static int daysInMonth(const int year, const HebrewMonth month) {
    const int monthNumber = static_cast<int>(month);

    if (monthNumber == 2 || monthNumber == 4 || monthNumber == 6 || monthNumber == 10 || monthNumber == 13) {
      return 29;
    }
    if (monthNumber == 12 && !isLeapYear(year)) {
      return 29;
    }
    if (monthNumber == 8 && daysInYear(year) % 10 != 5) {
      return 29;
    }
    if (monthNumber == 9 && daysInYear(year) % 10 == 3) {
      return 29;
    }
    return 30;
  }

 private:
  // Rata Die day 1 is Gregorian 0001-01-01. The Hebrew epoch is the
  // conventional epoch used by the fixed Hebrew calendar arithmetic.
  static constexpr int64_t kHebrewEpoch = -1373429;

  [[nodiscard]] static constexpr bool isGregorianLeapYear(const int year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
  }

  [[nodiscard]] static constexpr int monthsInYear(const int year) { return isLeapYear(year) ? 13 : 12; }

  [[nodiscard]] static int64_t gregorianToRataDie(const CivilDate& date) {
    const int64_t priorYears = static_cast<int64_t>(date.year) - 1;
    int64_t rd = 365 * priorYears + priorYears / 4 - priorYears / 100 + priorYears / 400;
    rd += (367 * static_cast<int64_t>(date.month) - 362) / 12;

    if (date.month > 2) {
      rd += isGregorianLeapYear(date.year) ? -1 : -2;
    }

    return rd + date.day;
  }

  [[nodiscard]] static int64_t firstPostponement(const int year) {
    const int64_t monthsElapsed = (235LL * year - 234) / 19;
    const int64_t partsElapsed = 12084 + 13753 * monthsElapsed;
    int64_t day = 29 * monthsElapsed + partsElapsed / 25920;

    if ((3 * (day + 1)) % 7 < 3) {
      ++day;
    }
    return day;
  }

  [[nodiscard]] static int secondPostponement(const int year) {
    const int64_t previous = firstPostponement(year - 1);
    const int64_t current = firstPostponement(year);
    const int64_t next = firstPostponement(year + 1);

    if (next - current == 356) {
      return 2;
    }
    if (current - previous == 382) {
      return 1;
    }
    return 0;
  }

  [[nodiscard]] static int64_t hebrewNewYearRataDie(const int year) {
    return kHebrewEpoch + firstPostponement(year) + secondPostponement(year) + 2;
  }

  [[nodiscard]] static int64_t hebrewDateToRataDie(const int year, const int month, const int day) {
    int64_t rd = hebrewNewYearRataDie(year) + day - 1;

    if (month < 7) {
      for (int currentMonth = 7; currentMonth <= monthsInYear(year); ++currentMonth) {
        rd += daysInMonth(year, static_cast<HebrewMonth>(currentMonth));
      }
      for (int currentMonth = 1; currentMonth < month; ++currentMonth) {
        rd += daysInMonth(year, static_cast<HebrewMonth>(currentMonth));
      }
    } else {
      for (int currentMonth = 7; currentMonth < month; ++currentMonth) {
        rd += daysInMonth(year, static_cast<HebrewMonth>(currentMonth));
      }
    }

    return rd;
  }
};

}  // namespace SiddurEngine
