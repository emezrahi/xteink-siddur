# SiddurEngine

Portable prayer-composition logic for the X3 Siddur project.

The UI does not decide which prayers belong in a service. It supplies a `PrayerContext` to `Composer`, which returns an ordered list of `PrayerBlockId` values. The nusach content layer then resolves those IDs to text.

Current Phase 5 behavior:

- The visible Shaharit flow remains the three hardware-tested morning blessing blocks.
- Weekday Amidah composition has its first real insertion point:
  `RetzehOpening -> [YaalehVeyavoRoshHodesh] -> RetzehConclusion -> Modim`.
- `YaalehVeyavoRoshHodesh` is inserted only when `PrayerContext::isRoshHodesh` is true.
- The partial Amidah sequence is not exposed in the Siddur UI yet, because the preceding Amidah blessings have not been added.
- Minha, Arvit, and Musaf remain unimplemented.

The next step is to expand the real weekday Amidah corpus around this tested rule before wiring automatic calendar detection.
