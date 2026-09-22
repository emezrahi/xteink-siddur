#pragma once

#include <vector>

#include "PrayerBlockId.h"
#include "PrayerContext.h"

namespace SiddurEngine {

class Composer final {
 public:
  [[nodiscard]] static std::vector<PrayerBlockId> compose(const PrayerContext& context) {
    switch (context.service) {
      case PrayerService::Shaharit:
        return composeWeekdayShaharit(context);
      case PrayerService::Minha:
        return composeMinha(context);
      case PrayerService::Arvit:
        return composeArvit(context);
      case PrayerService::Musaf:
        return composeMussaf(context);
    }
    return {};
  }

  [[nodiscard]] static std::vector<PrayerBlockId> composeOrdinaryWeekdayShaharit(const PrayerContext& context) {
    return composeWeekdayShaharit(context);
  }

  [[nodiscard]] static std::vector<PrayerBlockId> composeWeekdayShaharit(const PrayerContext& context) {
    if (context.service != PrayerService::Shaharit || context.weekday == Weekday::Shabbat) {
      return {};
    }

    std::vector<PrayerBlockId> blocks;
    blocks.reserve(32);

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

    blocks.push_back(PrayerBlockId::WeekdayAmidah);
    appendAmidahVariants(blocks, context);

    if (context.hallel == HallelKind::Half) blocks.push_back(PrayerBlockId::HallelHalf);
    if (context.hallel == HallelKind::Full) blocks.push_back(PrayerBlockId::HallelFull);

    const bool mondayOrThursday = context.weekday == Weekday::Monday || context.weekday == Weekday::Thursday;

    // ShacharitTahanunGenerator.
    if (context.sayTachanun) {
      blocks.push_back(mondayOrThursday ? PrayerBlockId::ViduiMondayThursday : PrayerBlockId::ViduiRegular);
    }

    // TorahReadingGenerator is part of ordinary Shaharit on Monday and Thursday.
    if (mondayOrThursday || context.isRoshHodesh || context.isFastDay) {
      blocks.push_back(PrayerBlockId::TorahReadingWeekday);
    }

    // ShacharitSofGenerator.
    blocks.push_back(PrayerBlockId::AshreiAfterTahanun);
    const bool hasTorahReading = mondayOrThursday || context.isRoshHodesh || context.isFastDay;
    blocks.push_back(hasTorahReading ? PrayerBlockId::UvaLezionTorah : PrayerBlockId::UvaLezionRegular);
    blocks.push_back(PrayerBlockId::BeitYaakov);
    blocks.push_back(songOfDayBlock(context.weekday));
    blocks.push_back(PrayerBlockId::Kaveh);
    appendMussaf(blocks, context);
    blocks.push_back(PrayerBlockId::Aleinu);

    return blocks;
  }

  [[nodiscard]] static std::vector<PrayerBlockId> composeWeekdayAmidah(const PrayerContext& context) {
    std::vector<PrayerBlockId> blocks = {
        PrayerBlockId::RetzehOpening,
    };

    if (context.isRoshHodesh) {
      blocks.push_back(PrayerBlockId::YaalehVeyavoRoshHodesh);
    } else if (context.sayYaalehVeyavo) {
      blocks.push_back(PrayerBlockId::YaalehVeyavoFestival);
    }

    blocks.push_back(PrayerBlockId::RetzehConclusion);
    blocks.push_back(PrayerBlockId::Modim);
    return blocks;
  }

 private:
  static void appendAmidahVariants(std::vector<PrayerBlockId>& blocks, const PrayerContext& context) {
    blocks.push_back(context.rainMention == RainMention::MashivHaruach ? PrayerBlockId::MashivHaruach
                                                                       : PrayerBlockId::MoridHatal);
    blocks.push_back(context.rainRequest == RainRequest::BarechAleinu ? PrayerBlockId::BarechAleinu
                                                                      : PrayerBlockId::Barechenu);
    if (context.sayYaalehVeyavo) {
      blocks.push_back(context.isRoshHodesh ? PrayerBlockId::YaalehVeyavoRoshHodesh
                                            : PrayerBlockId::YaalehVeyavoFestival);
    }
    if (context.isHanukkah) blocks.push_back(PrayerBlockId::AlHanissimHanukkah);
    if (context.isPurim) blocks.push_back(PrayerBlockId::AlHanissimPurim);
    if (context.sayAneinu) blocks.push_back(PrayerBlockId::Aneinu);
  }

  static void appendMussaf(std::vector<PrayerBlockId>& blocks, const PrayerContext& context) {
    if (!context.sayMussaf) return;
    blocks.push_back(context.isRoshHodesh && !context.isYomTov ? PrayerBlockId::MussafRoshHodesh
                                                               : PrayerBlockId::MussafFestival);
  }

  [[nodiscard]] static std::vector<PrayerBlockId> composeMinha(const PrayerContext& context) {
    std::vector<PrayerBlockId> blocks;
    blocks.reserve(12);
    blocks.push_back(PrayerBlockId::MinhaOpening);
    blocks.push_back(PrayerBlockId::WeekdayAmidah);
    appendAmidahVariants(blocks, context);
    if (context.sayTachanun) blocks.push_back(PrayerBlockId::ViduiRegular);
    blocks.push_back(PrayerBlockId::Aleinu);
    return blocks;
  }

  [[nodiscard]] static std::vector<PrayerBlockId> composeArvit(const PrayerContext& context) {
    std::vector<PrayerBlockId> blocks;
    blocks.reserve(12);
    blocks.push_back(PrayerBlockId::ArvitShemaAndBlessings);
    blocks.push_back(PrayerBlockId::WeekdayAmidah);
    appendAmidahVariants(blocks, context);
    if (context.isSefiratHaOmer) blocks.push_back(PrayerBlockId::SefiratHaOmer);
    blocks.push_back(PrayerBlockId::Aleinu);
    return blocks;
  }

  [[nodiscard]] static std::vector<PrayerBlockId> composeMussaf(const PrayerContext& context) {
    std::vector<PrayerBlockId> blocks;
    blocks.reserve(1);
    appendMussaf(blocks, context);
    return blocks;
  }

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
