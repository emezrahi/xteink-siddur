#include "SiddurActivity.h"

#include <Composer.h>
#include <HalClock.h>
#include <HalDisplay.h>
#include <HebrewCalendar.h>
#include <LocalClock.h>
#include <PrayerContextResolver.h>
#include <Zmanim.h>

#include <algorithm>
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
constexpr int kFooterReserve = 72;
constexpr int kChapterTop = 140;
constexpr int kChapterRowHeight = 54;

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

const char* serviceName(const SiddurEngine::PrayerService service) {
  switch (service) {
    case SiddurEngine::PrayerService::Shaharit:
      return "Shaharit";
    case SiddurEngine::PrayerService::Minha:
      return "Minha";
    case SiddurEngine::PrayerService::Arvit:
      return "Arvit";
    case SiddurEngine::PrayerService::Musaf:
      return "Mussaf";
  }
  return "";
}

// The body must stop above the page counter and the theme's button hints.
int pageNumberY(const GfxRenderer& renderer) {
  const Rect safe = UITheme::getInstance().getScreenSafeArea(renderer, true, false);
  return safe.y + safe.height - 24;
}

int prayerTextStartY(const SiddurContent::EdotWeekdayShaharit::PrayerTextBlock* block) {
  return block != nullptr && std::strcmp(block->sectionTitle, block->title) == 0 ? kPrayerStartY - 45
                                                                                  : kPrayerStartY;
}

int linesPerPage(const GfxRenderer& renderer,
                 const SiddurContent::EdotWeekdayShaharit::PrayerTextBlock* block) {
  const int lineHeight = renderer.getLineHeight(SIDDUR_HEBREW_16_FONT_ID);
  const int rowHeight = lineHeight + kLineGap;
  const Rect safe = UITheme::getInstance().getScreenSafeArea(renderer, true, false);
  const int bottom = safe.y + safe.height - kFooterReserve;
  const int available = bottom - prayerTextStartY(block) - lineHeight;
  return std::clamp(available / rowHeight + 1, 1, kMaxPrayerLines);
}

const char* chapterTitle(const SiddurEngine::PrayerBlockId id) {
  using SiddurEngine::PrayerBlockId;
  switch (id) {
    case PrayerBlockId::ModehAni:
      return "השכמת הבוקר";
    case PrayerBlockId::NetilatYadayim:
    case PrayerBlockId::AsherYatzar:
    case PrayerBlockId::ElohaiNeshama:
    case PrayerBlockId::MorningBlessings:
    case PrayerBlockId::TorahBlessings:
      return "ברכות השחר";
    case PrayerBlockId::PetichatEliyahu:
    case PrayerBlockId::Talit:
    case PrayerBlockId::Tefillin:
    case PrayerBlockId::HannasPrayer:
      return "הכנה לתפילה";
    case PrayerBlockId::MorningPrayer:
    case PrayerBlockId::IncenseOfferingMorning:
      return "קרבנות";
    case PrayerBlockId::Hodu:
    case PrayerBlockId::PesukeiDeZimra:
      return "פסוקי דזמרה";
    case PrayerBlockId::ShemaAndBlessings:
      return "קריאת שמע";
    case PrayerBlockId::WeekdayAmidah:
    case PrayerBlockId::MoridHatal:
    case PrayerBlockId::MashivHaruach:
    case PrayerBlockId::Barechenu:
    case PrayerBlockId::BarechAleinu:
    case PrayerBlockId::RetzehOpening:
    case PrayerBlockId::YaalehVeyavoRoshHodesh:
    case PrayerBlockId::YaalehVeyavoFestival:
    case PrayerBlockId::AlHanissimHanukkah:
    case PrayerBlockId::AlHanissimPurim:
    case PrayerBlockId::Aneinu:
    case PrayerBlockId::RetzehConclusion:
    case PrayerBlockId::Modim:
      return "עמידה";
    case PrayerBlockId::HallelHalf:
    case PrayerBlockId::HallelFull:
      return "הלל";
    case PrayerBlockId::ViduiRegular:
    case PrayerBlockId::ViduiMondayThursday:
      return "תחנון";
    case PrayerBlockId::TorahReadingWeekday:
      return "קריאת התורה";
    case PrayerBlockId::AshreiAfterTahanun:
    case PrayerBlockId::UvaLezionRegular:
    case PrayerBlockId::UvaLezionTorah:
    case PrayerBlockId::BeitYaakov:
      return "סיום שחרית";
    case PrayerBlockId::SongOfDaySunday:
    case PrayerBlockId::SongOfDayMonday:
    case PrayerBlockId::SongOfDayTuesday:
    case PrayerBlockId::SongOfDayWednesday:
    case PrayerBlockId::SongOfDayThursday:
    case PrayerBlockId::SongOfDayFriday:
    case PrayerBlockId::SongOfDaySaturday:
      return "שיר של יום";
    case PrayerBlockId::Kaveh:
    case PrayerBlockId::Aleinu:
      return "עלינו לשבח";
    case PrayerBlockId::MussafRoshHodesh:
    case PrayerBlockId::MussafFestival:
      return "מוסף";
  }
  return nullptr;
}

TextPage layoutTextPage(GfxRenderer& renderer, const char* text, const std::size_t startOffset, const int maxWidth,
                        const int maxLines) {
  TextPage page;
  page.lines.reserve(maxLines);
  if (text == nullptr || text[startOffset] == '\0') {
    page.nextOffset = startOffset;
    return page;
  }

  std::size_t pos = startOffset;
  while (text[pos] == ' ' || text[pos] == '\n') ++pos;

  std::string currentLine;
  currentLine.reserve(192);

  while (text[pos] != '\0' && static_cast<int>(page.lines.size()) < maxLines) {
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

      if (static_cast<int>(page.lines.size()) >= maxLines) {
        pos = wordStart;
        break;
      }
    }

    if (renderer.getTextWidth(SIDDUR_HEBREW_16_FONT_ID, token.c_str(), EpdFontFamily::REGULAR,
                              BidiUtils::BidiBaseDir::RTL) <= maxWidth) {
      currentLine = std::move(token);
    } else {
      page.lines.push_back(renderer.truncatedText(SIDDUR_HEBREW_16_FONT_ID, token.c_str(), maxWidth));
      if (static_cast<int>(page.lines.size()) >= maxLines) break;
    }
  }

  if (!currentLine.empty() && static_cast<int>(page.lines.size()) < maxLines) {
    page.lines.push_back(std::move(currentLine));
  }

  while (text[pos] == ' ' || text[pos] == '\n') ++pos;
  page.nextOffset = pos;
  page.hasNext = text[pos] != '\0';
  return page;
}

std::pair<std::size_t, std::size_t> lastPagePosition(GfxRenderer& renderer, const char* text, const int maxWidth,
                                                    const int maxLines) {
  std::size_t offset = 0;
  std::size_t pageIndex = 0;

  while (true) {
    const auto page = layoutTextPage(renderer, text, offset, maxWidth, maxLines);
    if (!page.hasNext || page.nextOffset <= offset) return {offset, pageIndex};
    offset = page.nextOffset;
    ++pageIndex;
  }
}

std::size_t previousPageOffset(GfxRenderer& renderer, const char* text, const std::size_t currentOffset,
                               const int maxWidth, const int maxLines) {
  if (currentOffset == 0) return 0;

  std::size_t offset = 0;
  while (true) {
    const auto page = layoutTextPage(renderer, text, offset, maxWidth, maxLines);
    if (!page.hasNext || page.nextOffset >= currentOffset || page.nextOffset <= offset) return offset;
    offset = page.nextOffset;
  }
}
}  // namespace

void SiddurActivity::onEnter() {
  Activity::onEnter();
  view = View::Menu;
  prayerIndex = 0;
  chapterCount = 0;
  selectedChapter = 0;
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

void SiddurActivity::resetTextPage() {
  textOffset = 0;
  nextTextOffset = 0;
  textPageIndex = 0;
  hasNextTextPage = false;
}

void SiddurActivity::openShaharit() {
  if (hasLocalCivilDate) {
    prayerContext = SiddurEngine::PrayerContextResolver::resolve(SiddurEngine::PrayerService::Shaharit, localCivilDate,
                                                                 afterSunset, SETTINGS.siddurDiaspora != 0)
                        .context;
  } else {
    prayerContext = {};
    prayerContext.service = SiddurEngine::PrayerService::Shaharit;
  }

  composedPrayer = SiddurEngine::Composer::compose(prayerContext);
  if (composedPrayer.empty()) return;

  buildChapters();
  if (chapterCount == 0) return;

  selectedChapter = 0;
  view = View::Chapters;
  cleanRefreshPending = true;
  requestUpdate();
}

void SiddurActivity::buildChapters() {
  chapterCount = 0;
  // Use only sections with text in today's composed prayer. This also omits
  // conditional sections that do not apply to the selected day.
  for (std::size_t i = 0; i < composedPrayer.size(); ++i) {
    if (SiddurContent::EdotWeekdayShaharit::findBlock(composedPrayer[i]) == nullptr) continue;
    const char* title = chapterTitle(composedPrayer[i]);
    if (title == nullptr) continue;

    bool present = false;
    for (std::size_t j = 0; j < chapterCount; ++j) {
      if (std::strcmp(chapters[j].title, title) == 0) {
        present = true;
        break;
      }
    }
    if (!present && chapterCount < chapters.size()) {
      chapters[chapterCount++] = {title, i};
    }
  }
}

void SiddurActivity::openSelectedChapter() {
  if (selectedChapter >= chapterCount) return;
  prayerIndex = chapters[selectedChapter].firstPrayerIndex;
  resetTextPage();
  view = View::Prayer;
  cleanRefreshPending = true;
  requestUpdate();
}

void SiddurActivity::showPreviousPrayer() {
  if (composedPrayer.empty()) return;

  const int maxWidth = renderer.getScreenWidth() - 2 * kSideMargin;
  const auto* block = SiddurContent::EdotWeekdayShaharit::findBlock(composedPrayer[prayerIndex]);

  if (block != nullptr && textOffset > 0) {
    textOffset = previousPageOffset(renderer, block->text, textOffset, maxWidth, linesPerPage(renderer, block));
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
    const auto last = lastPagePosition(renderer, block->text, maxWidth, linesPerPage(renderer, block));
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

  if (view == View::Chapters) {
    if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
      view = View::Menu;
      cleanRefreshPending = true;
      requestUpdate();
      return;
    }
    if ((mappedInput.wasReleased(MappedInputManager::Button::NavPrevious) ||
         mappedInput.wasReleased(MappedInputManager::Button::Up)) &&
        selectedChapter > 0) {
      --selectedChapter;
      requestUpdate();
      return;
    }
    if ((mappedInput.wasReleased(MappedInputManager::Button::NavNext) ||
         mappedInput.wasReleased(MappedInputManager::Button::Down)) &&
        selectedChapter + 1 < chapterCount) {
      ++selectedChapter;
      requestUpdate();
      return;
    }
    if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
      openSelectedChapter();
    }
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    // Return to the table of contents with the current section selected.
    for (std::size_t i = 0; i < chapterCount; ++i) {
      if (chapters[i].firstPrayerIndex <= prayerIndex) selectedChapter = i;
    }
    view = View::Chapters;
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

    if (view == View::Chapters) {
      drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, kTitleY, SiddurContent::EdotWeekdayShaharit::kShaharitTitle);
      renderer.drawCenteredText(UI_12_FONT_ID, 98, "CONTENTS", true, EpdFontFamily::BOLD);

      const Rect safe = UITheme::getInstance().getScreenSafeArea(renderer, true, false);
      const int available = pageNumberY(renderer) - kChapterTop - 38;
      const std::size_t visible = static_cast<std::size_t>(std::max(1, available / kChapterRowHeight));
      const std::size_t first = (selectedChapter / visible) * visible;
      const std::size_t last = std::min(chapterCount, first + visible);
      for (std::size_t i = first; i < last; ++i) {
        const int y = kChapterTop + static_cast<int>(i - first) * kChapterRowHeight;
        if (i == selectedChapter) {
          renderer.drawRect(kSideMargin, y - 5, renderer.getScreenWidth() - 2 * kSideMargin,
                            kChapterRowHeight - 3);
        }
        char index[12];
        std::snprintf(index, sizeof(index), "%u", static_cast<unsigned>(i + 1));
        renderer.drawText(UI_10_FONT_ID, kSideMargin + 12, y + 10, index);
        drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, y + 5, chapters[i].title);
      }

      char position[24];
      std::snprintf(position, sizeof(position), "%u/%u", static_cast<unsigned>(selectedChapter + 1),
                    static_cast<unsigned>(chapterCount));
      renderer.drawCenteredText(UI_10_FONT_ID, pageNumberY(renderer), position);
      const auto labels = mappedInput.mapLabels("Back", "Select", "Previous", "Next");
      GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
      renderer.displayBuffer(cleanRefreshPending ? HalDisplay::HALF_REFRESH : HalDisplay::FAST_REFRESH);
      cleanRefreshPending = false;
      return;
    }

    const auto* block = SiddurContent::EdotWeekdayShaharit::findBlock(composedPrayer[prayerIndex]);
    if (block == nullptr) {
      renderer.drawCenteredText(UI_12_FONT_ID, 250, "Missing prayer block");
      renderer.displayBuffer(cleanRefreshPending ? HalDisplay::HALF_REFRESH : HalDisplay::FAST_REFRESH);
      cleanRefreshPending = false;
      return;
    }

    drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, kTitleY, SiddurContent::EdotWeekdayShaharit::kShaharitTitle);
    drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, kSectionTitleY, block->sectionTitle);
    if (std::strcmp(block->sectionTitle, block->title) != 0) {
      drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, kPrayerTitleY, block->title);
    }

    const int maxWidth = renderer.getScreenWidth() - 2 * kSideMargin;
    const int startY = prayerTextStartY(block);
    const auto page = layoutTextPage(renderer, block->text, textOffset, maxWidth, linesPerPage(renderer, block));
    nextTextOffset = page.nextOffset;
    hasNextTextPage = page.hasNext;

    const int lineHeight = renderer.getLineHeight(SIDDUR_HEBREW_16_FONT_ID);
    int y = startY;
    for (const auto& line : page.lines) {
      if (!line.empty()) drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, y, line.c_str());
      y += lineHeight + kLineGap;
    }

    char pageLabel[40];
    std::snprintf(pageLabel, sizeof(pageLabel), "%u/%u  p%u", static_cast<unsigned>(prayerIndex + 1),
                  static_cast<unsigned>(composedPrayer.size()), static_cast<unsigned>(textPageIndex + 1));
    renderer.drawCenteredText(UI_10_FONT_ID, pageNumberY(renderer), pageLabel);

    const auto labels = mappedInput.mapLabels("Back", "", "Previous", "Next");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  }

  renderer.displayBuffer(cleanRefreshPending ? HalDisplay::HALF_REFRESH : HalDisplay::FAST_REFRESH);
  cleanRefreshPending = false;
}
