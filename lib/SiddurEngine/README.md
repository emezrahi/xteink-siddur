# SiddurEngine

Portable prayer-composition logic for the X3 Siddur project.

The UI does not decide which prayers belong in a service. It supplies a `PrayerContext` to `Composer`, which returns an ordered list of `PrayerBlockId` values. The nusach content layer then resolves those IDs to text.

Current Phase 4 behavior is intentionally minimal:

- Shaharit composes the three hardware-tested morning blessing blocks.
- Date/minhag flags exist in `PrayerContext` but do not alter composition yet.
- Minha, Arvit, and Musaf currently compose to an empty list.

Phase 5 will begin applying mock context rules before any real calendar calculations are connected.
