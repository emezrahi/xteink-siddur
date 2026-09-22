#include <PrayerContextResolver.h>
#include <ServiceSelector.h>
#include <Zmanim.h>
#include <gtest/gtest.h>

TEST(Zmanim, CalculatesBrooklynSunriseNoonAndSunset) {
  SiddurEngine::LocationConfig location;
  location.utcOffsetMinutes = -240;

  const auto zmanim = SiddurEngine::Zmanim::calculate({2026, 9, 20}, location);

  ASSERT_TRUE(zmanim.valid);
  EXPECT_NEAR(zmanim.sunriseMinutes, 402, 3);
  EXPECT_NEAR(zmanim.solarNoonMinutes, 771, 3);
  EXPECT_NEAR(zmanim.sunsetMinutes, 1140, 3);
}

TEST(Zmanim, RejectsInvalidLocation) {
  SiddurEngine::LocationConfig location;
  location.latitude = 91.0;
  EXPECT_FALSE(SiddurEngine::Zmanim::calculate({2026, 9, 20}, location).valid);
}

TEST(ServiceSelector, ChoosesServiceFromSolarBoundaries) {
  const SiddurEngine::DailyZmanim zmanim{400, 770, 1140, true};
  EXPECT_EQ(SiddurEngine::ServiceSelector::select({{2026, 9, 20}, 6, 0}, zmanim), SiddurEngine::PrayerService::Arvit);
  EXPECT_EQ(SiddurEngine::ServiceSelector::select({{2026, 9, 20}, 9, 0}, zmanim),
            SiddurEngine::PrayerService::Shaharit);
  EXPECT_EQ(SiddurEngine::ServiceSelector::select({{2026, 9, 20}, 14, 0}, zmanim), SiddurEngine::PrayerService::Minha);
  EXPECT_EQ(SiddurEngine::ServiceSelector::select({{2026, 9, 20}, 20, 0}, zmanim), SiddurEngine::PrayerService::Arvit);
}

TEST(PrayerContextResolver, RollsHebrewDateAndSelectsArvitAfterSunset) {
  SiddurEngine::LocationConfig location;
  location.utcOffsetMinutes = -240;
  const auto resolved = SiddurEngine::PrayerContextResolver::resolve({{2026, 9, 20}, 20, 0}, location);

  EXPECT_EQ(resolved.context.service, SiddurEngine::PrayerService::Arvit);
  EXPECT_EQ(resolved.hebrewDate.month, SiddurEngine::HebrewMonth::Tishrei);
  EXPECT_EQ(resolved.hebrewDate.day, 10);
  EXPECT_TRUE(resolved.context.isYomTov);
}
