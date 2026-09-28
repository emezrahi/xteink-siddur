#include "src/activities/siddur/content/EdotChapters.h"

#include <gtest/gtest.h>

#include <cstring>
#include <string>
#include <vector>

using SiddurEngine::PrayerBlockId;

TEST(EdotChapters, ApkVisibleEnglishNamesAndOrderingHaveRealContent) {
  const auto chapters = SiddurContent::weekdayChapters({
      PrayerBlockId::Talit,
      PrayerBlockId::MorningPrayer,
      PrayerBlockId::IncenseOfferingMorning,
      PrayerBlockId::Hodu,
      PrayerBlockId::PesukeiDeZimra,
      PrayerBlockId::ShemaAndBlessings,
      PrayerBlockId::WeekdayAmidah,
  });
  const std::vector<std::string> expected = {
      "Putting on Talit", "Leshem Ihud", "Akeda", "Leolam", "Korban HaTamid", "Parashat HaKtoret",
      "The Mishna of Sacrifices", "The Thirteen Principles of Interpretation", "Hodu", "Baruch SheAmar",
      "Ashrei", "The Lord Will Be Praised Forever", "Yotser Or", "Shma", "Amida"};
  ASSERT_EQ(chapters.size(), expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i) {
    EXPECT_STREQ(chapters[i].englishTitle, expected[i].c_str());
    EXPECT_FALSE(chapters[i].text.empty());
    EXPECT_FALSE(chapters[i].hebrewTitle[0] == '\0');
  }
}

TEST(EdotChapters, SplitChaptersReconstructEntireOpenSourceBlocksWithoutLoss) {
  const std::vector<PrayerBlockId> splitBlocks = {PrayerBlockId::MorningPrayer,
                                                  PrayerBlockId::IncenseOfferingMorning,
                                                  PrayerBlockId::PesukeiDeZimra,
                                                  PrayerBlockId::ShemaAndBlessings};
  const auto chapters = SiddurContent::weekdayChapters(splitBlocks);
  for (const auto id : splitBlocks) {
    const auto* source = SiddurContent::EdotWeekdayShaharit::findBlock(id);
    ASSERT_NE(source, nullptr);
    std::string combined;
    for (const auto& chapter : chapters) {
      if (chapter.sourceBlock == id) combined.append(chapter.text.data(), chapter.text.size());
    }
    EXPECT_EQ(combined, std::string(source->text, std::strlen(source->text)));
  }
}

TEST(EdotChapters, MissingSourceDoesNotCreateSelectableEmptyChapter) {
  const auto chapters = SiddurContent::weekdayChapters({PrayerBlockId::MussafFestival});
  EXPECT_TRUE(chapters.empty());
}
