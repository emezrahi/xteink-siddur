#include <StaticNavigation.h>
#include <gtest/gtest.h>

#include <array>

namespace Nav = SiddurStaticNavigation;

TEST(SiddurStaticNavigation, ExposesExactCategoryShapeAndAllLeaves) {
  constexpr std::array<std::string_view, 4> categoryIds = {
      "calendar_menu_group", "special_menu", "daily_menu", "blessings_menu"};
  constexpr std::array<std::size_t, 4> itemCounts = {2, 10, 6, 8};

  ASSERT_EQ(Nav::categories().size(), categoryIds.size());
  std::size_t leafCount = 0;
  for (std::size_t index = 0; index < categoryIds.size(); ++index) {
    EXPECT_EQ(Nav::categories()[index].entry.id, categoryIds[index]);
    EXPECT_EQ(Nav::categories()[index].items().size(), itemCounts[index]);
    leafCount += Nav::categories()[index].items().size();
  }
  EXPECT_EQ(leafCount, 26U);
  EXPECT_EQ(Nav::categories()[1].items()[0].id, "omer_menu");
  EXPECT_EQ(Nav::categories()[1].items()[9].id, "nedarimMenu");
  EXPECT_EQ(Nav::categories()[3].items()[7].id, "sheva_brachot_menu");
}

TEST(SiddurStaticNavigation, SelectsLabelsWithoutAllocatingTranslations) {
  const auto& entry = Nav::categories()[2].items()[4];
  EXPECT_EQ(entry.label(Nav::Language::English), "Bedtime Shma");
  EXPECT_EQ(entry.label(Nav::Language::Hebrew), "קריאת שמע שעל המיטה");
}

TEST(SiddurStaticNavigation, KeepsAllVerifiedNusachMetadata) {
  constexpr std::array<std::string_view, 4> values = {"0", "1", "2", "3"};
  constexpr std::array<std::string_view, 4> english = {
      "Edot HaMizrach", "Sfarad", "Ashkenaz", "Ari (Chabad)"};
  ASSERT_EQ(Nav::nusachChoices().size(), values.size());
  for (std::size_t index = 0; index < values.size(); ++index) {
    EXPECT_EQ(Nav::nusachChoices()[index].value, values[index]);
    EXPECT_EQ(Nav::nusachChoices()[index].label(Nav::Language::English), english[index]);
  }
  EXPECT_EQ(Nav::nusachChoices()[3].label(Nav::Language::Hebrew), "אר\"י (חב\"ד)");
}

TEST(SiddurStaticNavigation, ComputesDiscretePagesForRequiredCapacities) {
  for (const std::size_t rows : {1U, 2U, 5U}) {
    Nav::PagedSelection selection(8, rows);
    ASSERT_TRUE(selection.valid());
    EXPECT_EQ(selection.page()->first, 0U);
    EXPECT_EQ(selection.page()->pastLast, rows);
    selection.pageForward();
    EXPECT_EQ(*selection.selected(), rows);
    while (*selection.selected() < 7) selection.pageForward();
    EXPECT_EQ(*selection.selected(), 7U);
    EXPECT_EQ(selection.page()->pastLast, 8U);
    selection.pageForward();
    EXPECT_EQ(*selection.selected(), 7U);
    while (*selection.selected() > 0) selection.pageBack();
    EXPECT_EQ(*selection.selected(), 0U);
  }
}

TEST(SiddurStaticNavigation, ClampsSingleStepMovementAtEdges) {
  Nav::PagedSelection selection(2, 1);
  selection.up();
  EXPECT_EQ(*selection.selected(), 0U);
  selection.down();
  selection.down();
  EXPECT_EQ(*selection.selected(), 1U);
}

TEST(SiddurStaticNavigation, ZeroRowsAndEmptyCollectionsFailSafely) {
  Nav::PagedSelection zeroRows(2, 0);
  Nav::PagedSelection empty(0, 2);
  EXPECT_FALSE(zeroRows.valid());
  EXPECT_FALSE(zeroRows.selected().has_value());
  EXPECT_FALSE(zeroRows.page().has_value());
  EXPECT_FALSE(empty.valid());
  EXPECT_FALSE(empty.selected().has_value());
  zeroRows.pageForward();
  empty.pageBack();
}

TEST(SiddurStaticNavigation, EntersReturnsAndChangesCategory) {
  Nav::NavigationSelection navigation(2);
  EXPECT_EQ(navigation.level(), Nav::NavigationSelection::Level::Categories);
  EXPECT_FALSE(navigation.choose().has_value());
  EXPECT_EQ(navigation.level(), Nav::NavigationSelection::Level::Items);
  EXPECT_EQ(navigation.choose().value()->id, "calendar_menu");
  navigation.back();
  navigation.down();
  EXPECT_FALSE(navigation.choose().has_value());
  navigation.down();
  EXPECT_EQ(navigation.choose().value()->id, "ushpizin_menu");
}

TEST(SiddurStaticNavigation, PrototypePrayerRowsCannotEnterStaticMenu) {
  constexpr std::array<std::string_view, 5> unverifiedIds = {
      "ModehAni", "MorningBlessings", "PesukeiDeZimra", "WeekdayAmidah", "Aleinu"};
  for (const auto& category : Nav::categories()) {
    for (const auto& item : category.items()) {
      for (const auto id : unverifiedIds) EXPECT_NE(item.id, id);
    }
  }
}
