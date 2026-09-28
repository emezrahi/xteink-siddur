#pragma once

#include <Chapter.h>
#include <CivilDateTime.h>
#include <PrayerBlockId.h>
#include <PrayerContext.h>

#include <cstddef>
#include <string>
#include <vector>

#include "activities/Activity.h"

class SiddurActivity final : public Activity {
  enum class View { Chapters, Reading };

  View view = View::Chapters;
  SiddurEngine::PrayerContext prayerContext;
  std::vector<SiddurContent::Chapter> chapters;
  std::size_t chapterIndex = 0;
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
  void refreshChapters();
  void openSelectedChapter();
  void resetTextPage();
  void showPreviousPage();
  void showNextPage();

 public:
  explicit SiddurActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Siddur", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
