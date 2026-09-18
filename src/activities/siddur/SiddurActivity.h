#pragma once

#include <PrayerBlockId.h>
#include <PrayerContext.h>

#include <cstddef>
#include <vector>

#include "activities/Activity.h"

class SiddurActivity final : public Activity {
  enum class View { Menu, Shaharit };

  View view = View::Menu;
  SiddurEngine::PrayerContext prayerContext;
  std::vector<SiddurEngine::PrayerBlockId> composedPrayer;
  std::size_t menuSelection = 0;
  std::size_t prayerIndex = 0;
  std::size_t blockPageIndex = 0;
  bool cleanRefreshPending = true;

  void openShaharit(bool isRoshHodesh);
  void showPreviousPrayer();
  void showNextPrayer();
  std::size_t getBlockPageCount(std::size_t blockIndex);

 public:
  explicit SiddurActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Siddur", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
