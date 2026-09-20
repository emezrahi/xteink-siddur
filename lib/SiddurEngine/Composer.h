#pragma once

#include <vector>

#include "PrayerBlockId.h"
#include "PrayerContext.h"

namespace SiddurEngine {

class Composer final {
 public:
  [[nodiscard]] static std::vector<PrayerBlockId> compose(const PrayerContext& context) {
    if (context.service != PrayerService::Shaharit) {
      return {};
    }
    return composeOrdinaryWeekdayShaharit(context);
  }

  [[nodiscard]] static std::vector<PrayerBlockId> composeOrdinaryWeekdayShaharit(const PrayerContext& context) {
    if (context.service != PrayerService::Shaharit || context.weekday == Weekday::Shabbat) {
      return {};
    }

    std::vector<PrayerBlockId> blocks;
    blocks.reserve(24);

    // ShacharitShacharGenerator.
    blocks.push_back(PrayerBlockId::ModehAni);
    blocks.push_back(PrayerBlockId::MorningBlessings);
    blocks.push_back(PrayerBlockId::TorahBlessings);
    blocks.push_back(PrayerBlockId::PetichatEliyahu);
    blocks.push_back(PrayerBlockId::Talit);
    blocks.push_back(PrayerBlockId::Tefillin);
    blocks.push_back(PrayerBlockId::HannasPrayer);

    // ShacharitOpeningGenerator.
    blocks.push_back(PrayerBlockId::MorningPrayer);
    blocks.push_back(PrayerBlockId::IncenseOfferingMorning);

    // ShacharitZimraGenerator and ShacharitShmaGenerator.
    blocks.push_back(PrayerBlockId::Hodu);
    blocks.push_back(PrayerBlockId::PesukeiDeZimra);
    blocks.push_back(PrayerBlockId::ShemaAndBlessings);

    // Weekday Amidah. Conditional insertions are added in later rule slices.
    blocks.push_back(PrayerBlockId::WeekdayAmidah);

    const bool mondayOrThursday = context.weekday == Weekday::Monday || context.weekday == Weekday::Thursday;

    // ShacharitTahanunGenerator.
    blocks.push_back(mondayOrThursday ? PrayerBlockId::ViduiMondayThursday : PrayerBlockId::ViduiRegular);

    // TorahReadingGenerator is part of ordinary Shaharit on Monday and Thursday.
    if (mondayOrThursday) {
      blocks.push_back(PrayerBlockId::TorahReadingWeekday);
    }

    // ShacharitSofGenerator.
    blocks.push_back(PrayerBlockId::AshreiAfterTahanun);
    blocks.push_back(mondayOrThursday ? PrayerBlockId::UvaLezionTorah : PrayerBlockId::UvaLezionRegular);
    blocks.push_back(PrayerBlockId::BeitYaakov);
    blocks.push_back(songOfDayBlock(context.weekday));
    blocks.push_back(PrayerBlockId::Kaveh);
    blocks.push_back(PrayerBlockId::Aleinu);

    return blocks;
  }

  [[nodiscard]] static std::vector<PrayerBlockId> composeWeekdayAmidah(const PrayerContext& context) {
    std::vector<PrayerBlockId> blocks = {
        PrayerBlockId::RetzehOpening,
    };

    if (context.isRoshHodesh) {
      blocks.push_back(PrayerBlockId::YaalehVeyavoRoshHodesh);
    }

    blocks.push_back(PrayerBlockId::RetzehConclusion);
    blocks.push_back(PrayerBlockId::Modim);
    return blocks;
  }

 private:
  [[nodiscard]] static constexpr PrayerBlockId songOfDayBlock(const Weekday weekday) {
    switch (weekday) {
      case Weekday::Sunday:
        return PrayerBlockId::SongOfDaySunday;
      case Weekday::Monday:
        return PrayerBlockId::SongOfDayMonday;
      case Weekday::Tuesday:
        return PrayerBlockId::SongOfDayTuesday;
      case Weekday::Wednesday:
        return PrayerBlockId::SongOfDayWednesday;
      case Weekday::Thursday:
        return PrayerBlockId::SongOfDayThursday;
      case Weekday::Friday:
        return PrayerBlockId::SongOfDayFriday;
      case Weekday::Shabbat:
        return PrayerBlockId::SongOfDaySaturday;
    }
    return PrayerBlockId::SongOfDaySunday;
  }
};

}  // namespace SiddurEngine
