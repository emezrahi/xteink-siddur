#pragma once

#include <PrayerBlockId.h>

#include <string_view>

namespace SiddurContent {

// A zero-copy reference to immutable CC0 Siddur text. Keeping this header
// separate from the corpus avoids pulling 145 KB of Hebrew into every activity.
struct Chapter {
  SiddurEngine::PrayerBlockId sourceBlock;
  const char* englishTitle;
  const char* hebrewTitle;
  std::string_view text;
};

} // namespace SiddurContent
