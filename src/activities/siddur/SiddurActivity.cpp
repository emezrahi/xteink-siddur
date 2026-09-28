#include "SiddurActivity.h"

#include <Composer.h>
#include <HalClock.h>
#include <HalDisplay.h>
#include <HebrewCalendar.h>
#include <LocalClock.h>
#include <PrayerContextResolver.h>
#include <Utf8Pager.h>
#include <Zmanim.h>

#include <algorithm>
#include <cstdio>
#include <string>
#include <utility>

#include "CrossPointSettings.h"
#include "components/UITheme.h"
#include "content/EdotChapters.h"
#include "fontIds.h"

namespace {
constexpr int kSideMargin = 24;
constexpr int kHeaderTop = 15;
constexpr int kEnglishTitleTop = 49;
constexpr int kHebrewTitleTop = 86;
constexpr int kBodyTop = 138;
constexpr int kContinuationBodyTop = 91; // Reclaim the Hebrew title band on continuation pages.
constexpr int kReaderGap = 7;
constexpr int kMenuTop = 122;
constexpr int kMenuRowGap = 6;

struct ReaderGeometry {
  int bodyTop;
  int pageLabelTop;
  int lineAdvance;
  int lineCount;
  int textWidth;
};

ReaderGeometry readerGeometry(GfxRenderer& renderer, const bool firstPage) {
  const auto safe = UITheme::getInstance().getScreenSafeArea(renderer, true, false);
  const int labelTop = safe.y + safe.height - renderer.getTextHeight(UI_10_FONT_ID) - 16;
  const int bodyTop = safe.y + (firstPage ? kBodyTop : kContinuationBodyTop);
  const int glyphHeight = renderer.getTextHeight(SIDDUR_HEBREW_16_FONT_ID);
  const int advance = std::max(renderer.getLineHeight(SIDDUR_HEBREW_16_FONT_ID), glyphHeight) + kReaderGap;
  const int bodyBottom = labelTop - 14;
  return {bodyTop, labelTop, advance,
          SiddurEngine::PageGeometry::visibleLines(bodyTop, bodyBottom, glyphHeight, advance),
          renderer.getScreenWidth() - kSideMargin * 2};
}

const char* hebrewMonthName(const SiddurEngine::HebrewMonth month) {
  switch (month) {
    case SiddurEngine::HebrewMonth::Nisan: return "Nisan";
    case SiddurEngine::HebrewMonth::Iyar: return "Iyar";
    case SiddurEngine::HebrewMonth::Sivan: return "Sivan";
    case SiddurEngine::HebrewMonth::Tammuz: return "Tammuz";
    case SiddurEngine::HebrewMonth::Av: return "Av";
    case SiddurEngine::HebrewMonth::Elul: return "Elul";
    case SiddurEngine::HebrewMonth::Tishrei: return "Tishrei";
    case SiddurEngine::HebrewMonth::Cheshvan: return "Cheshvan";
    case SiddurEngine::HebrewMonth::Kislev: return "Kislev";
    case SiddurEngine::HebrewMonth::Tevet: return "Tevet";
    case SiddurEngine::HebrewMonth::Shevat: return "Shevat";
    case SiddurEngine::HebrewMonth::Adar: return "Adar";
    case SiddurEngine::HebrewMonth::AdarII: return "Adar II";
  }
  return "";
}

const char* serviceName(const SiddurEngine::PrayerService service) {
  switch (service) {
    case SiddurEngine::PrayerService::Shaharit: return "Shaharit";
    case SiddurEngine::PrayerService::Minha: return "Minha";
    case SiddurEngine::PrayerService::Arvit: return "Arvit";
    case SiddurEngine::PrayerService::Musaf: return "Mussaf";
  }
  return "";
}
} // namespace

void SiddurActivity::onEnter() {
  Activity::onEnter();
  view = View::Chapters;
  chapterIndex = 0;
  resetTextPage();
  refreshCalendarPreview();
  refreshChapters();
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
      static_cast<int>(utc.hour), static_cast<int>(utc.minute)};
  localDateTime = SiddurEngine::LocalClock::applyUtcOffset(utcDateTime, SETTINGS.clockUtcOffsetQ);
  localCivilDate = localDateTime.date;
  hasLocalCivilDate = true;
  SiddurEngine::LocationConfig location;
  location.latitude = static_cast<double>(SETTINGS.siddurLatitudeE6) / 1000000.0;
  location.longitude = static_cast<double>(SETTINGS.siddurLongitudeE6) / 1000000.0;
  location.utcOffsetMinutes = (static_cast<int>(SETTINGS.clockUtcOffsetQ) - 48) * 15;
  location.diaspora = SETTINGS.siddurDiaspora != 0;
  const auto resolved = SiddurEngine::PrayerContextResolver::resolve(localDateTime, location);
  const auto hebrewDate = resolved.hebrewDate;
  afterSunset = SiddurEngine::Zmanim::isAfterSunset(localDateTime, location);

  char localBuffer[32];
  std::snprintf(localBuffer, sizeof(localBuffer), "%04d-%02d-%02d  %02d:%02d", localDateTime.date.year,
                localDateTime.date.month, localDateTime.date.day, localDateTime.hour, localDateTime.minute);
  localDateTimePreview = localBuffer;

  char hebrewBuffer[48];
  std::snprintf(hebrewBuffer, sizeof(hebrewBuffer), "%d %s %d | %s", hebrewDate.day, hebrewMonthName(hebrewDate.month),
                hebrewDate.year, serviceName(resolved.context.service));
  hebrewDatePreview = hebrewBuffer;
}

void SiddurActivity::refreshChapters() {
  if (hasLocalCivilDate) {
    prayerContext = SiddurEngine::PrayerContextResolver::resolve(SiddurEngine::PrayerService::Shaharit, localCivilDate,
                                                                 afterSunset, SETTINGS.siddurDiaspora != 0)
                        .context;
  } else {
    prayerContext = {};
    prayerContext.service = SiddurEngine::PrayerService::Shaharit;
  }
  // Do not offer liturgical placeholders. Chapters are populated ONLY from
  // actual compiled CC0 text blocks that the date-aware composer selected.
  chapters = SiddurContent::weekdayChapters(SiddurEngine::Composer::compose(prayerContext));
  if (chapterIndex >= chapters.size()) chapterIndex = 0;
}

void SiddurActivity::resetTextPage() {
  textOffset = 0;
  nextTextOffset = 0;
  textPageIndex = 0;
  hasNextTextPage = false;
}

void SiddurActivity::openSelectedChapter() {
  if (chapters.empty() || chapterIndex >= chapters.size()) return;
  view = View::Reading;
  resetTextPage();
  cleanRefreshPending = true;
  requestUpdate();
}

void SiddurActivity::showPreviousPage() {
  if (chapters.empty()) return;
  // A chapter's first page includes the Hebrew title; continuation pages
  // have a larger text region. Previous/last navigation must replay both.
  const auto firstGeo = readerGeometry(renderer, true);
  const auto nextGeo = readerGeometry(renderer, false);
  auto measure = [this](const char* text) {
    return renderer.getTextWidth(SIDDUR_HEBREW_16_FONT_ID, text, EpdFontFamily::REGULAR,
                                 BidiUtils::BidiBaseDir::RTL);
  };

  if (textOffset > 0) {
    textOffset = SiddurEngine::Utf8Pager::previousOffsetVariable(chapters[chapterIndex].text, textOffset,
                                                                  firstGeo.textWidth, firstGeo.lineCount,
                                                                  nextGeo.lineCount, measure);
    if (textPageIndex > 0) --textPageIndex;
  } else if (chapterIndex > 0) {
    --chapterIndex;
    const auto last = SiddurEngine::Utf8Pager::lastPageVariable(chapters[chapterIndex].text, firstGeo.textWidth,
                                                                 firstGeo.lineCount, nextGeo.lineCount, measure);
    resetTextPage();
    textOffset = last.first;
    textPageIndex = last.second;
  }
  requestUpdate();
}

void SiddurActivity::showNextPage() {
  if (chapters.empty()) return;
  if (hasNextTextPage && nextTextOffset > textOffset) {
    textOffset = nextTextOffset;
    ++textPageIndex;
  } else if (chapterIndex + 1 < chapters.size()) {
    ++chapterIndex;
    resetTextPage();
  }
  requestUpdate();
}

void SiddurActivity::loop() {
  if (view == View::Chapters) {
    if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
      finish();
      return;
    }
    if (chapters.empty()) return;
    if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
      openSelectedChapter();
      return;
    }
    if (mappedInput.wasReleased(MappedInputManager::Button::NavPrevious)) {
      if (chapterIndex > 0) --chapterIndex;
      requestUpdate();
      return;
    }
    if (mappedInput.wasReleased(MappedInputManager::Button::NavNext)) {
      if (chapterIndex + 1 < chapters.size()) ++chapterIndex;
      requestUpdate();
      return;
    }
    int tapX = 0;
    int tapY = 0;
    if (mappedInput.wasScreenTapped(tapX, tapY)) {
      const auto safe = UITheme::getInstance().getScreenSafeArea(renderer, true, false);
      const int rowHeight = std::max(66, renderer.getLineHeight(UI_10_FONT_ID) * 2 + 12);
      const int pitch = rowHeight + kMenuRowGap;
      const int firstY = safe.y + kMenuTop;
      const int visible = std::max(1, (safe.y + safe.height - 46 - firstY) / pitch);
      const auto first = (chapterIndex / static_cast<std::size_t>(visible)) * static_cast<std::size_t>(visible);
      if (tapX >= kSideMargin && tapX < renderer.getScreenWidth() - kSideMargin && tapY >= firstY) {
        const int row = (tapY - firstY) / pitch;
        const std::size_t index = first + static_cast<std::size_t>(row);
        if (row >= 0 && row < visible && index < chapters.size() && (tapY - firstY) % pitch < rowHeight) {
          chapterIndex = index;
          openSelectedChapter();
        }
      }
    }
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Back) ||
      mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    view = View::Chapters;
    cleanRefreshPending = true;
    requestUpdate();
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::NavPrevious)) {
    showPreviousPage();
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::NavNext)) showNextPage();
}

void SiddurActivity::render(RenderLock&&) {
  renderer.clearScreen();
  const auto safe = UITheme::getInstance().getScreenSafeArea(renderer, true, false);
  const int screenWidth = renderer.getScreenWidth();

  if (view == View::Chapters) {
    renderer.drawCenteredText(UI_12_FONT_ID, safe.y + kHeaderTop, "SHAHARIT - CHAPTERS", true, EpdFontFamily::BOLD);
    renderer.drawCenteredText(UI_10_FONT_ID, safe.y + 54, localDateTimePreview.c_str());
    if (!hebrewDatePreview.empty()) {
      renderer.drawCenteredText(UI_10_FONT_ID, safe.y + 80, hebrewDatePreview.c_str());
    }

    if (chapters.empty()) {
      renderer.drawCenteredText(UI_12_FONT_ID, safe.y + 210, "No weekday chapters for this date");
    } else {
      const int rowHeight = std::max(66, renderer.getLineHeight(UI_10_FONT_ID) * 2 + 12);
      const int rowPitch = rowHeight + kMenuRowGap;
      const int firstY = safe.y + kMenuTop;
      const int visible = std::max(1, (safe.y + safe.height - 46 - firstY) / rowPitch);
      const std::size_t first = (chapterIndex / static_cast<std::size_t>(visible)) * static_cast<std::size_t>(visible);
      for (int row = 0; row < visible && first + static_cast<std::size_t>(row) < chapters.size(); ++row) {
        const std::size_t index = first + static_cast<std::size_t>(row);
        const int y = firstY + rowPitch * row;
        const bool selected = index == chapterIndex;
        const int width = screenWidth - kSideMargin * 2;
        renderer.fillRect(kSideMargin, y, width, rowHeight, selected);
        renderer.drawRect(kSideMargin, y, width, rowHeight);
        const auto lines = renderer.wrappedText(UI_10_FONT_ID, chapters[index].englishTitle, width - 28, 2);
        const int lineAdvance = renderer.getLineHeight(UI_10_FONT_ID);
        int textY = y + (rowHeight - static_cast<int>(lines.size()) * lineAdvance) / 2;
        for (const auto& line : lines) {
          renderer.drawText(UI_10_FONT_ID, kSideMargin + 14, textY, line.c_str(), !selected);
          textY += lineAdvance;
        }
      }
      char position[32];
      std::snprintf(position, sizeof(position), "%u / %u", static_cast<unsigned>(chapterIndex + 1),
                    static_cast<unsigned>(chapters.size()));
      renderer.drawCenteredText(UI_10_FONT_ID, safe.y + safe.height - 33, position);
    }
    const auto labels = mappedInput.mapLabels("Back", "Open", "Up", "Down");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  } else if (!chapters.empty()) {
    const auto& chapter = chapters[chapterIndex];
    auto drawRtl = [this, screenWidth](const int y, const char* text) {
      const int width = renderer.getTextWidth(SIDDUR_HEBREW_16_FONT_ID, text, EpdFontFamily::REGULAR,
                                              BidiUtils::BidiBaseDir::RTL);
      renderer.drawText(SIDDUR_HEBREW_16_FONT_ID, screenWidth - kSideMargin - width, y, text, true,
                        EpdFontFamily::REGULAR, BidiUtils::BidiBaseDir::RTL);
    };
    renderer.drawCenteredText(UI_10_FONT_ID, safe.y + kHeaderTop, "EDOT HAMIZRACH - SHAHARIT");
    const std::string english = renderer.truncatedText(UI_12_FONT_ID, chapter.englishTitle,
                                                       screenWidth - kSideMargin * 2, EpdFontFamily::BOLD);
    renderer.drawCenteredText(UI_12_FONT_ID, safe.y + kEnglishTitleTop, english.c_str(), true, EpdFontFamily::BOLD);
    // The Hebrew heading is shown once when opening a chapter, not on every page.
    if (textPageIndex == 0) drawRtl(safe.y + kHebrewTitleTop, chapter.hebrewTitle);

    const auto geo = readerGeometry(renderer, textPageIndex == 0);
    const auto measure = [this](const char* text) {
      return renderer.getTextWidth(SIDDUR_HEBREW_16_FONT_ID, text, EpdFontFamily::REGULAR,
                                   BidiUtils::BidiBaseDir::RTL);
    };
    const auto page = SiddurEngine::Utf8Pager::paginate(chapter.text, textOffset, geo.textWidth, geo.lineCount,
                                                       measure);
    nextTextOffset = page.nextOffset;
    hasNextTextPage = page.hasNext;
    int y = geo.bodyTop;
    for (const auto& line : page.lines) {
      if (!line.empty()) drawRtl(y, line.c_str());
      y += geo.lineAdvance;
    }
    char pageLabel[48];
    std::snprintf(pageLabel, sizeof(pageLabel), "%u / %u  page %u", static_cast<unsigned>(chapterIndex + 1),
                  static_cast<unsigned>(chapters.size()), static_cast<unsigned>(textPageIndex + 1));
    renderer.drawCenteredText(UI_10_FONT_ID, geo.pageLabelTop, pageLabel);
    const auto labels = mappedInput.mapLabels("Back", "Chapters", "Previous", "Next");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  }

  renderer.displayBuffer(cleanRefreshPending ? HalDisplay::HALF_REFRESH : HalDisplay::FAST_REFRESH);
  cleanRefreshPending = false;
}
