#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace SiddurEngine {

// Screen-space calculation is independent of the content. Both the height of
// the visible glyphs and the full baseline advance matter: fitting 11 lines by
// count alone was what put the X3's last lines underneath its button hints.
struct PageGeometry {
  static int visibleLines(const int bodyTop, const int bodyBottom, const int glyphHeight, const int advance) {
    if (bodyBottom - bodyTop < glyphHeight || glyphHeight <= 0 || advance <= 0) return 0;
    return 1 + (bodyBottom - bodyTop - glyphHeight) / advance;
  }
};

struct Utf8Page {
  std::vector<std::string> lines;
  std::size_t nextOffset = 0;
  bool hasNext = false;
};

class Utf8Pager {
  static bool whitespace(const char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

  static std::size_t skipWhitespace(const std::string_view text, std::size_t pos) {
    while (pos < text.size() && whitespace(text[pos])) ++pos;
    return pos;
  }

  static std::size_t nextScalar(const std::string_view text, const std::size_t pos, const std::size_t end) {
    const auto c = static_cast<unsigned char>(text[pos]);
    const std::size_t length = c < 0x80 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4;
    return std::min(end, pos + length);
  }

  static std::uint32_t scalar(const std::string_view text, const std::size_t begin, const std::size_t end) {
    const auto c = static_cast<unsigned char>(text[begin]);
    if (c < 0x80) return c;
    const std::size_t count = end - begin;
    std::uint32_t value = c & (count == 2 ? 0x1F : count == 3 ? 0x0F : 0x07);
    for (std::size_t i = begin + 1; i < end; ++i) value = (value << 6) | (static_cast<unsigned char>(text[i]) & 0x3F);
    return value;
  }

  static bool combining(const std::uint32_t codePoint) {
    // Hebrew niqqud / cantillation, Arabic vowels, and general combining marks.
    return (codePoint >= 0x0591 && codePoint <= 0x05C7) || (codePoint >= 0x0610 && codePoint <= 0x061A) ||
           (codePoint >= 0x064B && codePoint <= 0x065F) || (codePoint >= 0x0300 && codePoint <= 0x036F);
  }

  static std::size_t nextCluster(const std::string_view text, const std::size_t start, const std::size_t end) {
    std::size_t pos = nextScalar(text, start, end);
    while (pos < end) {
      const std::size_t next = nextScalar(text, pos, end);
      if (!combining(scalar(text, pos, next))) break;
      pos = next;
    }
    return pos;
  }

 public:
  // Measure receives an ordinary NUL-terminated UTF-8 string, shaped by the
  // same RTL-aware font path used for drawing. Offsets always refer to original
  // source bytes, so page turns cannot silently lose long words or niqqud.
  template <typename Measure>
  static Utf8Page paginate(const std::string_view text, const std::size_t startOffset, const int maxWidth,
                           const int maxLines, Measure&& measure) {
    Utf8Page page;
    page.nextOffset = std::min(startOffset, text.size());
    if (maxWidth <= 0 || maxLines <= 0 || startOffset >= text.size()) return page;
    page.lines.reserve(static_cast<std::size_t>(maxLines));
    std::size_t pos = skipWhitespace(text, startOffset);
    std::string current;

    auto flush = [&]() {
      page.lines.push_back(std::move(current));
      current.clear();
    };

    while (pos < text.size() && static_cast<int>(page.lines.size()) < maxLines) {
      if (text[pos] == '\n' || text[pos] == '\r') {
        if (!current.empty()) {
          flush();
        } else if (!page.lines.empty() && !page.lines.back().empty()) {
          page.lines.emplace_back(); // visible paragraph break, if space permits
        }
        ++pos;
        continue;
      }
      if (text[pos] == ' ' || text[pos] == '\t') {
        ++pos;
        continue;
      }

      const std::size_t wordStart = pos;
      while (pos < text.size() && !whitespace(text[pos])) ++pos;
      const std::size_t wordEnd = pos;
      const std::string_view word = text.substr(wordStart, wordEnd - wordStart);
      std::string candidate = current.empty() ? std::string(word) : current + " " + std::string(word);
      if (measure(candidate.c_str()) <= maxWidth) {
        current = std::move(candidate);
        continue;
      }
      if (!current.empty()) {
        flush();
        pos = wordStart; // the overflowing word belongs entirely to the next line
        continue;
      }

      // A single source token is wider than a whole line. Unlike truncatedText,
      // split it at UTF-8 grapheme boundaries and resume at the exact byte.
      std::size_t cursor = wordStart;
      std::string fragment;
      while (cursor < wordEnd && static_cast<int>(page.lines.size()) < maxLines) {
        const auto next = nextCluster(text, cursor, wordEnd);
        const std::string cluster(text.substr(cursor, next - cursor));
        const std::string proposal = fragment + cluster;
        if (!fragment.empty() && measure(proposal.c_str()) > maxWidth) {
          page.lines.push_back(std::move(fragment));
          fragment.clear();
          continue;
        }
        fragment = proposal; // always consume at least one cluster
        cursor = next;
      }
      if (!fragment.empty() && static_cast<int>(page.lines.size()) < maxLines) {
        current = std::move(fragment);
      }
      pos = cursor;
      if (cursor < wordEnd) break;
    }

    if (!current.empty() && static_cast<int>(page.lines.size()) < maxLines) flush();
    page.nextOffset = skipWhitespace(text, pos);
    page.hasNext = page.nextOffset < text.size();
    return page;
  }

  template <typename Measure>
  static std::pair<std::size_t, std::size_t> lastPage(const std::string_view text, const int maxWidth,
                                                      const int maxLines, Measure&& measure) {
    std::size_t offset = 0;
    std::size_t index = 0;
    while (true) {
      const auto page = paginate(text, offset, maxWidth, maxLines, measure);
      if (!page.hasNext || page.nextOffset <= offset) return {offset, index};
      offset = page.nextOffset;
      ++index;
    }
  }

  template <typename Measure>
  static std::size_t previousOffset(const std::string_view text, const std::size_t currentOffset,
                                    const int maxWidth, const int maxLines, Measure&& measure) {
    std::size_t offset = 0;
    while (offset < currentOffset) {
      const auto page = paginate(text, offset, maxWidth, maxLines, measure);
      if (!page.hasNext || page.nextOffset >= currentOffset || page.nextOffset <= offset) return offset;
      offset = page.nextOffset;
    }
    return 0;
  }
};

} // namespace SiddurEngine
