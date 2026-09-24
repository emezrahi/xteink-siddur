# Phase 3: render the verified Smart Siddur navigation on Xteink X3

Owner's non-negotiable target: an X3-native replica of **Smart Siddur Lite 7.5.286**, not EPUB. Preserve source-verified Hebrew labels, the four nusach choices, the original screen hierarchy and functionality where supported. The **only intended final UX deviation is discrete hardware-button pages instead of vertical scrolling**. This PR is a limited *preview milestone*, NOT a claim of full Android parity.

## Read before editing
- `AGENTS.md`, particularly HAL / MappedInputManager, `tr()` i18n, C3 RAM limits and the repository-only clang-format wrapper.
- `docs/siddur/APK_PARITY_AUDIT.md`, `docs/siddur/APK_STATIC_UI_FIXTURE.json`, `docs/siddur/APK_DYNAMIC_ROWS_STATUS.md`.
- `lib/SiddurEngine/StaticNavigation.h` (merged Phase 1), `lib/SiddurEngine/DynamicPrayerNavigation.h` (merged Phase 2).
- `src/activities/siddur/SiddurActivity.{cpp,h}` and `src/activities/siddur/content/EdotWeekdayShaharit.h`.
- `docs/siddur/ENGINE_PROGRESS.md` for known date/zmanim limitations.

## Deliverable: tangible X3 menu, not another isolated model
In `SiddurActivity`, replace the current single English `Shaharit` landing button with **real navigable, button-paged UI** using `SiddurStaticNavigation::NavigationSelection`: the APK fixture's **four exact, source-ordered top-level categories** and their 26 exact Hebrew/English labels. Default to Hebrew for this project, but *do not assert that the original Android app defaults to Hebrew or Edot*; runtime APK defaults remain unverified. Use the source-backed fixture data, not invented lists or prototype chapter names.

Use existing mapped physical `Back`, `Confirm`, `NavPrevious`, `NavNext` actions; query screen dimensions for row/pagination layout, do not hardcode an assumed button position or physical rotation. Right-align Hebrew text and use existing licensed X3 Hebrew font and Bidi RTL path. Ensure no scroll animation: screen/page turns only. Distinguish selected item and page number with minimal display refresh. Preserve RTC date/section header where feasible without claiming it exactly matches Android style. Consult current app UI components before implementing.

When a selected static item is `shacharit_menu`, connect to the existing **Edot HaMizrach weekday Shaharit prototype** (do not change its prayer text / calendar logic). Other 25 menu items are *not yet implemented*: handle gracefully with a clear, localized preview-only/unavailable state, **never silently substitute Shaharit or fabricate text**. Do not show a fake dynamic contents menu or infer `getAddToMenu` flags; the original APK's actual runtime prayer rows have not yet been verified.

Provide a nusach selection surface exposing the four **exact** APK choices from `SiddurStaticNavigation::nusachChoices()` (Edot HaMizrach/Sfarad/Ashkenaz/Ari-Chabad). Only Edot currently has prayer-body content: selecting the other choices must not mislabel or display Edot text as another nusach. Prefer a clear disabled/preview indicator over pretending all texts exist. Do not change persisted settings format without explicit migration and tests. No proprietary bundled APK fonts, binaries or copyrighted prayer bodies may be committed.

## Tests and safety
Add host tests for menu state transitions and **page actions**, all four categories and 26 exact leaves (already covered in model tests; test relevant activity adapter/helper independently if feasible), non-Shaharit selection handling, four-nusach availability gating, selection preservation when returning from prayer reader, small/large screen heights or row capacities and zero-size behavior. No fabricated runtime rows. Carefully inspect current activity lifecycle and preserve exit behavior. Avoid new dynamic heap allocation in render/loop hot paths; don't increase the ESP32-C3 memory burden unnecessarily.

Run `./bin/clang-format-fix -g` with clang-format 21 or rely on GitHub CI if unavailable; run all host tests, cppcheck and firmware CI. Report all failures. Request a new `@codex review` after code is published to this branch. Keep the PR **draft and unmerged** until I inspect it and the owner tests actual X3 navigation. Do not auto-deploy or replace the existing working firmware.

## Separate concurrent evidence task (NOT part of public PR)
The original base APK is present in the owner's private ChatGPT workspace, not in this GitHub checkout. The real emitted Shaharit `PrayerTextItem` rows, per-row `getAddToMenu` and `getExpand` values and four-nusach/date differences must be audited separately **before** we wire dynamic prayer contents to this menu. Do not invent or infer that list from the bytecode title-setter inventory.
