#pragma once

#include <CivilDateTime.h>
#include <HebrewDate.h>
#include <PrayerBlockId.h>
#include <PrayerContext.h>

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "activities/Activity.h"

class SiddurActivity final : public Activity {
  enum class View { Menu, Chapters, Prayer };
  struct Chapter {
    const char* title = nullptr;
    std::size_t firstPrayerIndex = 0;
  };

  View view = View::Menu;
  SiddurEngine::PrayerContext prayerContext;
  std::vector<SiddurEngine::PrayerBlockId> composedPrayer;
  std::array<Chapter, 16> chapters{};
  std::size_t chapterCount = 0;
  std::size_t selectedChapter = 0;
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
  void buildChapters();
  void openSelectedChapter();
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
