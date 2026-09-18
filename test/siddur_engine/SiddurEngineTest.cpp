#include <Composer.h>
#include <gtest/gtest.h>

TEST(SiddurComposer, WeekdayShaharitComposesHardwareTestedMorningBlessingsInOrder) {
  SiddurEngine::PrayerContext context;
  context.service = SiddurEngine::PrayerService::Shaharit;

  const auto blocks = SiddurEngine::Composer::compose(context);

  ASSERT_EQ(blocks.size(), 3U);
  EXPECT_EQ(blocks[0], SiddurEngine::PrayerBlockId::NetilatYadayim);
  EXPECT_EQ(blocks[1], SiddurEngine::PrayerBlockId::AsherYatzar);
  EXPECT_EQ(blocks[2], SiddurEngine::PrayerBlockId::ElohaiNeshama);
}

TEST(SiddurComposer, RoshHodeshDoesNotCorruptIncompleteVisibleShaharitFlow) {
  SiddurEngine::PrayerContext context;
  context.service = SiddurEngine::PrayerService::Shaharit;
  context.isRoshHodesh = true;

  const auto blocks = SiddurEngine::Composer::compose(context);

  ASSERT_EQ(blocks.size(), 3U);
  EXPECT_EQ(blocks[0], SiddurEngine::PrayerBlockId::NetilatYadayim);
  EXPECT_EQ(blocks[1], SiddurEngine::PrayerBlockId::AsherYatzar);
  EXPECT_EQ(blocks[2], SiddurEngine::PrayerBlockId::ElohaiNeshama);
}

TEST(SiddurComposer, WeekdayAmidahKeepsRetzehAndModimAdjacentWithoutRoshHodesh) {
  SiddurEngine::PrayerContext context;
  context.service = SiddurEngine::PrayerService::Shaharit;

  const auto blocks = SiddurEngine::Composer::composeWeekdayAmidah(context);

  ASSERT_EQ(blocks.size(), 3U);
  EXPECT_EQ(blocks[0], SiddurEngine::PrayerBlockId::RetzehOpening);
  EXPECT_EQ(blocks[1], SiddurEngine::PrayerBlockId::RetzehConclusion);
  EXPECT_EQ(blocks[2], SiddurEngine::PrayerBlockId::Modim);
}

TEST(SiddurComposer, RoshHodeshInsertsYaalehVeyavoInsideRetzehBeforeConclusion) {
  SiddurEngine::PrayerContext context;
  context.service = SiddurEngine::PrayerService::Shaharit;
  context.isRoshHodesh = true;

  const auto blocks = SiddurEngine::Composer::composeWeekdayAmidah(context);

  ASSERT_EQ(blocks.size(), 4U);
  EXPECT_EQ(blocks[0], SiddurEngine::PrayerBlockId::RetzehOpening);
  EXPECT_EQ(blocks[1], SiddurEngine::PrayerBlockId::YaalehVeyavoRoshHodesh);
  EXPECT_EQ(blocks[2], SiddurEngine::PrayerBlockId::RetzehConclusion);
  EXPECT_EQ(blocks[3], SiddurEngine::PrayerBlockId::Modim);
}

TEST(SiddurComposer, UnsupportedServicesComposeToEmptyUntilImplemented) {
  SiddurEngine::PrayerContext context;

  context.service = SiddurEngine::PrayerService::Minha;
  EXPECT_TRUE(SiddurEngine::Composer::compose(context).empty());

  context.service = SiddurEngine::PrayerService::Arvit;
  EXPECT_TRUE(SiddurEngine::Composer::compose(context).empty());

  context.service = SiddurEngine::PrayerService::Musaf;
  EXPECT_TRUE(SiddurEngine::Composer::compose(context).empty());
}
