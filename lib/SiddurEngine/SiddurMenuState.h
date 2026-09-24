#pragma once

#include <StaticNavigation.h>

#include <cstddef>
#include <optional>
#include <string_view>

namespace SiddurMenu {

enum class Screen { Navigation, Nusach, Preview, Reading };

class State {
 public:
  explicit constexpr State(const std::size_t visibleRows)
      : navigation(visibleRows), nusach(SiddurStaticNavigation::nusachChoices().size(), visibleRows) {}

  [[nodiscard]] constexpr Screen screen() const { return currentScreen; }
  [[nodiscard]] constexpr const SiddurStaticNavigation::NavigationSelection& menu() const { return navigation; }
  [[nodiscard]] constexpr const SiddurStaticNavigation::PagedSelection& nusachMenu() const { return nusach; }
  [[nodiscard]] constexpr std::size_t selectedNusach() const { return nusachIndex; }
  [[nodiscard]] constexpr std::optional<const SiddurStaticNavigation::MenuEntry*> previewEntry() const {
    return preview;
  }

  constexpr void showNusach() { currentScreen = Screen::Nusach; }
  constexpr void previous() {
    if (currentScreen == Screen::Navigation) navigation.up();
    if (currentScreen == Screen::Nusach) nusach.up();
  }
  constexpr void next() {
    if (currentScreen == Screen::Navigation) navigation.down();
    if (currentScreen == Screen::Nusach) nusach.down();
  }
  constexpr void choose() {
    if (currentScreen == Screen::Nusach) {
      const auto selected = nusach.selected();
      if (selected) nusachIndex = *selected;
      currentScreen = nusachIndex == 0 ? Screen::Navigation : Screen::Preview;
      return;
    }
    if (currentScreen != Screen::Navigation) return;
    const auto entry = navigation.choose();
    if (!entry) return;
    if ((*entry)->id == "shacharit_menu" && nusachIndex == 0) {
      currentScreen = Screen::Reading;
    } else {
      preview = *entry;
      currentScreen = Screen::Preview;
    }
  }
  // Returns false only when the activity itself should exit.
  [[nodiscard]] constexpr bool back() {
    if (currentScreen == Screen::Reading || currentScreen == Screen::Preview) {
      currentScreen = Screen::Navigation;
      return true;
    }
    if (currentScreen == Screen::Nusach) return false;
    if (navigation.level() == SiddurStaticNavigation::NavigationSelection::Level::Items) {
      navigation.back();
      return true;
    }
    currentScreen = Screen::Nusach;
    return true;
  }

 private:
  Screen currentScreen = Screen::Navigation;
  SiddurStaticNavigation::NavigationSelection navigation;
  SiddurStaticNavigation::PagedSelection nusach;
  std::size_t nusachIndex = 0;  // Only Edot has an installed prayer body in this preview.
  std::optional<const SiddurStaticNavigation::MenuEntry*> preview;
};

}  // namespace SiddurMenu
