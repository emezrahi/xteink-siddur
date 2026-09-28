#pragma once

#include <Chapter.h>

#include <cstring>
#include <string_view>
#include <vector>

#include "EdotWeekdayShaharit.h"

namespace SiddurContent {

namespace detail {
struct Segment {
  const char* englishTitle;
  const char* hebrewTitle;
  const char* startMarker; // paragraph prefix, or nullptr for first chapter
};

inline void appendSegments(std::vector<Chapter>& chapters, const EdotWeekdayShaharit::PrayerTextBlock& block,
                           const Segment* segments, const std::size_t count) {
  const std::string_view source(block.text, std::strlen(block.text));
  std::vector<std::size_t> starts;
  starts.reserve(count);
  starts.push_back(0);
  for (std::size_t i = 1; i < count; ++i) {
    const auto pos = source.find(segments[i].startMarker, starts.back());
    if (pos == std::string_view::npos || pos <= starts.back()) {
      // Never mislabel a whole prayer as several different chapters if the
      // underlying corpus is revised and a paragraph anchor disappears.
      chapters.push_back({block.id, "Prayer section", block.title, source});
      return;
    }
    starts.push_back(pos);
  }
  for (std::size_t i = 0; i < count; ++i) {
    const auto end = i + 1 < count ? starts[i + 1] : source.size();
    chapters.push_back({block.id, segments[i].englishTitle, segments[i].hebrewTitle,
                        source.substr(starts[i], end - starts[i])});
  }
}
} // namespace detail

// The English titles and navigation order follow the observed Smart Siddur
// chapter menu where a corresponding CC0 Edot HaMizrach chapter is available.
// Text is ALWAYS from Sefaria's CC0 Shaliehsaboo edition, never APK assets.
// The APK screenshot covers only part of Shaharit; other entries use descriptive
// English names until a complete permitted chapter-reference audit is available.
inline std::vector<Chapter> weekdayChapters(const std::vector<SiddurEngine::PrayerBlockId>& prayer) {
  using SiddurEngine::PrayerBlockId;
  std::vector<Chapter> result;
  result.reserve(prayer.size() + 16);
  for (const auto id : prayer) {
    const auto* block = EdotWeekdayShaharit::findBlock(id);
    if (block == nullptr || block->text == nullptr || block->text[0] == '\0') continue;

    switch (id) {
      case PrayerBlockId::MorningPrayer: {
        constexpr detail::Segment segments[] = {
            {"Leshem Ihud", "לשם יחוד", nullptr},
            {"Akeda", "עקדת יצחק", "\n\nוַיְהִ֗י"},
            {"Leolam", "לעולם יהא אדם", "\n\nלְעוֹלָם"},
            {"Korban HaTamid", "קרבן התמיד", "\n\nוַיְדַבֵּ֥ר"},
        };
        detail::appendSegments(result, *block, segments, 4);
        break;
      }
      case PrayerBlockId::IncenseOfferingMorning: {
        constexpr detail::Segment segments[] = {
            {"Parashat HaKtoret", "פיטום הקטורת", nullptr},
            {"The Mishna of Sacrifices", "איזהו מקומן", "\n\nאֵיזֶהוּ"},
            {"The Thirteen Principles of Interpretation", "רבי ישמעאל", "\n\nרִבִּי יִשְׁמָעֵאל"},
        };
        detail::appendSegments(result, *block, segments, 3);
        break;
      }
      case PrayerBlockId::PesukeiDeZimra: {
        constexpr detail::Segment segments[] = {
            {"Baruch SheAmar", "ברוך שאמר", nullptr},
            {"Ashrei", "אשרי", "\n\nאַ֭שְׁרֵי"},
            {"The Lord Will Be Praised Forever", "ישתבח", "\n\nיִשְׁתַּבַּח"},
        };
        detail::appendSegments(result, *block, segments, 3);
        break;
      }
      case PrayerBlockId::ShemaAndBlessings: {
        constexpr detail::Segment segments[] = {
            {"Yotser Or", "יוצר אור", nullptr},
            {"Shma", "שמע ישראל", "\n\nקודם שיקרא"},
        };
        detail::appendSegments(result, *block, segments, 2);
        break;
      }
      default: {
        const char* title = nullptr;
        switch (id) {
          case PrayerBlockId::ModehAni: title = "Modeh Ani"; break;
          case PrayerBlockId::MorningBlessings: title = "Morning Blessings"; break;
          case PrayerBlockId::TorahBlessings: title = "Blessings of the Torah"; break;
          case PrayerBlockId::PetichatEliyahu: title = "Petichat Eliyahu"; break;
          case PrayerBlockId::Talit: title = "Putting on Talit"; break;
          case PrayerBlockId::Tefillin: title = "Putting on Tefillin"; break;
          case PrayerBlockId::HannasPrayer: title = "Hannah's Prayer"; break;
          case PrayerBlockId::Hodu: title = "Hodu"; break;
          case PrayerBlockId::WeekdayAmidah: title = "Amida"; break;
          case PrayerBlockId::ViduiRegular: title = "Tahanun"; break;
          case PrayerBlockId::ViduiMondayThursday: title = "Tahanun (Mon / Thu)"; break;
          case PrayerBlockId::TorahReadingWeekday: title = "Torah Reading"; break;
          case PrayerBlockId::AshreiAfterTahanun: title = "Ashrei (after Tahanun)"; break;
          case PrayerBlockId::UvaLezionRegular:
          case PrayerBlockId::UvaLezionTorah: title = "Uva LeTzion"; break;
          case PrayerBlockId::BeitYaakov: title = "Beit Yaakov"; break;
          case PrayerBlockId::SongOfDaySunday:
          case PrayerBlockId::SongOfDayMonday:
          case PrayerBlockId::SongOfDayTuesday:
          case PrayerBlockId::SongOfDayWednesday:
          case PrayerBlockId::SongOfDayThursday:
          case PrayerBlockId::SongOfDayFriday: title = "Song of the Day"; break;
          case PrayerBlockId::Kaveh: title = "Kaveh"; break;
          case PrayerBlockId::Aleinu: title = "Aleinu"; break;
          default: break;
        }
        if (title != nullptr) {
          result.push_back({id, title, block->title, std::string_view(block->text, std::strlen(block->text))});
        }
        break;
      }
    }
  }
  return result;
}

} // namespace SiddurContent
