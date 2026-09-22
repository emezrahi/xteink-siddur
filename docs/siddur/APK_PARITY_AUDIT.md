# Smart Siddur APK parity audit (scope reset)

The project owner requested that the X3 Siddur match the supplied Smart Siddur APK in its **actual on-screen navigation, menu hierarchy, all nusachim, displayed text, settings, date/zmanim functionality, fonts, and behavior**. The **only intentional interaction change** is replacing vertical scrolling with discrete pages and button-operated selection suitable for the X3. Other deviations must be identified and approved, not guessed. This document is the initial audit scope, **not a claim of parity**.

## Source of truth

Inspect the actual user-provided APK set, held outside this public repository. Do not re-upload or ask for new ADB extraction.

| File | SHA-256 |
| --- | --- |
| smart-siddur-base.apk | e5e36e41d9e317ad920f173e4383c04ca9aa4b39aade4ba65ec64882147e35ac |
| smart-siddur-arm64.apk | 7f9a4516983150ce215be0d773485387fe1510677ed1b8bcbb796ff688cf3101 |
| smart-siddur-xxhdpi.apk | 765063a19a39641c99863c27265050fe7ebeaf0bd701b032d6cb8798c1b8c8cb |
| smart-siddur.apk (additional archive) | 188bbdb899be2da401ac0471ba4d10693612fa1067f2ea0987cc6e1afcfd4374 |

Partial earlier analysis: `docs/siddur/SMART_SIDDUR_REFERENCE.md` and user-supplied `smart_siddur_resource_map.md`. **These record internal generators/resource names, not the exact visible user interface.**

## Observed in base APK (direct ZIP inventory)

- Four internal nusach values documented from earlier analysis: `EDOT`, `SFARAD`, `ASHKENAZ`, `CHABAD`. Exact user-visible labels and settings must still be verified.
- Typeface files under `assets/`: Alef Regular/Bold, KeterYG Medium/Bold, Mekorot-Vilna, Shofar Regular/Demi, TaameyFrankCLM Medium/Bold, FontAwesome. Presence does **not** establish the default fonts or a license to redistribute them.
- Data assets include `cities1000.json`, `locations.db`, `slihot.json`, `split_1.json` through `split_4.json`, `sukot.json`.
- The APK contains `res/menu/navigation.xml`, `res/menu/tfila_menu.xml`, `res/menu/treefold.xml` and calendar/mazon menu resources. These require decoding and comparison with actual on-screen behavior.

## Required audit before more UI code

1. Decode and document every visible screen and its exact hierarchy, labels, language, ordering, selection rules, navigation/back behavior and conditional entries. Check against the reference running on Android if static resources cannot establish visible behavior.
2. Inventory every nusach's prayer content and inclusion conditions, including service/day/holiday, gender, location and user settings, as exposed by the application. Record differences between nusachim, do not flatten them.
3. Document zmanim, geographic lookup, timezone/DST, Hebrew-date changes, calendar, holidays, alarms/notifications, prayer reminders and any other present feature. Mark unverified items explicitly.
4. Document default and selectable typefaces, sizes, text direction, vowel-point/cantillation support, visual hierarchy and readability; investigate licenses before packaging.
5. Define screen-by-screen X3 page-turn and physical-button interaction, preserving reference labels and options. Show exact deviations forced by hardware or licensing and obtain owner approval **before** coding.
6. Create a parity checklist and real-device acceptance tests that compare the Android reference with the X3 implementation.

## Technical and licensing constraints

The original APK is an Android app. Xteink X3 uses an ESP32-C3 without Android and cannot run the APK. A native X3 implementation can reproduce observed behavior but cannot be a literal execution of the same APK; mobile screen geometry, touch interactions and Android-dependent integrations must be audited for feasibility. The X3 has severely constrained RAM/flash and needs offline/paged storage for large data. Do not commit or redistribute the original APK, proprietary code, prayer assets or bundled fonts without verified permission/licenses. Use independently implemented logic and appropriately licensed or permissioned content when shipping.

## Existing prototype

PR #12 (hand-designed chapter/navigation prototype) was closed without merging at the owner's request. Preserve Git history for recovery but treat previously invented chapter labels and unverified UI as discarded. Do not merge or continue that prototype. The merged date/zmanim engine can be kept only after comparing its behavior with the actual reference. No destructive Git resets or deletions are needed for the audit.
