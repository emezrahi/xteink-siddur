#pragma once

namespace SiddurEngine {

struct CivilDate {
  int year;
  int month;
  int day;
};

// Hebrew months use the conventional Nisan-based numbering used by calendar
// arithmetic: Nisan=1 through Adar=12, with AdarII=13 in leap years.
// In a leap year, month 12 is Adar I; in a common year it is simply Adar.
enum class HebrewMonth : int {
  Nisan = 1,
  Iyar = 2,
  Sivan = 3,
  Tammuz = 4,
  Av = 5,
  Elul = 6,
  Tishrei = 7,
  Cheshvan = 8,
  Kislev = 9,
  Tevet = 10,
  Shevat = 11,
  Adar = 12,
  AdarII = 13,
};

struct HebrewDate {
  int year;
  HebrewMonth month;
  int day;
};

}  // namespace SiddurEngine
