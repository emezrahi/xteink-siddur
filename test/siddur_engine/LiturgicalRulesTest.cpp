#include <PrayerContextResolver.h>
#include <gtest/gtest.h>

namespace {

SiddurEngine::PrayerContext resolve(const SiddurEngine::PrayerService service, const SiddurEngine::CivilDate date) {
  return SiddurEngine::PrayerContextResolver::resolve(service, date, false, true).context;
}

}  // namespace

TEST(LiturgicalRules, HanukkahUsesAlHanissimFullHallelAndNoTachanun) {
  const auto context = resolve(SiddurEngine::PrayerService::Shaharit, {2026, 12, 5});
  EXPECT_TRUE(context.isHanukkah);
  EXPECT_TRUE(context.sayAlHanissim);
  EXPECT_EQ(context.hallel, SiddurEngine::HallelKind::Full);
  EXPECT_FALSE(context.sayTachanun);
}

TEST(LiturgicalRules, PesachUsesYaalehVeyavoHallelAndMussaf) {
  const auto context = resolve(SiddurEngine::PrayerService::Shaharit, {2027, 4, 22});
  EXPECT_TRUE(context.isYomTov);
  EXPECT_TRUE(context.sayYaalehVeyavo);
  EXPECT_TRUE(context.sayMussaf);
  EXPECT_EQ(context.hallel, SiddurEngine::HallelKind::Full);
}

TEST(LiturgicalRules, CountsEveryOmerDayBoundary) {
  EXPECT_EQ(resolve(SiddurEngine::PrayerService::Arvit, {2027, 4, 23}).omerDay, 1);
  EXPECT_EQ(resolve(SiddurEngine::PrayerService::Arvit, {2027, 5, 23}).omerDay, 31);
}

TEST(LiturgicalRules, SeasonalAmidahChangesAtFestivalServices) {
  auto context = resolve(SiddurEngine::PrayerService::Shaharit, {2026, 10, 3});  // 22 Tishrei
  EXPECT_EQ(context.rainMention, SiddurEngine::RainMention::MoridHatal);
  context = resolve(SiddurEngine::PrayerService::Musaf, {2026, 10, 3});
  EXPECT_EQ(context.rainMention, SiddurEngine::RainMention::MashivHaruach);

  context = resolve(SiddurEngine::PrayerService::Shaharit, {2027, 4, 22});  // 15 Nisan
  EXPECT_EQ(context.rainMention, SiddurEngine::RainMention::MashivHaruach);
  context = resolve(SiddurEngine::PrayerService::Musaf, {2027, 4, 22});
  EXPECT_EQ(context.rainMention, SiddurEngine::RainMention::MoridHatal);
}

TEST(LiturgicalRules, DiasporaRainRequestStartsAtDecemberArvit) {
  auto context = resolve(SiddurEngine::PrayerService::Minha, {2026, 12, 4});
  EXPECT_EQ(context.rainRequest, SiddurEngine::RainRequest::Barechenu);
  context = resolve(SiddurEngine::PrayerService::Arvit, {2026, 12, 4});
  EXPECT_EQ(context.rainRequest, SiddurEngine::RainRequest::BarechAleinu);
}
