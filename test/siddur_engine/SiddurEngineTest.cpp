#include <Composer.h>
#include <gtest/gtest.h>

namespace {

bool contains(const std::vector<SiddurEngine::PrayerBlockId>& blocks, const SiddurEngine::PrayerBlockId id) {
  for (const auto block : blocks) {
    if (block == id) return true;
  }
  return false;
}

}  // namespace

TEST(SiddurComposer, SundayShaharitUsesRegularTahanunAndNoTorahReading) {
  SiddurEngine::PrayerContext context;
  context.service = SiddurEngine::PrayerService::Shaharit;
  context.weekday = SiddurEngine::Weekday::Sunday;

  const auto blocks = SiddurEngine::Composer::compose(context);

  ASSERT_EQ(blocks.size(), 22U);
  EXPECT_EQ(blocks.front(), SiddurEngine::PrayerBlockId::ModehAni);
  EXPECT_TRUE(contains(blocks, SiddurEngine::PrayerBlockId::ViduiRegular));
  EXPECT_FALSE(contains(blocks, SiddurEngine::PrayerBlockId::ViduiMondayThursday));
  EXPECT_FALSE(contains(blocks, SiddurEngine::PrayerBlockId::TorahReadingWeekday));
  EXPECT_TRUE(contains(blocks, SiddurEngine::PrayerBlockId::UvaLezionRegular));
  EXPECT_EQ(blocks[19], SiddurEngine::PrayerBlockId::SongOfDaySunday);
  EXPECT_EQ(blocks.back(), SiddurEngine::PrayerBlockId::Aleinu);
}

TEST(SiddurComposer, MondayShaharitAddsExtendedTahanunAndTorahReading) {
  SiddurEngine::PrayerContext context;
  context.service = SiddurEngine::PrayerService::Shaharit;
  context.weekday = SiddurEngine::Weekday::Monday;

  const auto blocks = SiddurEngine::Composer::compose(context);

  ASSERT_EQ(blocks.size(), 23U);
  EXPECT_TRUE(contains(blocks, SiddurEngine::PrayerBlockId::ViduiMondayThursday));
  EXPECT_TRUE(contains(blocks, SiddurEngine::PrayerBlockId::TorahReadingWeekday));
  EXPECT_TRUE(contains(blocks, SiddurEngine::PrayerBlockId::UvaLezionTorah));
  EXPECT_FALSE(contains(blocks, SiddurEngine::PrayerBlockId::UvaLezionRegular));
  EXPECT_EQ(blocks[20], SiddurEngine::PrayerBlockId::SongOfDayMonday);
}

TEST(SiddurComposer, ThursdayUsesThursdaySongOfDay) {
  SiddurEngine::PrayerContext context;
  context.service = SiddurEngine::PrayerService::Shaharit;
  context.weekday = SiddurEngine::Weekday::Thursday;

  const auto blocks = SiddurEngine::Composer::compose(context);

  ASSERT_EQ(blocks.size(), 23U);
  EXPECT_EQ(blocks[20], SiddurEngine::PrayerBlockId::SongOfDayThursday);
}

TEST(SiddurComposer, FridayUsesFridaySongOfDay) {
  SiddurEngine::PrayerContext context;
  context.service = SiddurEngine::PrayerService::Shaharit;
  context.weekday = SiddurEngine::Weekday::Friday;

  const auto blocks = SiddurEngine::Composer::compose(context);

  ASSERT_EQ(blocks.size(), 22U);
  EXPECT_EQ(blocks[19], SiddurEngine::PrayerBlockId::SongOfDayFriday);
}

TEST(SiddurComposer, ShabbatIsNotPresentedAsWeekdayShaharit) {
  SiddurEngine::PrayerContext context;
  context.service = SiddurEngine::PrayerService::Shaharit;
  context.weekday = SiddurEngine::Weekday::Shabbat;

  EXPECT_TRUE(SiddurEngine::Composer::compose(context).empty());
}

TEST(SiddurComposer, WeekdayAmidahOmitsYaalehVeyavo) {
  SiddurEngine::PrayerContext context;

  const auto blocks = SiddurEngine::Composer::composeWeekdayAmidah(context);

  ASSERT_EQ(blocks.size(), 3U);
  EXPECT_EQ(blocks[0], SiddurEngine::PrayerBlockId::RetzehOpening);
  EXPECT_EQ(blocks[1], SiddurEngine::PrayerBlockId::RetzehConclusion);
  EXPECT_EQ(blocks[2], SiddurEngine::PrayerBlockId::Modim);
}

TEST(SiddurComposer, RoshHodeshAmidahInsertsYaalehVeyavoInsideRetzeh) {
  SiddurEngine::PrayerContext context;
  context.isRoshHodesh = true;

  const auto blocks = SiddurEngine::Composer::composeWeekdayAmidah(context);

  ASSERT_EQ(blocks.size(), 4U);
  EXPECT_EQ(blocks[0], SiddurEngine::PrayerBlockId::RetzehOpening);
  EXPECT_EQ(blocks[1], SiddurEngine::PrayerBlockId::YaalehVeyavoRoshHodesh);
  EXPECT_EQ(blocks[2], SiddurEngine::PrayerBlockId::RetzehConclusion);
  EXPECT_EQ(blocks[3], SiddurEngine::PrayerBlockId::Modim);
}

TEST(SiddurComposer, MinhaArvitAndMussafComposeConditionally) {
  SiddurEngine::PrayerContext context;

  context.service = SiddurEngine::PrayerService::Minha;
  EXPECT_TRUE(contains(SiddurEngine::Composer::compose(context), SiddurEngine::PrayerBlockId::MinhaOpening));

  context.service = SiddurEngine::PrayerService::Arvit;
  EXPECT_TRUE(contains(SiddurEngine::Composer::compose(context), SiddurEngine::PrayerBlockId::ArvitShemaAndBlessings));

  context.service = SiddurEngine::PrayerService::Musaf;
  EXPECT_TRUE(SiddurEngine::Composer::compose(context).empty());

  context.sayMussaf = true;
  context.isRoshHodesh = true;
  EXPECT_EQ(SiddurEngine::Composer::compose(context).front(), SiddurEngine::PrayerBlockId::MussafRoshHodesh);
}
