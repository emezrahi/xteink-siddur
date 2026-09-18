# SiddurEngine

Portable prayer-composition logic for the X3 Siddur project.

The UI does not decide which prayers belong in a service. It supplies a `PrayerContext` to `Composer`, which returns an ordered list of `PrayerBlockId` values. The nusach content layer then resolves those IDs to text.

Current Phase 5 behavior:

- The visible Shaharit flow remains the three hardware-tested morning blessing blocks.
- A separate `composeWeekdayAmidah()` path models the first Amidah insertion boundary.
- Normal context composes `RetzehOpening -> RetzehConclusion -> Modim`.
- When `PrayerContext::isRoshHodesh` is true, `YaalehVeyavoRoshHodesh` is inserted between the two Retzeh blocks.
- Minha, Arvit, and Musaf are not yet exposed as complete services.

The incomplete Amidah segment is intentionally not appended to the visible Shaharit reader. It exists to prove the rules architecture without presenting a structurally incomplete service as usable prayer.


## Phase 6: offline Hebrew calendar

`HebrewCalendar` converts a civil Gregorian date to a fixed Hebrew calendar date without network access.

It currently provides:

- Gregorian -> Hebrew date conversion
- Hebrew leap-year detection
- variable Cheshvan/Kislev month lengths
- Adar / Adar I / Adar II handling
- Rosh Hodesh detection for both day 1 and day 30

The conversion intentionally treats the supplied civil date as a daytime calendar date. Sunset rollover, timezone, and location are separate concerns and will be applied when resolving a real `PrayerContext` from the X3 clock.


## Phase 7: PrayerContext resolution

`PrayerContextResolver` bridges the calendar and prayer-rule layers. Given a prayer service, civil date, and whether sunset has already passed, it:

1. advances the civil date by one day when necessary,
2. converts that date to the Hebrew calendar,
3. derives liturgical flags currently supported by the engine.

The first derived flag is `isRoshHodesh`. Rosh Hashanah (1 Tishrei) is explicitly excluded from that flag.

The resolver accepts `afterSunset` as an input; it does not calculate sunset itself. Location/timezone-aware sunset calculation remains a later device-integration layer.
