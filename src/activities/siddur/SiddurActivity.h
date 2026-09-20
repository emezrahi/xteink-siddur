#pragma once

#include <HebrewDate.h>
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
  std::size_t textOffset = 0;
  std::size_t nextTextOffset = 0;
  std::size_t textPageIndex = 0;
  bool hasNextTextPage = false;
  bool cleanRefreshPending = true;
  bool hasLocalCivilDate = false;
  SiddurEngine::CivilDate localCivilDate{1970, 1, 1};
  std::string localDateTimePreview;
  std::string hebrewDatePreview;

  void refreshCalendarPreview();
  void openShaharit();
  void resetTextPage();
  void showPreviousPrayer();
  void showNextPrayer();

 public:
  explicit SiddurActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Siddur", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
