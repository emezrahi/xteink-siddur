#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

namespace SiddurStaticNavigation {

enum class Language { English, Hebrew };

struct MenuEntry {
  std::string_view id;
  std::string_view english;
  std::string_view hebrew;

  [[nodiscard]] constexpr std::string_view label(Language language) const {
    return language == Language::Hebrew ? hebrew : english;
  }
};

struct Category {
  MenuEntry entry;
  const MenuEntry* children;
  std::size_t childCount;

  [[nodiscard]] constexpr std::span<const MenuEntry> items() const { return {children, childCount}; }
};

struct NusachChoice {
  std::string_view value;
  std::string_view english;
  std::string_view hebrew;

  [[nodiscard]] constexpr std::string_view label(Language language) const {
    return language == Language::Hebrew ? hebrew : english;
  }
};

#include "StaticNavigationData.h"

[[nodiscard]] constexpr std::span<const Category> categories() { return Data::CATEGORIES; }
[[nodiscard]] constexpr std::span<const NusachChoice> nusachChoices() { return Data::NUSACH_CHOICES; }

struct PageBounds {
  std::size_t first;
  std::size_t pastLast;
};

class PagedSelection {
 public:
  constexpr PagedSelection(std::size_t itemCount, std::size_t visibleRows) : count(itemCount), rows(visibleRows) {}

  [[nodiscard]] constexpr bool valid() const { return count > 0 && rows > 0; }
  [[nodiscard]] constexpr std::optional<std::size_t> selected() const {
    return valid() ? std::optional<std::size_t>{index} : std::nullopt;
  }
  [[nodiscard]] constexpr std::optional<PageBounds> page() const {
    if (!valid()) return std::nullopt;
    const std::size_t first = (index / rows) * rows;
    return PageBounds{first, first + std::min(rows, count - first)};
  }
  constexpr void up() {
    if (valid() && index > 0) --index;
  }
  constexpr void down() {
    if (valid() && index + 1 < count) ++index;
  }
  constexpr void pageBack() {
    if (!valid()) return;
    index = index < rows ? 0 : index - rows;
  }
  constexpr void pageForward() {
    if (!valid()) return;
    index += std::min(rows, (count - 1) - index);
  }

 private:
  std::size_t count;
  std::size_t rows;
  std::size_t index = 0;
};

class NavigationSelection {
 public:
  enum class Level { Categories, Items };

  explicit constexpr NavigationSelection(std::size_t visibleRows)
      : rows(visibleRows), categoryCursor(categories().size(), visibleRows), itemCursor(0, visibleRows) {}

  [[nodiscard]] constexpr Level level() const { return currentLevel; }
  [[nodiscard]] constexpr const PagedSelection& selection() const {
    return currentLevel == Level::Categories ? categoryCursor : itemCursor;
  }
  [[nodiscard]] constexpr std::size_t categoryIndex() const { return selectedCategory; }
  [[nodiscard]] constexpr std::optional<const MenuEntry*> choose() {
    const auto selectedIndex = selection().selected();
    if (!selectedIndex) return std::nullopt;
    if (currentLevel == Level::Categories) {
      selectedCategory = *selectedIndex;
      itemCursor = PagedSelection(categories()[selectedCategory].childCount, rows);
      if (!itemCursor.valid()) return std::nullopt;
      currentLevel = Level::Items;
      return std::nullopt;
    }
    return &categories()[selectedCategory].items()[*selectedIndex];
  }
  constexpr void back() {
    if (currentLevel == Level::Items) currentLevel = Level::Categories;
  }
  constexpr void up() { active().up(); }
  constexpr void down() { active().down(); }
  constexpr void pageBack() { active().pageBack(); }
  constexpr void pageForward() { active().pageForward(); }

 private:
  [[nodiscard]] constexpr PagedSelection& active() {
    return currentLevel == Level::Categories ? categoryCursor : itemCursor;
  }

  std::size_t rows;
  Level currentLevel = Level::Categories;
  std::size_t selectedCategory = 0;
  PagedSelection categoryCursor;
  PagedSelection itemCursor;
};

}  // namespace SiddurStaticNavigation
