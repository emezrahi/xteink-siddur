#include "SiddurActivity.h"

#include <Composer.h>
#include <HalClock.h>
#include <HalDisplay.h>
#include <HebrewCalendar.h>
#include <LocalClock.h>
#include <PrayerContextResolver.h>
#include <Zmanim.h>

#include <array>
#include <cstdio>
#include <cstring>
#include <span>
#include <string>
#include <string_view>
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
constexpr int kMenuTop = 150;

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
  menuState = SiddurMenu::State(menuRowCapacity());
  prayerIndex = 0;
  resetTextPage();
  refreshCalendarPreview();
  cleanRefreshPending = true;
  requestUpdate();
}

std::size_t SiddurActivity::menuRowCapacity() const {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int available = renderer.getScreenHeight() - kMenuTop - metrics.buttonHintsHeight - metrics.verticalSpacing;
  const int step = metrics.menuRowHeight + metrics.menuSpacing;
  if (available <= 0 || step <= 0) return 0;
  return std::min(kMaximumMenuRows, static_cast<std::size_t>(available / step));
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
  if (composedPrayer.empty()) {
    menuState.back();
    return;
  }

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
  if (menuState.screen() != SiddurMenu::Screen::Reading) {
    if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
      if (!menuState.back()) finish();
      cleanRefreshPending = true;
      requestUpdate();
      return;
    }

    if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
      menuState.choose();
      if (menuState.screen() == SiddurMenu::Screen::Reading) openShaharit();
      cleanRefreshPending = true;
      requestUpdate();
      return;
    }
    if (mappedInput.wasReleased(MappedInputManager::Button::NavPrevious)) {
      menuState.previous();
      requestUpdate();
      return;
    }
    if (mappedInput.wasReleased(MappedInputManager::Button::NavNext)) {
      menuState.next();
      requestUpdate();
      return;
    }
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    menuState.back();
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

  if (menuState.screen() != SiddurMenu::Screen::Reading) {
    GUI.drawHeader(renderer, Rect{0, 0, renderer.getScreenWidth(), 50}, tr(STR_SIDDUR));
    renderer.drawCenteredText(UI_10_FONT_ID, 90, localDateTimePreview.c_str());

    if (!hebrewDatePreview.empty()) {
      renderer.drawCenteredText(UI_10_FONT_ID, 115, hebrewDatePreview.c_str());
    }

    std::array<std::string_view, kMaximumMenuRows> rows{};
    std::size_t rowCount = 0;
    std::size_t selectedOffset = 0;
    if (menuState.screen() == SiddurMenu::Screen::Navigation) {
      const auto& navigation = menuState.menu();
      const auto page = navigation.selection().page();
      const auto selected = navigation.selection().selected();
      if (page && selected) {
        selectedOffset = *selected - page->first;
        if (navigation.level() == SiddurStaticNavigation::NavigationSelection::Level::Categories) {
          for (std::size_t i = page->first; i < page->pastLast; ++i) {
            rows[rowCount++] = SiddurStaticNavigation::categories()[i].entry.hebrew;
          }
        } else {
          const auto items = SiddurStaticNavigation::categories()[navigation.categoryIndex()].items();
          for (std::size_t i = page->first; i < page->pastLast; ++i) rows[rowCount++] = items[i].hebrew;
        }
      }
    } else if (menuState.screen() == SiddurMenu::Screen::Nusach) {
      const auto page = menuState.nusachMenu().page();
      const auto selected = menuState.nusachMenu().selected();
      if (page && selected) {
        selectedOffset = *selected - page->first;
        for (std::size_t i = page->first; i < page->pastLast; ++i) {
          rows[rowCount++] = SiddurStaticNavigation::nusachChoices()[i].hebrew;
        }
      }
    } else {
      renderer.drawCenteredText(UI_12_FONT_ID, 230, tr(STR_SIDDUR_PREVIEW_ONLY), true, EpdFontFamily::BOLD);
      renderer.drawCenteredText(UI_10_FONT_ID, 280, tr(STR_SIDDUR_NOT_AVAILABLE));
      const auto preview = menuState.previewEntry();
      if (preview) rows[rowCount++] = preview.value()->hebrew;
    }
    if (rowCount > 0) {
      GUI.drawRtlButtonMenu(
          renderer, Rect{0, kMenuTop, renderer.getScreenWidth(), renderer.getScreenHeight() - kMenuTop},
          std::span<const std::string_view>(rows.data(), rowCount), static_cast<int>(selectedOffset),
          SIDDUR_HEBREW_16_FONT_ID);
    }

    char pageLabel[24] = {};
    const auto page = menuState.screen() == SiddurMenu::Screen::Nusach ? menuState.nusachMenu().page()
                                                                       : menuState.menu().selection().page();
    if (page) std::snprintf(pageLabel, sizeof(pageLabel), "%u-%u", static_cast<unsigned>(page->first + 1),
                            static_cast<unsigned>(page->pastLast));
    renderer.drawCenteredText(UI_10_FONT_ID,
                              renderer.getScreenHeight() - UITheme::getInstance().getMetrics().buttonHintsHeight - 20,
                              pageLabel);
    const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
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
