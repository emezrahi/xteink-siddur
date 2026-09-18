#pragma once

namespace SiddurEngine {

enum class PrayerService {
  Shaharit,
  Minha,
  Arvit,
  Musaf,
};

struct PrayerContext {
  PrayerService service = PrayerService::Shaharit;

  bool isRoshHodesh = false;
  bool isYomTov = false;
  bool isHolHamoed = false;
  bool isFastDay = false;
  bool isHanukkah = false;
  bool isPurim = false;
};

}  // namespace SiddurEngine
