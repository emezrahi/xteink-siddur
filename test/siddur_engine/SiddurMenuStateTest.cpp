#include <SiddurMenuState.h>
#include <gtest/gtest.h>

namespace Menu = SiddurMenu;
namespace Nav = SiddurStaticNavigation;

TEST(SiddurMenuState, NavigatesAllCategoriesAndPreservesSelectionOnReturn) {
  Menu::State state(2);
  EXPECT_EQ(state.screen(), Menu::Screen::Navigation);
  state.next();
  state.next();
  state.choose();
  EXPECT_EQ(state.menu().level(), Nav::NavigationSelection::Level::Items);
  state.choose();
  EXPECT_EQ(state.screen(), Menu::Screen::Reading);
  EXPECT_TRUE(state.back());
  EXPECT_EQ(state.screen(), Menu::Screen::Navigation);
  EXPECT_EQ(*state.menu().selection().selected(), 0U);
}

TEST(SiddurMenuState, NonShaharitEntriesArePreviewOnly) {
  Menu::State state(4);
  state.choose();
  state.choose();
  EXPECT_EQ(state.screen(), Menu::Screen::Preview);
  ASSERT_TRUE(state.previewEntry().has_value());
  EXPECT_EQ(state.previewEntry().value()->id, "calendar_menu");
}

TEST(SiddurMenuState, GatesPrayerBodyByNusachAvailability) {
  Menu::State state(4);
  state.showNusach();
  state.next();
  state.choose();
  EXPECT_EQ(state.selectedNusach(), 1U);
  EXPECT_EQ(state.screen(), Menu::Screen::Preview);
  EXPECT_TRUE(state.back());
  state.next();
  state.next();
  state.choose();
  state.choose();
  EXPECT_EQ(state.screen(), Menu::Screen::Preview);
}

TEST(SiddurMenuState, SupportsSmallLargeAndZeroRowCapacities) {
  Menu::State small(1);
  small.next();
  EXPECT_EQ(small.menu().selection().page()->first, 1U);

  Menu::State large(20);
  large.next();
  EXPECT_EQ(large.menu().selection().page()->first, 0U);

  Menu::State zero(0);
  zero.choose();
  zero.next();
  EXPECT_EQ(zero.screen(), Menu::Screen::Navigation);
  EXPECT_FALSE(zero.menu().selection().page().has_value());
}

TEST(SiddurMenuState, ExposesAllFourNusachimAndTwentySixLeaves) {
  EXPECT_EQ(Nav::nusachChoices().size(), 4U);
  std::size_t leaves = 0;
  for (const auto& category : Nav::categories()) leaves += category.items().size();
  EXPECT_EQ(Nav::categories().size(), 4U);
  EXPECT_EQ(leaves, 26U);
}
