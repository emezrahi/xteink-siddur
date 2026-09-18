#include <LocalClock.h>
#include <gtest/gtest.h>

TEST(LocalClock, AppliesNegativeOffsetAcrossPreviousDay) {
  const SiddurEngine::CivilDateTime utc{{2026, 9, 19}, 1, 30};

  const auto local = SiddurEngine::LocalClock::applyUtcOffset(utc, 32);  // UTC-4

  EXPECT_EQ(local.date.year, 2026);
  EXPECT_EQ(local.date.month, 9);
  EXPECT_EQ(local.date.day, 18);
  EXPECT_EQ(local.hour, 21);
  EXPECT_EQ(local.minute, 30);
}

TEST(LocalClock, AppliesPositiveOffsetAcrossNewYear) {
  const SiddurEngine::CivilDateTime utc{{2026, 12, 31}, 23, 30};

  const auto local = SiddurEngine::LocalClock::applyUtcOffset(utc, 52);  // UTC+1

  EXPECT_EQ(local.date.year, 2027);
  EXPECT_EQ(local.date.month, 1);
  EXPECT_EQ(local.date.day, 1);
  EXPECT_EQ(local.hour, 0);
  EXPECT_EQ(local.minute, 30);
}

TEST(LocalClock, HandlesLeapDayWhenCrossingBackward) {
  const SiddurEngine::CivilDateTime utc{{2024, 3, 1}, 1, 0};

  const auto local = SiddurEngine::LocalClock::applyUtcOffset(utc, 40);  // UTC-2

  EXPECT_EQ(local.date.year, 2024);
  EXPECT_EQ(local.date.month, 2);
  EXPECT_EQ(local.date.day, 29);
  EXPECT_EQ(local.hour, 23);
  EXPECT_EQ(local.minute, 0);
}

TEST(LocalClock, LeavesUtcUnchangedAtZeroOffset) {
  const SiddurEngine::CivilDateTime utc{{2026, 9, 18}, 17, 34};

  const auto local = SiddurEngine::LocalClock::applyUtcOffset(utc, 48);

  EXPECT_EQ(local.date.year, 2026);
  EXPECT_EQ(local.date.month, 9);
  EXPECT_EQ(local.date.day, 18);
  EXPECT_EQ(local.hour, 17);
  EXPECT_EQ(local.minute, 34);
}
