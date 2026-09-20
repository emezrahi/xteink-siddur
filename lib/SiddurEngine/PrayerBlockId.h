#pragma once

namespace SiddurEngine {

enum class PrayerBlockId {
  // Weekday Shaharit: Shachar / preparatory prayer.
  ModehAni,
  NetilatYadayim,
  AsherYatzar,
  ElohaiNeshama,
  MorningBlessings,
  TorahBlessings,
  PetichatEliyahu,
  Talit,
  Tefillin,
  HannasPrayer,

  // Opening / korbanot.
  MorningPrayer,
  IncenseOfferingMorning,

  // Pesukei DeZimra and Shema.
  Hodu,
  PesukeiDeZimra,
  ShemaAndBlessings,

  // Weekday Amidah.
  WeekdayAmidah,
  MoridHatal,
  MashivHaruach,
  Barechenu,
  BarechAleinu,
  RetzehOpening,
  YaalehVeyavoRoshHodesh,
  YaalehVeyavoFestival,
  AlHanissimHanukkah,
  AlHanissimPurim,
  Aneinu,
  RetzehConclusion,
  Modim,

  // Tahanun and weekday Torah service.
  ViduiRegular,
  ViduiMondayThursday,
  TorahReadingWeekday,
  HallelHalf,
  HallelFull,
  MussafRoshHodesh,
  MussafFestival,

  // Minha and Arvit.
  MinhaOpening,
  ArvitShemaAndBlessings,
  SefiratHaOmer,

  // End of Shaharit.
  AshreiAfterTahanun,
  UvaLezionRegular,
  UvaLezionTorah,
  BeitYaakov,
  SongOfDaySunday,
  SongOfDayMonday,
  SongOfDayTuesday,
  SongOfDayWednesday,
  SongOfDayThursday,
  SongOfDayFriday,
  SongOfDaySaturday,
  Kaveh,
  Aleinu,
};

}  // namespace SiddurEngine
