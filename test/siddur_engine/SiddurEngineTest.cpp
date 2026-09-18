#include <SiddurEngine/Composer.h>
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

TEST(SiddurComposer, UnsupportedServicesComposeToEmptyUntilImplemented) {
  SiddurEngine::PrayerContext context;

  context.service = SiddurEngine::PrayerService::Minha;
  EXPECT_TRUE(SiddurEngine::Composer::compose(context).empty());

  context.service = SiddurEngine::PrayerService::Arvit;
  EXPECT_TRUE(SiddurEngine::Composer::compose(context).empty());

  context.service = SiddurEngine::PrayerService::Musaf;
  EXPECT_TRUE(SiddurEngine::Composer::compose(context).empty());
}

TEST(SiddurComposer, ContextFlagsDoNotChangeCompositionBeforeRulesAreImplemented) {
  SiddurEngine::PrayerContext context;
  context.service = SiddurEngine::PrayerService::Shaharit;
  context.isRoshHodesh = true;
  context.isYomTov = true;
  context.isHolHamoed = true;
  context.isFastDay = true;
  context.isHanukkah = true;
  context.isPurim = true;

  const auto blocks = SiddurEngine::Composer::compose(context);

  ASSERT_EQ(blocks.size(), 3U);
  EXPECT_EQ(blocks[0], SiddurEngine::PrayerBlockId::NetilatYadayim);
  EXPECT_EQ(blocks[1], SiddurEngine::PrayerBlockId::AsherYatzar);
  EXPECT_EQ(blocks[2], SiddurEngine::PrayerBlockId::ElohaiNeshama);
}
