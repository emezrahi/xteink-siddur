#pragma once

#include <cstddef>

#include "activities/Activity.h"

class SiddurActivity final : public Activity {
  enum class View { Menu, Shaharit };

  View view = View::Menu;
  std::size_t prayerIndex = 0;
  bool cleanRefreshPending = true;

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
