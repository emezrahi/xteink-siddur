#include "SiddurActivity.h"

#include <HalDisplay.h>

#include <string>

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
}  // namespace

void SiddurActivity::onEnter() {
  Activity::onEnter();
  view = View::Menu;
  prayerIndex = 0;
  cleanRefreshPending = true;
  requestUpdate();
}

void SiddurActivity::openShaharit() {
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
  if (prayerIndex + 1 >= SiddurContent::EdotHamizrach::kShaharitMorningBlessingsCount) return;
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

    GUI.drawButtonMenu(renderer, Rect{0, 130, renderer.getScreenWidth(), 220}, 1, 0,
                       [](int) { return std::string("Shaharit"); }, [](int) { return UIIcon::Book; });

    const auto labels = mappedInput.mapLabels("Back", "Select", "", "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  } else {
    const auto drawRtlLine = [this](int fontId, int y, const char* text) {
      const int width = renderer.getTextWidth(fontId, text, EpdFontFamily::REGULAR, BidiUtils::BidiBaseDir::RTL);
      const int x = renderer.getScreenWidth() - kSideMargin - width;
      renderer.drawText(fontId, x, y, text, true, EpdFontFamily::REGULAR, BidiUtils::BidiBaseDir::RTL);
    };

    const auto& block = SiddurContent::EdotHamizrach::kShaharitMorningBlessings[prayerIndex];

    drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, kTitleY, SiddurContent::EdotHamizrach::kShaharitTitle);
    drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, kSectionTitleY, SiddurContent::EdotHamizrach::kMorningBlessingsTitle);
    drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, kPrayerTitleY, block.title);

    const int maxWidth = renderer.getScreenWidth() - 2 * kSideMargin;
    const auto lines = renderer.wrappedText(SIDDUR_HEBREW_16_FONT_ID, block.text, maxWidth, kMaxPrayerLines);
    const int lineHeight = renderer.getLineHeight(SIDDUR_HEBREW_16_FONT_ID);

    int y = kPrayerStartY;
    for (const auto& line : lines) {
      drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, y, line.c_str());
      y += lineHeight + kLineGap;
    }

    const std::string pageLabel = std::to_string(prayerIndex + 1) + " / " +
                                  std::to_string(SiddurContent::EdotHamizrach::kShaharitMorningBlessingsCount);
    renderer.drawCenteredText(UI_10_FONT_ID, kPageNumberY, pageLabel.c_str());

    const auto labels = mappedInput.mapLabels("Back", "", "Previous", "Next");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  }

  renderer.displayBuffer(cleanRefreshPending ? HalDisplay::HALF_REFRESH : HalDisplay::FAST_REFRESH);
  cleanRefreshPending = false;
}
