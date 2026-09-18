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
