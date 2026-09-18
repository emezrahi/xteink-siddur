#include "SiddurActivity.h"

#include <Composer.h>
#include <HalClock.h>
#include <HalDisplay.h>
#include <HebrewCalendar.h>
#include <LocalClock.h>

#include <cstdio>
#include <string>

#include "CrossPointSettings.h"
#include "components/UITheme.h"
#include "content/EdotHamizrach.h"
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
}  // namespace

void SiddurActivity::onEnter() {
  Activity::onEnter();
  view = View::Menu;
  prayerIndex = 0;
  refreshCalendarPreview();
  cleanRefreshPending = true;
  requestUpdate();
}

void SiddurActivity::refreshCalendarPreview() {
  Rtc::DateTime utc;
  if (!halClock.getUtcDateTime(utc)) {
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

void SiddurActivity::openShaharit() {
  prayerContext = {};
  prayerContext.service = SiddurEngine::PrayerService::Shaharit;
  composedPrayer = SiddurEngine::Composer::compose(prayerContext);

  if (composedPrayer.empty()) {
    return;
  }

  view = View::Shaharit;
  prayerIndex = 0;
  cleanRefreshPending = true;
  requestUpdate();
}

void SiddurActivity::showPreviousPrayer() {
  if (prayerIndex == 0) return;
  --prayerIndex;
  requestUpdate();
}

void SiddurActivity::showNextPrayer() {
  if (prayerIndex + 1 >= composedPrayer.size()) return;
  ++prayerIndex;
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
    const auto drawRtlLine = [this](int fontId, int y, const char* text) {
      const int width = renderer.getTextWidth(fontId, text, EpdFontFamily::REGULAR, BidiUtils::BidiBaseDir::RTL);
      const int x = renderer.getScreenWidth() - kSideMargin - width;
      renderer.drawText(fontId, x, y, text, true, EpdFontFamily::REGULAR, BidiUtils::BidiBaseDir::RTL);
    };

    const auto* block = SiddurContent::EdotHamizrach::findBlock(composedPrayer[prayerIndex]);
    if (block == nullptr) {
      renderer.drawCenteredText(UI_12_FONT_ID, 250, "Missing prayer block");
      renderer.displayBuffer(cleanRefreshPending ? HalDisplay::HALF_REFRESH : HalDisplay::FAST_REFRESH);
      cleanRefreshPending = false;
      return;
    }

    drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, kTitleY, SiddurContent::EdotHamizrach::kShaharitTitle);
    drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, kSectionTitleY, block->sectionTitle);
    drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, kPrayerTitleY, block->title);

    const int maxWidth = renderer.getScreenWidth() - 2 * kSideMargin;
    const auto lines = renderer.wrappedText(SIDDUR_HEBREW_16_FONT_ID, block->text, maxWidth, kMaxPrayerLines);
    const int lineHeight = renderer.getLineHeight(SIDDUR_HEBREW_16_FONT_ID);

    int y = kPrayerStartY;
    for (const auto& line : lines) {
      drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, y, line.c_str());
      y += lineHeight + kLineGap;
    }

    const std::string pageLabel = std::to_string(prayerIndex + 1) + " / " + std::to_string(composedPrayer.size());
    renderer.drawCenteredText(UI_10_FONT_ID, kPageNumberY, pageLabel.c_str());

    const auto labels = mappedInput.mapLabels("Back", "", "Previous", "Next");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  }

  renderer.displayBuffer(cleanRefreshPending ? HalDisplay::HALF_REFRESH : HalDisplay::FAST_REFRESH);
  cleanRefreshPending = false;
}
