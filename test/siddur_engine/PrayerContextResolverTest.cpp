#include <PrayerContextResolver.h>
#include <gtest/gtest.h>

TEST(PrayerContextResolver, ResolvesDaytimeCivilDate) {
  const auto resolved =
      SiddurEngine::PrayerContextResolver::resolve(SiddurEngine::PrayerService::Shaharit, {2024, 11, 30}, false);

  EXPECT_EQ(resolved.hebrewDate.year, 5785);
  EXPECT_EQ(resolved.hebrewDate.month, SiddurEngine::HebrewMonth::Cheshvan);
  EXPECT_EQ(resolved.hebrewDate.day, 29);
  EXPECT_FALSE(resolved.context.isRoshHodesh);
}

TEST(PrayerContextResolver, AdvancesJewishDateAfterSunset) {
  const auto resolved =
      SiddurEngine::PrayerContextResolver::resolve(SiddurEngine::PrayerService::Shaharit, {2024, 11, 30}, true);

  EXPECT_EQ(resolved.hebrewDate.year, 5785);
  EXPECT_EQ(resolved.hebrewDate.month, SiddurEngine::HebrewMonth::Cheshvan);
  EXPECT_EQ(resolved.hebrewDate.day, 30);
  EXPECT_TRUE(resolved.context.isRoshHodesh);
}

TEST(PrayerContextResolver, HandlesGregorianLeapDayWhenAdvancingAfterSunset) {
  const auto resolved =
      SiddurEngine::PrayerContextResolver::resolve(SiddurEngine::PrayerService::Shaharit, {2024, 2, 28}, true);

  EXPECT_EQ(resolved.hebrewDate.year, 5784);
  EXPECT_EQ(resolved.hebrewDate.month, SiddurEngine::HebrewMonth::Adar);
  EXPECT_EQ(resolved.hebrewDate.day, 20);
}

TEST(PrayerContextResolver, PreservesRequestedPrayerService) {
  const auto resolved =
      SiddurEngine::PrayerContextResolver::resolve(SiddurEngine::PrayerService::Minha, {2024, 12, 2}, false);

  EXPECT_EQ(resolved.context.service, SiddurEngine::PrayerService::Minha);
  EXPECT_TRUE(resolved.context.isRoshHodesh);
}
