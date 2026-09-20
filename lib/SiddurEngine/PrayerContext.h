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

enum class HallelKind { None, Half, Full };
enum class RainMention { MoridHatal, MashivHaruach };
enum class RainRequest { Barechenu, BarechAleinu };

struct PrayerContext {
  PrayerService service = PrayerService::Shaharit;
  Weekday weekday = Weekday::Sunday;

  bool isRoshHodesh = false;
  bool isYomTov = false;
  bool isHolHamoed = false;
  bool isFastDay = false;
  bool isHanukkah = false;
  bool isPurim = false;
  bool sayTachanun = true;
  bool sayYaalehVeyavo = false;
  bool sayAlHanissim = false;
  bool sayMussaf = false;
  bool sayAneinu = false;
  bool isSefiratHaOmer = false;
  int omerDay = 0;
  HallelKind hallel = HallelKind::None;
  RainMention rainMention = RainMention::MoridHatal;
  RainRequest rainRequest = RainRequest::Barechenu;
};

}  // namespace SiddurEngine
