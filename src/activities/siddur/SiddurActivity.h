#pragma once

#include "activities/Activity.h"

class SiddurActivity final : public Activity {
 public:
  explicit SiddurActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Siddur", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
