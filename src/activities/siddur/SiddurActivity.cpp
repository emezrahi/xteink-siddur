#include "SiddurActivity.h"

#include <Composer.h>
#include <HalDisplay.h>

#include <algorithm>
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
constexpr int kLinesPerPage = 11;
constexpr int kMaxWrappedPrayerLines = 256;
constexpr int kPageNumberY = 705;
constexpr std::size_t kContextMenuItemCount = 2;
}  // namespace

void SiddurActivity::onEnter() {
  Activity::onEnter();
  view = View::Menu;
  menuSelection = 0;
  prayerIndex = 0;
  blockPageIndex = 0;
  cleanRefreshPending = true;
  requestUpdate();
}

void SiddurActivity::openShaharit(const bool isRoshHodesh) {
  prayerContext = {};
  prayerContext.service = SiddurEngine::PrayerService::Shaharit;
  prayerContext.isRoshHodesh = isRoshHodesh;
  composedPrayer = SiddurEngine::Composer::compose(prayerContext);

  if (composedPrayer.empty()) {
    return;
  }

  view = View::Shaharit;
  prayerIndex = 0;
  blockPageIndex = 0;
  cleanRefreshPending = true;
  requestUpdate();
}

std::size_t SiddurActivity::getBlockPageCount(const std::size_t blockIndex) {
  if (blockIndex >= composedPrayer.size()) return 1;

  const auto* block = SiddurContent::EdotHamizrach::findBlock(composedPrayer[blockIndex]);
  if (block == nullptr) return 1;

  const int maxWidth = renderer.getScreenWidth() - 2 * kSideMargin;
  const auto lines = renderer.wrappedText(SIDDUR_HEBREW_16_FONT_ID, block->text, maxWidth, kMaxWrappedPrayerLines);
  return std::max<std::size_t>(1, (lines.size() + kLinesPerPage - 1) / kLinesPerPage);
}

void SiddurActivity::showPreviousPrayer() {
  if (blockPageIndex > 0) {
    --blockPageIndex;
    requestUpdate();
    return;
  }

  if (prayerIndex == 0) return;

  --prayerIndex;
  blockPageIndex = getBlockPageCount(prayerIndex) - 1;
  requestUpdate();
}

void SiddurActivity::showNextPrayer() {
  const std::size_t blockPageCount = getBlockPageCount(prayerIndex);
  if (blockPageIndex + 1 < blockPageCount) {
    ++blockPageIndex;
    requestUpdate();
    return;
  }

  if (prayerIndex + 1 >= composedPrayer.size()) return;

  ++prayerIndex;
  blockPageIndex = 0;
  requestUpdate();
}

void SiddurActivity::loop() {
  if (view == View::Menu) {
    if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
      finish();
      return;
    }

    if (mappedInput.wasReleased(MappedInputManager::Button::NavPrevious) && menuSelection > 0) {
      --menuSelection;
      requestUpdate();
      return;
    }

    if (mappedInput.wasReleased(MappedInputManager::Button::NavNext) && menuSelection + 1 < kContextMenuItemCount) {
      ++menuSelection;
      requestUpdate();
      return;
    }

    if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
      openShaharit(menuSelection == 1);
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
    renderer.drawCenteredText(UI_12_FONT_ID, kTitleY, "SIDDUR - TEST CONTEXT", true, EpdFontFamily::BOLD);

    GUI.drawButtonMenu(
        renderer, Rect{0, 130, renderer.getScreenWidth(), 260}, kContextMenuItemCount, menuSelection,
        [](int index) {
          return index == 0 ? std::string("Shaharit - Weekday") : std::string("Shaharit - Rosh Hodesh");
        },
        [](int) { return UIIcon::Book; });

    const auto labels = mappedInput.mapLabels("Back", "Select", "Previous", "Next");
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
    const auto lines =
        renderer.wrappedText(SIDDUR_HEBREW_16_FONT_ID, block->text, maxWidth, kMaxWrappedPrayerLines);
    const int lineHeight = renderer.getLineHeight(SIDDUR_HEBREW_16_FONT_ID);
    const std::size_t startLine = blockPageIndex * kLinesPerPage;
    const std::size_t endLine = std::min(lines.size(), startLine + kLinesPerPage);

    int y = kPrayerStartY;
    for (std::size_t i = startLine; i < endLine; ++i) {
      drawRtlLine(SIDDUR_HEBREW_16_FONT_ID, y, lines[i].c_str());
      y += lineHeight + kLineGap;
    }

    const std::size_t blockPageCount = getBlockPageCount(prayerIndex);
    std::string pageLabel = std::to_string(prayerIndex + 1) + " / " + std::to_string(composedPrayer.size());
    if (blockPageCount > 1) {
      pageLabel += "   page " + std::to_string(blockPageIndex + 1) + " / " + std::to_string(blockPageCount);
    }
    renderer.drawCenteredText(UI_10_FONT_ID, kPageNumberY, pageLabel.c_str());

    const auto labels = mappedInput.mapLabels("Back", "", "Previous", "Next");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  }

  renderer.displayBuffer(cleanRefreshPending ? HalDisplay::HALF_REFRESH : HalDisplay::FAST_REFRESH);
  cleanRefreshPending = false;
}
