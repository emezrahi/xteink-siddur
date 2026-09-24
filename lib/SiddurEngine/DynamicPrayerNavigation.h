#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

#include "StaticNavigation.h"

namespace SiddurDynamicPrayerNavigation {

// Descriptors must remain alive and unchanged for the navigator's lifetime.
// sourceRowIndex is the descriptor's position in the caller's ordered row span.
struct PrayerRow {
  std::string_view id;
  std::string_view title;
  bool addToMenu = false;
  bool expandable = false;
};

struct MenuEntry {
  std::size_t sourceRowIndex;
  std::string_view id;
  std::string_view title;
  bool expandable;
};

// Stores only a non-owning span and two small cursor values. The visible menu
// is derived from the generator's actual addToMenu flags, never guessed titles.
class PrayerContents {
 public:
  enum class Screen { Contents, Reading };
  using PageBounds = SiddurStaticNavigation::PageBounds;

  constexpr PrayerContents(std::span<const PrayerRow> rows, std::size_t visibleRows)
      : rows(rows), cursor(countEligible(rows), visibleRows) {}

  [[nodiscard]] constexpr std::size_t menuCount() const { return cursorCount(); }
  [[nodiscard]] constexpr Screen screen() const { return currentScreen; }
  [[nodiscard]] constexpr bool hasContents() const { return cursor.valid(); }

  [[nodiscard]] constexpr std::optional<PageBounds> page() const { return cursor.page(); }

  // The ordinal is among menu-eligible items, not an index into the input rows.
  [[nodiscard]] constexpr std::optional<MenuEntry> entry(std::size_t ordinal) const {
    if (ordinal >= menuCount()) return std::nullopt;
    std::size_t eligible = 0;
    for (std::size_t index = 0; index < rows.size(); ++index) {
      if (!rows[index].addToMenu) continue;
      if (eligible == ordinal) {
        return MenuEntry{index, rows[index].id, rows[index].title, rows[index].expandable};
      }
      ++eligible;
    }
    return std::nullopt;
  }

  [[nodiscard]] constexpr std::optional<MenuEntry> selected() const {
    const auto position = cursor.selected();
    return position ? entry(*position) : std::nullopt;
  }

  // Offset inside the current discrete page, useful for row-by-row rendering.
  [[nodiscard]] constexpr std::optional<MenuEntry> pageEntry(std::size_t offset) const {
    const auto bounds = page();
    if (!bounds || offset >= bounds->pastLast - bounds->first) return std::nullopt;
    return entry(bounds->first + offset);
  }

  // Returns the original source-row index for the prayer reader.
  [[nodiscard]] constexpr std::optional<std::size_t> choose() {
    if (currentScreen != Screen::Contents) return std::nullopt;
    const auto choice = selected();
    if (!choice) return std::nullopt;
    readingIndex = choice->sourceRowIndex;
    currentScreen = Screen::Reading;
    return readingIndex;
  }

  // Returning from reading keeps the same selected contents item and page.
  constexpr void back() {
    if (currentScreen == Screen::Reading) currentScreen = Screen::Contents;
  }

  [[nodiscard]] constexpr std::optional<std::size_t> readingSourceRow() const {
    return currentScreen == Screen::Reading ? readingIndex : std::nullopt;
  }

  constexpr void up() {
    if (currentScreen == Screen::Contents) cursor.up();
  }
  constexpr void down() {
    if (currentScreen == Screen::Contents) cursor.down();
  }
  constexpr void pageBack() {
    if (currentScreen == Screen::Contents) cursor.pageBack();
  }
  constexpr void pageForward() {
    if (currentScreen == Screen::Contents) cursor.pageForward();
  }

 private:
  [[nodiscard]] static constexpr std::size_t countEligible(std::span<const PrayerRow> input) {
    std::size_t count = 0;
    for (const auto& row : input) {
      if (row.addToMenu) ++count;
    }
    return count;
  }

  [[nodiscard]] constexpr std::size_t cursorCount() const { return countEligible(rows); }

  std::span<const PrayerRow> rows;
  SiddurStaticNavigation::PagedSelection cursor;
  Screen currentScreen = Screen::Contents;
  std::optional<std::size_t> readingIndex;
};

}  // namespace SiddurDynamicPrayerNavigation
