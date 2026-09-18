#include <HebrewCalendar.h>
#include <gtest/gtest.h>

namespace {

void expectHebrewDate(const SiddurEngine::CivilDate civil, const int year, const SiddurEngine::HebrewMonth month,
                      const int day) {
  const auto hebrew = SiddurEngine::HebrewCalendar::fromGregorian(civil);
  EXPECT_EQ(hebrew.year, year);
  EXPECT_EQ(hebrew.month, month);
  EXPECT_EQ(hebrew.day, day);
}

}  // namespace

TEST(HebrewCalendar, ConvertsKnownGregorianDates) {
  expectHebrewDate({2008, 11, 13}, 5769, SiddurEngine::HebrewMonth::Cheshvan, 15);
  expectHebrewDate({2026, 9, 12}, 5787, SiddurEngine::HebrewMonth::Tishrei, 1);
}

TEST(HebrewCalendar, HandlesLeapYearAdarBoundary) {
  EXPECT_TRUE(SiddurEngine::HebrewCalendar::isLeapYear(5784));
  EXPECT_FALSE(SiddurEngine::HebrewCalendar::isLeapYear(5785));

  expectHebrewDate({2024, 3, 10}, 5784, SiddurEngine::HebrewMonth::Adar, 30);
  expectHebrewDate({2024, 3, 11}, 5784, SiddurEngine::HebrewMonth::AdarII, 1);
  expectHebrewDate({2024, 3, 24}, 5784, SiddurEngine::HebrewMonth::AdarII, 14);
  expectHebrewDate({2024, 4, 9}, 5784, SiddurEngine::HebrewMonth::Nisan, 1);
}

TEST(HebrewCalendar, HandlesCommonYearAdar) {
  expectHebrewDate({2026, 2, 18}, 5786, SiddurEngine::HebrewMonth::Adar, 1);
  EXPECT_EQ(SiddurEngine::HebrewCalendar::daysInMonth(5786, SiddurEngine::HebrewMonth::Adar), 29);
}

TEST(HebrewCalendar, HandlesVariableCheshvanAndKislevLengths) {
  EXPECT_EQ(SiddurEngine::HebrewCalendar::daysInYear(5784), 383);
  EXPECT_EQ(SiddurEngine::HebrewCalendar::daysInMonth(5784, SiddurEngine::HebrewMonth::Cheshvan), 29);
  EXPECT_EQ(SiddurEngine::HebrewCalendar::daysInMonth(5784, SiddurEngine::HebrewMonth::Kislev), 29);

  EXPECT_EQ(SiddurEngine::HebrewCalendar::daysInYear(5785), 355);
  EXPECT_EQ(SiddurEngine::HebrewCalendar::daysInMonth(5785, SiddurEngine::HebrewMonth::Cheshvan), 30);
  EXPECT_EQ(SiddurEngine::HebrewCalendar::daysInMonth(5785, SiddurEngine::HebrewMonth::Kislev), 30);

  expectHebrewDate({2024, 12, 1}, 5785, SiddurEngine::HebrewMonth::Cheshvan, 30);
  expectHebrewDate({2024, 12, 2}, 5785, SiddurEngine::HebrewMonth::Kislev, 1);
}

TEST(HebrewCalendar, DetectsBothFormsOfRoshHodesh) {
  EXPECT_TRUE(SiddurEngine::HebrewCalendar::isRoshHodesh({5785, SiddurEngine::HebrewMonth::Cheshvan, 30}));
  EXPECT_TRUE(SiddurEngine::HebrewCalendar::isRoshHodesh({5785, SiddurEngine::HebrewMonth::Kislev, 1}));
  EXPECT_FALSE(SiddurEngine::HebrewCalendar::isRoshHodesh({5785, SiddurEngine::HebrewMonth::Kislev, 15}));
}
