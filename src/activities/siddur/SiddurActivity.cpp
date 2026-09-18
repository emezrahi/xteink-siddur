#include "SiddurActivity.h"

#include <HalDisplay.h>

#include <algorithm>

#include "components/UITheme.h"
#include "content/EdotHamizrach.h"
#include "fontIds.h"

namespace {
constexpr int kSideMargin = 24;
constexpr int kTitleY = 46;
constexpr int kSectionTitleY = 118;
constexpr int kPrayerStartY = 190;
constexpr int kLineGap = 10;
constexpr int kMaxPrayerLines = 8;
}  // namespace

void SiddurActivity::onEnter() {
  Activity::onEnter();
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
    const int width = renderer.getTextWidth(fontId, text, EpdFontFamily::REGULAR, BidiUtils::BidiBaseDir::RTL);
    const int x = renderer.getScreenWidth() - kSideMargin - width;
    renderer.drawText(fontId, x, y, text, true, EpdFontFamily::REGULAR, BidiUtils::BidiBaseDir::RTL);
  };

  drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, kSectionTitleY, SiddurContent::EdotHamizrach::kMorningBlessingsTitle);

  const int maxWidth = renderer.getScreenWidth() - 2 * kSideMargin;
  const auto lines = renderer.wrappedText(SIDDUR_HEBREW_16_FONT_ID, SiddurContent::EdotHamizrach::kNetilatYadayim,
                                          maxWidth, kMaxPrayerLines);
  const int lineHeight = renderer.getLineHeight(SIDDUR_HEBREW_16_FONT_ID);

  int y = kPrayerStartY;
  for (const auto& line : lines) {
    drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, y, line.c_str());
    y += lineHeight + kLineGap;
  }

  const auto labels = mappedInput.mapLabels("Back", "", "", "");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer(HalDisplay::FAST_REFRESH);
}
