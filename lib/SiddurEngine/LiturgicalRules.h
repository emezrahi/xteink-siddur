#pragma once

#include "CivilCalendar.h"
#include "HebrewCalendar.h"
#include "PrayerContext.h"

namespace SiddurEngine {

class LiturgicalRules final {
 public:
  static void apply(PrayerContext& context, const HebrewDate& date, const CivilDate civilDate, const bool diaspora) {
    context.isRoshHodesh = HebrewCalendar::isRoshHodesh(date);
    context.isHanukkah = isHanukkah(date);
    context.isPurim = isPurim(date);
    context.isFastDay = isMinorFast(date, context.weekday);
    context.isYomTov = isYomTov(date, diaspora);
    context.isHolHamoed = isHolHamoed(date, diaspora);
    context.sayYaalehVeyavo = context.isRoshHodesh || context.isYomTov || context.isHolHamoed;
    context.sayAlHanissim = context.isHanukkah || context.isPurim;
    context.sayMussaf = context.isRoshHodesh || context.isYomTov || context.isHolHamoed ||
                        context.weekday == Weekday::Shabbat;
    context.sayAneinu = context.isFastDay && (context.service == PrayerService::Shaharit ||
                                              context.service == PrayerService::Minha);
    context.hallel = hallel(date, diaspora);
    context.sayTachanun = shouldSayTachanun(date, context, diaspora);
    context.omerDay = omerDay(date);
    context.isSefiratHaOmer = context.omerDay != 0;
    context.rainMention = rainMention(date, context.service);
    context.rainRequest = rainRequest(date, civilDate, context.service, diaspora);
  }

 private:
  [[nodiscard]] static bool isYomTov(const HebrewDate& date, const bool diaspora) {
    switch (date.month) {
      case HebrewMonth::Tishrei:
        return date.day <= 2 || date.day == 10 || date.day == 15 || (diaspora && date.day == 16) ||
               date.day == 22 || (diaspora && date.day == 23);
      case HebrewMonth::Nisan:
        return date.day == 15 || (diaspora && date.day == 16) || date.day == 21 || (diaspora && date.day == 22);
      case HebrewMonth::Sivan:
        return date.day == 6 || (diaspora && date.day == 7);
      default:
        return false;
    }
  }

  [[nodiscard]] static bool isHolHamoed(const HebrewDate& date, const bool diaspora) {
    if (date.month == HebrewMonth::Tishrei) return date.day >= (diaspora ? 17 : 16) && date.day <= 21;
    if (date.month == HebrewMonth::Nisan) return date.day >= (diaspora ? 17 : 16) && date.day <= 20;
    return false;
  }

  [[nodiscard]] static bool isHanukkah(const HebrewDate& date) {
    if (date.month == HebrewMonth::Kislev) return date.day >= 25;
    if (date.month != HebrewMonth::Tevet) return false;
    return date.day <= (HebrewCalendar::daysInMonth(date.year, HebrewMonth::Kislev) == 30 ? 2 : 3);
  }

  [[nodiscard]] static bool isPurim(const HebrewDate& date) {
    const HebrewMonth purimMonth = HebrewCalendar::isLeapYear(date.year) ? HebrewMonth::AdarII : HebrewMonth::Adar;
    return date.month == purimMonth && (date.day == 14 || date.day == 15);
  }

  [[nodiscard]] static bool isMinorFast(const HebrewDate& date, const Weekday weekday) {
    if (date.month == HebrewMonth::Tevet && date.day == 10) return true;
    if (date.month == HebrewMonth::Tishrei &&
        ((date.day == 3 && weekday != Weekday::Shabbat) || (date.day == 4 && weekday == Weekday::Sunday))) {
      return true;
    }
    if (date.month == HebrewMonth::Tammuz &&
        ((date.day == 17 && weekday != Weekday::Shabbat) || (date.day == 18 && weekday == Weekday::Sunday))) {
      return true;
    }
    if (date.month == HebrewMonth::Av &&
        ((date.day == 9 && weekday != Weekday::Shabbat) || (date.day == 10 && weekday == Weekday::Sunday))) {
      return true;
    }

    const HebrewMonth purimMonth = HebrewCalendar::isLeapYear(date.year) ? HebrewMonth::AdarII : HebrewMonth::Adar;
    if (date.month != purimMonth) return false;
    if (date.day == 13 && weekday != Weekday::Friday && weekday != Weekday::Shabbat) return true;
    return (date.day == 11 || date.day == 12) && weekday == Weekday::Thursday;
  }

  [[nodiscard]] static HallelKind hallel(const HebrewDate& date, const bool diaspora) {
    if (isHanukkah(date)) return HallelKind::Full;
    if (date.month == HebrewMonth::Sivan && (date.day == 6 || (diaspora && date.day == 7))) return HallelKind::Full;
    if (date.month == HebrewMonth::Tishrei && date.day >= 15 && date.day <= (diaspora ? 23 : 22)) {
      return HallelKind::Full;
    }
    if (date.month == HebrewMonth::Nisan) {
      if (date.day == 15 || (diaspora && date.day == 16)) return HallelKind::Full;
      if (date.day >= (diaspora ? 17 : 16) && date.day <= (diaspora ? 22 : 21)) return HallelKind::Half;
    }
    return HebrewCalendar::isRoshHodesh(date) ? HallelKind::Half : HallelKind::None;
  }

  [[nodiscard]] static bool shouldSayTachanun(const HebrewDate& date, const PrayerContext& context,
                                               const bool diaspora) {
    if (context.weekday == Weekday::Shabbat || context.isRoshHodesh || context.isYomTov || context.isHolHamoed ||
        context.isHanukkah || context.isPurim) {
      return false;
    }
    if (date.month == HebrewMonth::Nisan) return false;
    if (date.month == HebrewMonth::Iyar && (date.day == 14 || date.day == 18)) return false;
    if (date.month == HebrewMonth::Sivan && date.day <= (diaspora ? 13 : 12)) return false;
    if (date.month == HebrewMonth::Av && date.day == 15) return false;
    if (date.month == HebrewMonth::Tishrei && (date.day == 9 || date.day >= 11)) return false;
    if (date.month == HebrewMonth::Shevat && date.day == 15) return false;
    const HebrewMonth purimMonth = HebrewCalendar::isLeapYear(date.year) ? HebrewMonth::AdarII : HebrewMonth::Adar;
    return !(date.month == purimMonth && (date.day == 13 || date.day == 14 || date.day == 15));
  }

  [[nodiscard]] static int omerDay(const HebrewDate& date) {
    if (date.month == HebrewMonth::Nisan && date.day >= 16) return date.day - 15;
    if (date.month == HebrewMonth::Iyar) return date.day + 15;
    if (date.month == HebrewMonth::Sivan && date.day <= 5) return date.day + 44;
    return 0;
  }

  [[nodiscard]] static RainMention rainMention(const HebrewDate& date, const PrayerService service) {
    if (date.month == HebrewMonth::Tishrei) {
      if (date.day > 22 || (date.day == 22 && service == PrayerService::Musaf)) return RainMention::MashivHaruach;
      return RainMention::MoridHatal;
    }
    if (date.month == HebrewMonth::Nisan) {
      if (date.day < 15 || (date.day == 15 && service != PrayerService::Musaf)) return RainMention::MashivHaruach;
      return RainMention::MoridHatal;
    }
    const int month = static_cast<int>(date.month);
    return month >= static_cast<int>(HebrewMonth::Cheshvan) ? RainMention::MashivHaruach : RainMention::MoridHatal;
  }

  [[nodiscard]] static RainRequest rainRequest(const HebrewDate& date, const CivilDate civilDate,
                                               const PrayerService service, const bool diaspora) {
    if (date.month == HebrewMonth::Nisan && date.day >= 15) return RainRequest::Barechenu;
    if (!diaspora) {
      if (date.month == HebrewMonth::Cheshvan && date.day >= 7) return RainRequest::BarechAleinu;
      const int month = static_cast<int>(date.month);
      return month > static_cast<int>(HebrewMonth::Cheshvan)
                 ? RainRequest::BarechAleinu
                 : RainRequest::Barechenu;
    }

    if (civilDate.month < 4) return RainRequest::BarechAleinu;
    if (civilDate.month > 4 && civilDate.month < 12) return RainRequest::Barechenu;
    if (civilDate.month == 4) return civilDate.day < 15 ? RainRequest::BarechAleinu : RainRequest::Barechenu;
    const int startDay = CivilCalendar::isLeapYear(civilDate.year + 1) ? 5 : 4;
    if (civilDate.day > startDay || (civilDate.day == startDay && service == PrayerService::Arvit)) {
      return RainRequest::BarechAleinu;
    }
    return RainRequest::Barechenu;
  }
};

}  // namespace SiddurEngine
