#pragma once

namespace SiddurEngine {

// Clean-room generator boundaries modeled from the Smart Siddur APK.
// These identify composition stages only; no Smart Siddur code is copied.
enum class PrayerGeneratorId {
  ShaharitOpening,
  ShaharitShachar,
  ShaharitZimra,
  ShaharitShma,
  Amidah,
  ShaharitTahanun,
  ShaharitSof,
  Mussaf,
  Minha,
  Arvit,
  Hallel,
  Omer,
  TorahReading,
  Slihot,
  BirkatHamazon,
};

}  // namespace SiddurEngine
