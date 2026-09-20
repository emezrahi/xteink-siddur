#pragma once

#include <cstddef>

#include "PrayerContext.h"
#include "PrayerGeneratorId.h"

namespace SiddurEngine {

struct PrayerGeneratorPlan {
  const PrayerGeneratorId* generators;
  std::size_t size;

  [[nodiscard]] constexpr const PrayerGeneratorId* begin() const { return generators; }
  [[nodiscard]] constexpr const PrayerGeneratorId* end() const { return generators + size; }
  [[nodiscard]] constexpr const PrayerGeneratorId& operator[](const std::size_t index) const {
    return generators[index];
  }
};

class ServiceGeneratorPlan final {
 public:
  [[nodiscard]] static constexpr PrayerGeneratorPlan forService(const PrayerService service) {
    switch (service) {
      case PrayerService::Shaharit:
        return {kShaharit, sizeof(kShaharit) / sizeof(kShaharit[0])};
      case PrayerService::Minha:
        return {kMinha, sizeof(kMinha) / sizeof(kMinha[0])};
      case PrayerService::Arvit:
        return {kArvit, sizeof(kArvit) / sizeof(kArvit[0])};
      case PrayerService::Musaf:
        return {kMusaf, sizeof(kMusaf) / sizeof(kMusaf[0])};
    }

    return {nullptr, 0};
  }

 private:
  // Verified from ShacharitGenerator construction order in the reference APK.
  // Hallel, Torah reading, and Mussaf are conditional stages inside Shaharit.
  static inline constexpr PrayerGeneratorId kShaharit[] = {
      PrayerGeneratorId::ShaharitShachar, PrayerGeneratorId::ShaharitOpening, PrayerGeneratorId::ShaharitZimra,
      PrayerGeneratorId::ShaharitShma,    PrayerGeneratorId::Amidah,          PrayerGeneratorId::Hallel,
      PrayerGeneratorId::ShaharitTahanun, PrayerGeneratorId::TorahReading,    PrayerGeneratorId::Mussaf,
      PrayerGeneratorId::ShaharitSof,
  };

  static inline constexpr PrayerGeneratorId kMinha[] = {
      PrayerGeneratorId::Minha,
  };

  static inline constexpr PrayerGeneratorId kArvit[] = {
      PrayerGeneratorId::Arvit,
  };

  static inline constexpr PrayerGeneratorId kMusaf[] = {
      PrayerGeneratorId::Mussaf,
  };
};

}  // namespace SiddurEngine
