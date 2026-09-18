#include <Composer.h>
#include <gtest/gtest.h>

TEST(SiddurComposer, WeekdayShaharitOmitsYaalehVeyavo) {
  SiddurEngine::PrayerContext context;
  context.service = SiddurEngine::PrayerService::Shaharit;

  const auto blocks = SiddurEngine::Composer::compose(context);

  ASSERT_EQ(blocks.size(), 6U);
  EXPECT_EQ(blocks[0], SiddurEngine::PrayerBlockId::NetilatYadayim);
  EXPECT_EQ(blocks[1], SiddurEngine::PrayerBlockId::AsherYatzar);
  EXPECT_EQ(blocks[2], SiddurEngine::PrayerBlockId::ElohaiNeshama);
  EXPECT_EQ(blocks[3], SiddurEngine::PrayerBlockId::RetzehOpening);
  EXPECT_EQ(blocks[4], SiddurEngine::PrayerBlockId::RetzehConclusion);
  EXPECT_EQ(blocks[5], SiddurEngine::PrayerBlockId::Modim);
}

TEST(SiddurComposer, RoshHodeshShaharitInsertsYaalehVeyavoInsideRetzeh) {
  SiddurEngine::PrayerContext context;
  context.service = SiddurEngine::PrayerService::Shaharit;
  context.isRoshHodesh = true;

  const auto blocks = SiddurEngine::Composer::compose(context);

  ASSERT_EQ(blocks.size(), 7U);
  EXPECT_EQ(blocks[3], SiddurEngine::PrayerBlockId::RetzehOpening);
  EXPECT_EQ(blocks[4], SiddurEngine::PrayerBlockId::YaalehVeyavoRoshHodesh);
  EXPECT_EQ(blocks[5], SiddurEngine::PrayerBlockId::RetzehConclusion);
  EXPECT_EQ(blocks[6], SiddurEngine::PrayerBlockId::Modim);
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

TEST(SiddurComposer, UnimplementedContextFlagsDoNotChangeWeekdayComposition) {
  SiddurEngine::PrayerContext context;
  context.service = SiddurEngine::PrayerService::Shaharit;
  context.isYomTov = true;
  context.isHolHamoed = true;
  context.isFastDay = true;
  context.isHanukkah = true;
  context.isPurim = true;

  const auto blocks = SiddurEngine::Composer::compose(context);

  ASSERT_EQ(blocks.size(), 6U);
  EXPECT_EQ(blocks[3], SiddurEngine::PrayerBlockId::RetzehOpening);
  EXPECT_EQ(blocks[4], SiddurEngine::PrayerBlockId::RetzehConclusion);
  EXPECT_EQ(blocks[5], SiddurEngine::PrayerBlockId::Modim);
}
