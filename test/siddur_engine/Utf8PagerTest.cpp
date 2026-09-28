#include <Utf8Pager.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace {
int codepointWidth(const char* text) {
  int width = 0;
  for (const unsigned char* p = reinterpret_cast<const unsigned char*>(text); *p; ++p) {
    if ((*p & 0xC0) != 0x80) ++width;
  }
  return width * 11;
}

std::string removeSpaces(const std::string_view text) {
  std::string result;
  for (const auto c : text) if (c != ' ' && c != '\n' && c != '\r' && c != '\t') result += c;
  return result;
}
}

TEST(Utf8Pager, FooterNeverOverlapsLastVisibleLine) {
  constexpr int bodyTop = 138;
  constexpr int labelTop = 710;
  constexpr int bodyBottom = labelTop - 14;
  constexpr int glyphHeight = 38;
  constexpr int lineAdvance = 51;
  const int count = SiddurEngine::PageGeometry::visibleLines(bodyTop, bodyBottom, glyphHeight, lineAdvance);
  ASSERT_GT(count, 0);
  EXPECT_LE(bodyTop + (count - 1) * lineAdvance + glyphHeight, bodyBottom);
  EXPECT_GT(bodyTop + count * lineAdvance + glyphHeight, bodyBottom);
  EXPECT_EQ(SiddurEngine::PageGeometry::visibleLines(130, 135, 40, 46), 0);
}

TEST(Utf8Pager, NoLostWordsAcrossMultiplePages) {
  const std::string text = "one two three four five six seven eight nine ten eleven twelve";
  std::string rebuilt;
  std::size_t offset = 0;
  for (int pageNo = 0; pageNo < 30; ++pageNo) {
    const auto page = SiddurEngine::Utf8Pager::paginate(text, offset, 100, 2, codepointWidth);
    ASSERT_FALSE(page.lines.empty());
    for (const auto& line : page.lines) rebuilt += line;
    if (!page.hasNext) break;
    ASSERT_GT(page.nextOffset, offset);
    offset = page.nextOffset;
  }
  EXPECT_EQ(removeSpaces(rebuilt), removeSpaces(text));
}

TEST(Utf8Pager, HebrewAndCombiningMarksAreNeverTruncated) {
  // Each Hebrew base codepoint is followed by niqqud; even a single long
  // unbroken word must survive every page transition in full.
  const std::string text = "שָׁלוֹםשָׁלוֹםשָׁלוֹםשָׁלוֹם";
  std::size_t offset = 0;
  std::string rebuilt;
  for (int step = 0; step < 30; ++step) {
    const auto page = SiddurEngine::Utf8Pager::paginate(text, offset, 44, 2, codepointWidth);
    for (const auto& line : page.lines) rebuilt += line;
    if (!page.hasNext) break;
    ASSERT_GT(page.nextOffset, offset);
    offset = page.nextOffset;
  }
  EXPECT_EQ(rebuilt, text);
}

TEST(Utf8Pager, PreviousAndLastPagePreserveExactBoundaries) {
  const std::string text = "alpha beta gamma delta epsilon zeta eta theta iota kappa lambda mu";
  auto measure = codepointWidth;
  const auto first = SiddurEngine::Utf8Pager::paginate(text, 0, 110, 2, measure);
  ASSERT_TRUE(first.hasNext);
  const auto second = SiddurEngine::Utf8Pager::paginate(text, first.nextOffset, 110, 2, measure);
  EXPECT_EQ(SiddurEngine::Utf8Pager::previousOffset(text, first.nextOffset, 110, 2, measure), 0U);
  if (second.hasNext) {
    EXPECT_EQ(SiddurEngine::Utf8Pager::previousOffset(text, second.nextOffset, 110, 2, measure), first.nextOffset);
  }
  const auto last = SiddurEngine::Utf8Pager::lastPage(text, 110, 2, measure);
  const auto tail = SiddurEngine::Utf8Pager::paginate(text, last.first, 110, 2, measure);
  EXPECT_FALSE(tail.hasNext);
  EXPECT_LT(last.second, 20U);
}

TEST(Utf8Pager, ParagraphBreaksStayInsideAllocatedLines) {
  const std::string text = "one two\n\nthree four\n\nfive six seven";
  std::size_t offset = 0;
  std::string rebuilt;
  for (int i = 0; i < 20; ++i) {
    const auto page = SiddurEngine::Utf8Pager::paginate(text, offset, 150, 3, codepointWidth);
    ASSERT_LE(page.lines.size(), 3U);
    for (const auto& line : page.lines) rebuilt += line;
    if (!page.hasNext) break;
    ASSERT_GT(page.nextOffset, offset);
    offset = page.nextOffset;
  }
  EXPECT_EQ(removeSpaces(rebuilt), removeSpaces(text));
}


TEST(Utf8Pager, HebrewHeadingOnlyOnFirstPageWithExactReversePaging) {
  // The first page reserves room for a Hebrew title, later pages do not.
  constexpr int firstBodyTop = 138;
  constexpr int nextBodyTop = 91;
  constexpr int bodyBottom = 696;
  constexpr int glyphHeight = 38;
  constexpr int lineAdvance = 51;
  const int firstLines = SiddurEngine::PageGeometry::visibleLines(firstBodyTop, bodyBottom, glyphHeight, lineAdvance);
  const int nextLines = SiddurEngine::PageGeometry::visibleLines(nextBodyTop, bodyBottom, glyphHeight, lineAdvance);
  ASSERT_GT(nextLines, firstLines);
  std::string text;
  for (int i = 0; i < 120; ++i) text += "שְׁמַע יִשְׂרָאֵל אַהֲבָה ";
  std::vector<std::size_t> starts{0};
  std::string rebuilt;
  for (int pageNo = 0; pageNo < 100; ++pageNo) {
    const auto page = SiddurEngine::Utf8Pager::paginate(text, starts.back(), 170,
                                                        pageNo == 0 ? firstLines : nextLines, codepointWidth);
    ASSERT_FALSE(page.lines.empty());
    for (const auto& line : page.lines) rebuilt += line;
    if (!page.hasNext) break;
    ASSERT_GT(page.nextOffset, starts.back());
    starts.push_back(page.nextOffset);
  }
  EXPECT_EQ(removeSpaces(rebuilt), removeSpaces(text));
  ASSERT_GT(starts.size(), 2U);
  for (std::size_t i = 1; i < starts.size(); ++i) {
    EXPECT_EQ(SiddurEngine::Utf8Pager::previousOffsetVariable(text, starts[i], 170,
               firstLines, nextLines, codepointWidth), starts[i - 1]);
  }
  const auto last = SiddurEngine::Utf8Pager::lastPageVariable(text, 170,
                                                             firstLines, nextLines, codepointWidth);
  EXPECT_EQ(last.first, starts.back());
  EXPECT_EQ(last.second, starts.size() - 1);
}
