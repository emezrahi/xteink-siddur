#include "SiddurActivity.h"

#include <Composer.h>
#include <HalClock.h>
#include <HalDisplay.h>
#include <HebrewCalendar.h>
#include <LocalClock.h>
#include <PrayerContextResolver.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "CrossPointSettings.h"
#include "components/UITheme.h"
#include "content/EdotWeekdayShaharit.h"
#include "fontIds.h"

namespace {
constexpr int kSideMargin = 24;
constexpr int kTitleY = 46;
constexpr int kSectionTitleY = 105;
constexpr int kPrayerTitleY = 155;
constexpr int kPrayerStartY = 215;
constexpr int kLineGap = 9;
constexpr int kMaxPrayerLines = 11;
constexpr int kPageNumberY = 705;

struct TextPage {
  std::vector<std::string> lines;
  std::size_t nextOffset = 0;
  bool hasNext = false;
};

const char* hebrewMonthName(const SiddurEngine::HebrewMonth month) {
  switch (month) {
    case SiddurEngine::HebrewMonth::Nisan:
      return "Nisan";
    case SiddurEngine::HebrewMonth::Iyar:
      return "Iyar";
    case SiddurEngine::HebrewMonth::Sivan:
      return "Sivan";
    case SiddurEngine::HebrewMonth::Tammuz:
      return "Tammuz";
    case SiddurEngine::HebrewMonth::Av:
      return "Av";
    case SiddurEngine::HebrewMonth::Elul:
      return "Elul";
    case SiddurEngine::HebrewMonth::Tishrei:
      return "Tishrei";
    case SiddurEngine::HebrewMonth::Cheshvan:
      return "Cheshvan";
    case SiddurEngine::HebrewMonth::Kislev:
      return "Kislev";
    case SiddurEngine::HebrewMonth::Tevet:
      return "Tevet";
    case SiddurEngine::HebrewMonth::Shevat:
      return "Shevat";
    case SiddurEngine::HebrewMonth::Adar:
      return "Adar";
    case SiddurEngine::HebrewMonth::AdarII:
      return "Adar II";
  }
  return "";
}

TextPage layoutTextPage(GfxRenderer& renderer, const char* text, const std::size_t startOffset, const int maxWidth) {
  TextPage page;
  page.lines.reserve(kMaxPrayerLines);
  if (text == nullptr || text[startOffset] == '\0') {
    page.nextOffset = startOffset;
    return page;
  }

  std::size_t pos = startOffset;
  while (text[pos] == ' ' || text[pos] == '\n') ++pos;

  std::string currentLine;
  currentLine.reserve(192);

  while (text[pos] != '\0' && static_cast<int>(page.lines.size()) < kMaxPrayerLines) {
    if (text[pos] == '\n') {
      if (!currentLine.empty()) {
        page.lines.push_back(std::move(currentLine));
        currentLine.clear();
        currentLine.reserve(192);
      } else {
        page.lines.emplace_back();
      }
      ++pos;
      continue;
    }

    while (text[pos] == ' ') ++pos;
    if (text[pos] == '\0' || text[pos] == '\n') continue;

    const std::size_t wordStart = pos;
    while (text[pos] != '\0' && text[pos] != ' ' && text[pos] != '\n') ++pos;
    std::string token(text + wordStart, pos - wordStart);
    std::string candidate = currentLine.empty() ? token : currentLine + " " + token;

    if (renderer.getTextWidth(SIDDUR_HEBREW_16_FONT_ID, candidate.c_str(), EpdFontFamily::REGULAR,
                              BidiUtils::BidiBaseDir::RTL) <= maxWidth) {
      currentLine = std::move(candidate);
      continue;
    }

    if (!currentLine.empty()) {
      page.lines.push_back(std::move(currentLine));
      currentLine.clear();
      currentLine.reserve(192);

      if (static_cast<int>(page.lines.size()) >= kMaxPrayerLines) {
        pos = wordStart;
        break;
      }
    }

    if (renderer.getTextWidth(SIDDUR_HEBREW_16_FONT_ID, token.c_str(), EpdFontFamily::REGULAR,
                              BidiUtils::BidiBaseDir::RTL) <= maxWidth) {
      currentLine = std::move(token);
    } else {
      page.lines.push_back(renderer.truncatedText(SIDDUR_HEBREW_16_FONT_ID, token.c_str(), maxWidth));
      if (static_cast<int>(page.lines.size()) >= kMaxPrayerLines) break;
    }
  }

  if (!currentLine.empty() && static_cast<int>(page.lines.size()) < kMaxPrayerLines) {
    page.lines.push_back(std::move(currentLine));
  }

  while (text[pos] == ' ' || text[pos] == '\n') ++pos;
  page.nextOffset = pos;
  page.hasNext = text[pos] != '\0';
  return page;
}

std::pair<std::size_t, std::size_t> lastPagePosition(GfxRenderer& renderer, const char* text, const int maxWidth) {
  std::size_t offset = 0;
  std::size_t pageIndex = 0;

  while (true) {
    const auto page = layoutTextPage(renderer, text, offset, maxWidth);
    if (!page.hasNext || page.nextOffset <= offset) return {offset, pageIndex};
    offset = page.nextOffset;
    ++pageIndex;
  }
}

std::size_t previousPageOffset(GfxRenderer& renderer, const char* text, const std::size_t currentOffset,
                               const int maxWidth) {
  if (currentOffset == 0) return 0;

  std::size_t offset = 0;
  while (true) {
    const auto page = layoutTextPage(renderer, text, offset, maxWidth);
    if (!page.hasNext || page.nextOffset >= currentOffset || page.nextOffset <= offset) return offset;
    offset = page.nextOffset;
  }
}
}  // namespace

void SiddurActivity::onEnter() {
  Activity::onEnter();
  view = View::Menu;
  prayerIndex = 0;
  resetTextPage();
  refreshCalendarPreview();
  cleanRefreshPending = true;
  requestUpdate();
}

void SiddurActivity::refreshCalendarPreview() {
  Rtc::DateTime utc;
  if (!halClock.getUtcDateTime(utc)) {
    hasLocalCivilDate = false;
    localDateTimePreview = "RTC date unavailable";
    hebrewDatePreview.clear();
    return;
  }

  const SiddurEngine::CivilDateTime utcDateTime{
      {static_cast<int>(utc.year), static_cast<int>(utc.month), static_cast<int>(utc.day)},
      static_cast<int>(utc.hour),
      static_cast<int>(utc.minute),
  };
  const auto localDateTime = SiddurEngine::LocalClock::applyUtcOffset(utcDateTime, SETTINGS.clockUtcOffsetQ);
  localCivilDate = localDateTime.date;
  hasLocalCivilDate = true;
  const auto hebrewDate = SiddurEngine::HebrewCalendar::fromGregorian(localDateTime.date);

  char localBuffer[32];
  std::snprintf(localBuffer, sizeof(localBuffer), "%04d-%02d-%02d  %02d:%02d", localDateTime.date.year,
                localDateTime.date.month, localDateTime.date.day, localDateTime.hour, localDateTime.minute);
  localDateTimePreview = localBuffer;

  char hebrewBuffer[48];
  std::snprintf(hebrewBuffer, sizeof(hebrewBuffer), "Daytime Hebrew date: %d %s %d", hebrewDate.day,
                hebrewMonthName(hebrewDate.month), hebrewDate.year);
  hebrewDatePreview = hebrewBuffer;
}

void SiddurActivity::resetTextPage() {
  textOffset = 0;
  nextTextOffset = 0;
  textPageIndex = 0;
  hasNextTextPage = false;
}

void SiddurActivity::openShaharit() {
  if (hasLocalCivilDate) {
    prayerContext =
        SiddurEngine::PrayerContextResolver::resolve(SiddurEngine::PrayerService::Shaharit, localCivilDate, false)
            .context;
  } else {
    prayerContext = {};
    prayerContext.service = SiddurEngine::PrayerService::Shaharit;
  }

  composedPrayer = SiddurEngine::Composer::compose(prayerContext);
  if (composedPrayer.empty()) return;

  view = View::Shaharit;
  prayerIndex = 0;
  resetTextPage();
  cleanRefreshPending = true;
  requestUpdate();
}

void SiddurActivity::showPreviousPrayer() {
  if (composedPrayer.empty()) return;

  const int maxWidth = renderer.getScreenWidth() - 2 * kSideMargin;
  const auto* block = SiddurContent::EdotWeekdayShaharit::findBlock(composedPrayer[prayerIndex]);

  if (block != nullptr && textOffset > 0) {
    textOffset = previousPageOffset(renderer, block->text, textOffset, maxWidth);
    if (textPageIndex > 0) --textPageIndex;
    requestUpdate();
    return;
  }

  if (prayerIndex == 0) return;
  --prayerIndex;

  block = SiddurContent::EdotWeekdayShaharit::findBlock(composedPrayer[prayerIndex]);
  if (block == nullptr) {
    resetTextPage();
  } else {
    const auto last = lastPagePosition(renderer, block->text, maxWidth);
    textOffset = last.first;
    textPageIndex = last.second;
    nextTextOffset = textOffset;
    hasNextTextPage = false;
  }
  requestUpdate();
}

void SiddurActivity::showNextPrayer() {
  if (hasNextTextPage) {
    textOffset = nextTextOffset;
    ++textPageIndex;
    requestUpdate();
    return;
  }

  if (prayerIndex + 1 >= composedPrayer.size()) return;
  ++prayerIndex;
  resetTextPage();
  requestUpdate();
}

void SiddurActivity::loop() {
  if (view == View::Menu) {
    if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
      finish();
      return;
    }

    if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
      openShaharit();
    }
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    view = View::Menu;
    cleanRefreshPending = true;
    requestUpdate();
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::NavPrevious)) {
    showPreviousPrayer();
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::NavNext)) {
    showNextPrayer();
  }
}

void SiddurActivity::render(RenderLock&&) {
  renderer.clearScreen();

  if (view == View::Menu) {
    renderer.drawCenteredText(UI_12_FONT_ID, kTitleY, "SIDDUR", true, EpdFontFamily::BOLD);
    renderer.drawCenteredText(UI_10_FONT_ID, 90, localDateTimePreview.c_str());

    if (!hebrewDatePreview.empty()) {
      renderer.drawCenteredText(UI_10_FONT_ID, 115, hebrewDatePreview.c_str());
    }

    GUI.drawButtonMenu(
        renderer, Rect{0, 150, renderer.getScreenWidth(), 220}, 1, 0, [](int) { return std::string("Shaharit"); },
        [](int) { return UIIcon::Book; });

    const auto labels = mappedInput.mapLabels("Back", "Select", "", "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  } else {
    const auto drawRtlLine = [this](const int fontId, const int y, const char* text) {
      const int width = renderer.getTextWidth(fontId, text, EpdFontFamily::REGULAR, BidiUtils::BidiBaseDir::RTL);
      const int x = renderer.getScreenWidth() - kSideMargin - width;
      renderer.drawText(fontId, x, y, text, true, EpdFontFamily::REGULAR, BidiUtils::BidiBaseDir::RTL);
    };

    const auto* block = SiddurContent::EdotWeekdayShaharit::findBlock(composedPrayer[prayerIndex]);
    if (block == nullptr) {
      renderer.drawCenteredText(UI_12_FONT_ID, 250, "Missing prayer block");
      renderer.displayBuffer(cleanRefreshPending ? HalDisplay::HALF_REFRESH : HalDisplay::FAST_REFRESH);
      cleanRefreshPending = false;
      return;
    }

    drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, kTitleY, SiddurContent::EdotWeekdayShaharit::kShaharitTitle);
    drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, kSectionTitleY, block->sectionTitle);
    drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, kPrayerTitleY, block->title);

    const int maxWidth = renderer.getScreenWidth() - 2 * kSideMargin;
    const auto page = layoutTextPage(renderer, block->text, textOffset, maxWidth);
    nextTextOffset = page.nextOffset;
    hasNextTextPage = page.hasNext;

    const int lineHeight = renderer.getLineHeight(SIDDUR_HEBREW_16_FONT_ID);
    int y = kPrayerStartY;
    for (const auto& line : page.lines) {
      if (!line.empty()) drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, y, line.c_str());
      y += lineHeight + kLineGap;
    }

    char pageLabel[40];
    std::snprintf(pageLabel, sizeof(pageLabel), "%u/%u  p%u", static_cast<unsigned>(prayerIndex + 1),
                  static_cast<unsigned>(composedPrayer.size()), static_cast<unsigned>(textPageIndex + 1));
    renderer.drawCenteredText(UI_10_FONT_ID, kPageNumberY, pageLabel);

    const auto labels = mappedInput.mapLabels("Back", "", "Previous", "Next");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  }

  renderer.displayBuffer(cleanRefreshPending ? HalDisplay::HALF_REFRESH : HalDisplay::FAST_REFRESH);
  cleanRefreshPending = false;
}
