#pragma once

#include <CivilDateTime.h>
#include <HebrewDate.h>
#include <PrayerBlockId.h>
#include <PrayerContext.h>
#include <SiddurMenuState.h>

#include <cstddef>
#include <string>
#include <vector>

#include "activities/Activity.h"

class SiddurActivity final : public Activity {
  static constexpr std::size_t kMaximumMenuRows = 10;
  SiddurMenu::State menuState{1};
  SiddurEngine::PrayerContext prayerContext;
  std::vector<SiddurEngine::PrayerBlockId> composedPrayer;
  std::size_t prayerIndex = 0;
  std::size_t textOffset = 0;
  std::size_t nextTextOffset = 0;
  std::size_t textPageIndex = 0;
  bool hasNextTextPage = false;
  bool cleanRefreshPending = true;
  bool hasLocalCivilDate = false;
  SiddurEngine::CivilDate localCivilDate{1970, 1, 1};
  SiddurEngine::CivilDateTime localDateTime{{1970, 1, 1}, 0, 0};
  bool afterSunset = false;
  std::string localDateTimePreview;
  std::string hebrewDatePreview;

  void refreshCalendarPreview();
  void openShaharit();
  void resetTextPage();
  void showPreviousPrayer();
  void showNextPrayer();
  [[nodiscard]] std::size_t menuRowCapacity() const;

 public:
  explicit SiddurActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Siddur", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
