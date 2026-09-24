#include <DynamicPrayerNavigation.h>
#include <gtest/gtest.h>

#include <array>
#include <limits>
#include <span>
#include <string>

namespace Nav = SiddurDynamicPrayerNavigation;

TEST(SiddurDynamicPrayerNavigation, PreservesInputOrderIndicesAndRepeatedTitles) {
  constexpr std::array rows = {
      Nav::PrayerRow{"hidden", "Hidden", false},
      Nav::PrayerRow{"first", "שחרית", true, true},
      Nav::PrayerRow{"hidden2", "Not a menu row", false},
      Nav::PrayerRow{"second", "שחרית", true},
      Nav::PrayerRow{"last", "סיום", true},
  };
  Nav::PrayerContents contents(rows, 2);
  ASSERT_EQ(contents.menuCount(), 3U);
  ASSERT_TRUE(contents.entry(0).has_value());
  EXPECT_EQ(contents.entry(0)->sourceRowIndex, 1U);
  EXPECT_EQ(contents.entry(0)->title, "שחרית");
  EXPECT_TRUE(contents.entry(0)->expandable);
  EXPECT_EQ(contents.entry(1)->sourceRowIndex, 3U);
  EXPECT_EQ(contents.entry(1)->title, contents.entry(0)->title);
  EXPECT_EQ(contents.entry(2)->sourceRowIndex, 4U);
  EXPECT_EQ(contents.entry(2)->title, "סיום");
  EXPECT_FALSE(contents.entry(3).has_value());
  EXPECT_EQ(contents.page()->first, 0U);
  EXPECT_EQ(contents.pageEntry(1)->sourceRowIndex, 3U);
  EXPECT_FALSE(contents.pageEntry(2).has_value());
  contents.down();
  EXPECT_EQ(contents.choose(), std::optional<std::size_t>{3});
  EXPECT_EQ(contents.screen(), Nav::PrayerContents::Screen::Reading);
  EXPECT_EQ(contents.readingSourceRow(), std::optional<std::size_t>{3});
  contents.down();  // Ignore contents controls while reading.
  contents.back();
  EXPECT_EQ(contents.screen(), Nav::PrayerContents::Screen::Contents);
  EXPECT_EQ(contents.selected()->sourceRowIndex, 3U);
  EXPECT_EQ(contents.choose(), std::optional<std::size_t>{3});
}

TEST(SiddurDynamicPrayerNavigation, EmptyHiddenAndZeroCapacityAreSafe) {
  constexpr std::array<Nav::PrayerRow, 0> none{};
  constexpr std::array hidden = {Nav::PrayerRow{"hidden", "Hidden", false}};
  constexpr std::array shown = {Nav::PrayerRow{"shown", "Shown", true}};
  for (const auto& rows : {std::span<const Nav::PrayerRow>{none}, std::span<const Nav::PrayerRow>{hidden}}) {
    Nav::PrayerContents contents(rows, 2);
    EXPECT_EQ(contents.menuCount(), 0U);
    EXPECT_FALSE(contents.hasContents());
    EXPECT_FALSE(contents.selected().has_value());
    EXPECT_FALSE(contents.page().has_value());
    EXPECT_FALSE(contents.pageEntry(0).has_value());
    EXPECT_FALSE(contents.choose().has_value());
    contents.down();
    contents.pageForward();
    contents.pageBack();
    contents.back();
  }
  Nav::PrayerContents invalid(shown, 0);
  EXPECT_EQ(invalid.menuCount(), 1U);
  EXPECT_FALSE(invalid.hasContents());
  EXPECT_FALSE(invalid.selected().has_value());
  EXPECT_FALSE(invalid.choose().has_value());
}

TEST(SiddurDynamicPrayerNavigation, DifferentInputsDetermineDifferentMenus) {
  constexpr std::array weekday = {
      Nav::PrayerRow{"a", "א", true},
      Nav::PrayerRow{"conditional", "ב", false},
      Nav::PrayerRow{"c", "ג", true},
  };
  constexpr std::array otherDay = {
      Nav::PrayerRow{"a", "א", true},
      Nav::PrayerRow{"conditional", "ב", true},
      Nav::PrayerRow{"c", "ג", false},
  };
  Nav::PrayerContents first(weekday, 1);
  Nav::PrayerContents second(otherDay, 1);
  EXPECT_EQ(first.entry(1)->id, "c");
  EXPECT_EQ(first.entry(1)->sourceRowIndex, 2U);
  EXPECT_EQ(second.entry(1)->id, "conditional");
  EXPECT_EQ(second.entry(1)->sourceRowIndex, 1U);
}

TEST(SiddurDynamicPrayerNavigation, DiscretePageCapacitiesAndEdgeClamps) {
  constexpr std::array rows = {
      Nav::PrayerRow{"0", "First", true},   Nav::PrayerRow{"1", "Second", true}, Nav::PrayerRow{"2", "Third", true},
      Nav::PrayerRow{"3", "Fourth", true},  Nav::PrayerRow{"4", "Fifth", true},  Nav::PrayerRow{"5", "Sixth", true},
      Nav::PrayerRow{"6", "Seventh", true}, Nav::PrayerRow{"7", "Eighth", true},
  };
  for (const std::size_t capacity : {1U, 2U, 5U}) {
    Nav::PrayerContents contents(rows, capacity);
    contents.up();
    contents.pageBack();
    EXPECT_EQ(contents.selected()->sourceRowIndex, 0U);
    EXPECT_EQ(contents.page()->pastLast, capacity);
    contents.pageForward();
    EXPECT_EQ(contents.selected()->sourceRowIndex, capacity);
    while (contents.selected()->sourceRowIndex < 7U) contents.pageForward();
    EXPECT_EQ(contents.selected()->sourceRowIndex, 7U);
    EXPECT_EQ(contents.page()->pastLast, 8U);
    contents.pageForward();
    EXPECT_EQ(contents.selected()->sourceRowIndex, 7U);
    while (contents.selected()->sourceRowIndex > 0U) contents.pageBack();
    EXPECT_EQ(contents.selected()->sourceRowIndex, 0U);
  }
}

TEST(SiddurDynamicPrayerNavigation, HugePageCapacityNeverOverflows) {
  constexpr std::array rows = {Nav::PrayerRow{"0", "Zero", true}, Nav::PrayerRow{"1", "One", true},
                               Nav::PrayerRow{"2", "Two", true}, Nav::PrayerRow{"3", "Three", true}};
  Nav::PrayerContents contents(rows, std::numeric_limits<std::size_t>::max());
  contents.down();
  contents.pageForward();
  EXPECT_EQ(contents.selected()->sourceRowIndex, 3U);
  ASSERT_TRUE(contents.page().has_value());
  EXPECT_EQ(contents.page()->first, 0U);
  EXPECT_EQ(contents.page()->pastLast, 4U);
  EXPECT_FALSE(contents.pageEntry(std::numeric_limits<std::size_t>::max()).has_value());
}

TEST(SiddurDynamicPrayerNavigation, SingleVisibleEntryHasStableBackNavigation) {
  constexpr std::array rows = {Nav::PrayerRow{"x", "Skip", false}, Nav::PrayerRow{"y", "יחיד", true}};
  Nav::PrayerContents contents(rows, 5);
  EXPECT_EQ(contents.menuCount(), 1U);
  EXPECT_EQ(contents.choose(), std::optional<std::size_t>{1});
  EXPECT_FALSE(contents.choose().has_value());
  contents.back();
  EXPECT_EQ(contents.selected()->sourceRowIndex, 1U);
  EXPECT_EQ(contents.choose(), std::optional<std::size_t>{1});
}

TEST(SiddurDynamicPrayerNavigation, UsesStableDynamicallyOwnedLabelStorage) {
  // Models generator-owned text buffers that outlive both rows and navigator.
  const std::array<std::string, 3> ownedText = {"row-id", "תפילה לדוגמה", "second-title"};
  const std::array rows = {
      Nav::PrayerRow{ownedText[0], ownedText[1], true},
      Nav::PrayerRow{"second-id", ownedText[2], true},
  };
  const Nav::PrayerContents contents(rows, 2);
  ASSERT_TRUE(contents.entry(0).has_value());
  EXPECT_EQ(contents.entry(0)->id, ownedText[0]);
  EXPECT_EQ(contents.entry(0)->title, ownedText[1]);
  EXPECT_EQ(contents.entry(1)->title, ownedText[2]);
}
