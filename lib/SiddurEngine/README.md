# SiddurEngine

Portable prayer-composition logic for the X3 Siddur project.

The UI does not decide which prayers belong in a service. It supplies a `PrayerContext` to `Composer`, which returns an ordered list of `PrayerBlockId` values. The nusach content layer then resolves those IDs to text.

Current Phase 5 behavior:

- Shaharit includes the three hardware-tested morning blessing blocks.
- The Amidah test segment contains `RetzehOpening -> RetzehConclusion -> Modim`.
- When `PrayerContext::isRoshHodesh` is true, `YaalehVeyavoRoshHodesh` is inserted between the two Retzeh blocks.
- The X3 UI has a temporary Weekday / Rosh Hodesh selector so both compositions can be verified before the real calendar engine is connected.
- Minha, Arvit, and Musaf still compose to an empty list.

This is intentionally a structural test. The missing portions of Shaharit will be filled in as modular blocks after the insertion/rules architecture is hardware-verified.
