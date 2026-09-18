#pragma once

#include <PrayerBlockId.h>
#include <PrayerContext.h>

#include <cstddef>
#include <string>
#include <vector>

#include "activities/Activity.h"

class SiddurActivity final : public Activity {
  enum class View { Menu, Shaharit };

  View view = View::Menu;
  SiddurEngine::PrayerContext prayerContext;
  std::vector<SiddurEngine::PrayerBlockId> composedPrayer;
  std::size_t prayerIndex = 0;
  bool cleanRefreshPending = true;
  std::string localDateTimePreview;
  std::string hebrewDatePreview;

  void refreshCalendarPreview();
  void openShaharit();
  void showPreviousPrayer();
  void showNextPrayer();

 public:
  explicit SiddurActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Siddur", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
