#pragma once

namespace SiddurEngine {

enum class PrayerService {
  Shaharit,
  Minha,
  Arvit,
  Musaf,
};

enum class Weekday {
  Sunday,
  Monday,
  Tuesday,
  Wednesday,
  Thursday,
  Friday,
  Shabbat,
};

struct PrayerContext {
  PrayerService service = PrayerService::Shaharit;
  Weekday weekday = Weekday::Sunday;

  bool isRoshHodesh = false;
  bool isYomTov = false;
  bool isHolHamoed = false;
  bool isFastDay = false;
  bool isHanukkah = false;
  bool isPurim = false;
};

}  // namespace SiddurEngine
