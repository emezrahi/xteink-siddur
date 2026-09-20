#include <PrayerGeneratorId.h>
#include <ServiceGeneratorPlan.h>
#include <gtest/gtest.h>

TEST(ServiceGeneratorPlan, ShaharitMatchesReferenceGeneratorOrder) {
  const auto plan = SiddurEngine::ServiceGeneratorPlan::forService(SiddurEngine::PrayerService::Shaharit);

  ASSERT_EQ(plan.size, 10U);
  EXPECT_EQ(plan[0], SiddurEngine::PrayerGeneratorId::ShaharitShachar);
  EXPECT_EQ(plan[1], SiddurEngine::PrayerGeneratorId::ShaharitOpening);
  EXPECT_EQ(plan[2], SiddurEngine::PrayerGeneratorId::ShaharitZimra);
  EXPECT_EQ(plan[3], SiddurEngine::PrayerGeneratorId::ShaharitShma);
  EXPECT_EQ(plan[4], SiddurEngine::PrayerGeneratorId::Amidah);
  EXPECT_EQ(plan[5], SiddurEngine::PrayerGeneratorId::Hallel);
  EXPECT_EQ(plan[6], SiddurEngine::PrayerGeneratorId::ShaharitTahanun);
  EXPECT_EQ(plan[7], SiddurEngine::PrayerGeneratorId::TorahReading);
  EXPECT_EQ(plan[8], SiddurEngine::PrayerGeneratorId::Mussaf);
  EXPECT_EQ(plan[9], SiddurEngine::PrayerGeneratorId::ShaharitSof);
}

TEST(ServiceGeneratorPlan, OtherPrimaryServicesHaveReferenceGeneratorRoots) {
  const auto minha = SiddurEngine::ServiceGeneratorPlan::forService(SiddurEngine::PrayerService::Minha);
  ASSERT_EQ(minha.size, 1U);
  EXPECT_EQ(minha[0], SiddurEngine::PrayerGeneratorId::Minha);

  const auto arvit = SiddurEngine::ServiceGeneratorPlan::forService(SiddurEngine::PrayerService::Arvit);
  ASSERT_EQ(arvit.size, 1U);
  EXPECT_EQ(arvit[0], SiddurEngine::PrayerGeneratorId::Arvit);

  const auto musaf = SiddurEngine::ServiceGeneratorPlan::forService(SiddurEngine::PrayerService::Musaf);
  ASSERT_EQ(musaf.size, 1U);
  EXPECT_EQ(musaf[0], SiddurEngine::PrayerGeneratorId::Mussaf);
}
