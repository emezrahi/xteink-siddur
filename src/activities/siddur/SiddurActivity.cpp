#include "SiddurActivity.h"

#include <HalDisplay.h>

#include "CrossPointSettings.h"
#include "SdCardFontSystem.h"
#include "components/UITheme.h"
#include "fontIds.h"

namespace {
constexpr int kSideMargin = 24;
constexpr int kTitleY = 46;
constexpr int kPrayerStartY = 160;
constexpr int kPrayerLineGap = 18;
}  // namespace

void SiddurActivity::onEnter() {
  Activity::onEnter();

  // The selected SD-card reader font is the path to full Hebrew coverage,
  // including niqqud. Built-in UI fonts intentionally contain only the basic
  // Hebrew alphabet.
  sdFontSystem.ensureLoaded(renderer);
  prayerFontId = SETTINGS.getReaderFontId();

  const auto& fonts = renderer.getFontMap();
  const auto it = fonts.find(prayerFontId);
  pointedHebrewAvailable =
      it != fonts.end() && it->second.hasCodepoint(0x05D0) && it->second.hasCodepoint(0x05B0);

  if (!pointedHebrewAvailable) {
    prayerFontId = UI_12_FONT_ID;
  }

  requestUpdate();
}

void SiddurActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
  }
}

void SiddurActivity::render(RenderLock&&) {
  renderer.clearScreen();

  renderer.drawCenteredText(UI_12_FONT_ID, kTitleY, "SIDDUR", true, EpdFontFamily::BOLD);

  const auto drawRtlLine = [this](int fontId, int y, const char* text) {
    const int width =
        renderer.getTextWidth(fontId, text, EpdFontFamily::REGULAR, BidiUtils::BidiBaseDir::RTL);
    const int x = renderer.getScreenWidth() - kSideMargin - width;
    renderer.drawText(fontId, x, y, text, true, EpdFontFamily::REGULAR, BidiUtils::BidiBaseDir::RTL);
  };

  if (pointedHebrewAvailable) {
    drawRtlLine(prayerFontId, kPrayerStartY, "שְׁמַע יִשְׂרָאֵל");
    drawRtlLine(prayerFontId, kPrayerStartY + renderer.getLineHeight(prayerFontId) + kPrayerLineGap,
                "ה׳ אֱלֹהֵינוּ ה׳ אֶחָד");
  } else {
    // Basic Hebrew still lets us verify the activity + BiDi path immediately.
    // Pointed text will be enabled as soon as a Hebrew-capable SD reader font
    // such as DavidLibre or FrankRuhlLibre is selected.
    drawRtlLine(UI_12_FONT_ID, kPrayerStartY, "שמע ישראל");

    renderer.drawCenteredText(UI_10_FONT_ID, 270, "Pointed Hebrew font not selected.");
    renderer.drawCenteredText(UI_10_FONT_ID, 305, "Select DavidLibre or FrankRuhlLibre");
    renderer.drawCenteredText(UI_10_FONT_ID, 340, "in Settings > Reader > Font Family.");
  }

  const auto labels = mappedInput.mapLabels("Back", "", "", "");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer(HalDisplay::FAST_REFRESH);
}
