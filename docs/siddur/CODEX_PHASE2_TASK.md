# Codex Phase 2: APK-style dynamic prayer row navigation (data model and tests)

**Owner has authorized Codex coding and manual review.** Implement on branch `codex/siddur-prayer-row-navigation-phase2` against merged `master`. Submit changes to the existing draft PR. No force pushes, resets, hidden costs, automatic merge or deployment.

## Read first

- `AGENTS.md` for ESP32-C3 memory rules (380 KB usable RAM), repository's clang-format-21 wrapper and build/test instructions.
- `docs/siddur/APK_PARITY_AUDIT.md` and `docs/siddur/APK_STATIC_UI_FIXTURE.json`.
- `docs/siddur/APK_SHACHARIT_TITLE_SETTERS.md` for newly recovered, **non-runtime** bytecode evidence from the owner's actual Smart Siddur 7.5.286 base APK; the APK itself is kept private.
- `docs/siddur/APK_DYNAMIC_ROWS_STATUS.md`.
- `lib/SiddurEngine/StaticNavigation.h` and its 219-test CI baseline (PR #16, merged). Reuse its safe paging semantics where suitable. Do not use the obsolete, invented grouping implementation from closed/unmerged PR #12.

## Required deliverable

Create a compact, independent **data-driven dynamic prayer-row navigation model** suitable for plugging into a source-verified Smart Siddur prayer generator in a later PR. The Android APK distinguishes title, text, expansion and `PrayerTextItem.getAddToMenu()`; our first APK static audit **does not** yet prove exact runtime rows or conditional menus. This task must make **no claims of having recovered those conditions**.

Design an API taking a non-owning ordered span of row descriptors, with:
- stable source-row index, title (UTF-8 view), and the actual **caller-provided** `addToMenu` boolean;
- optional row ID/expandable metadata if useful; no proprietary body text, no baked-in title lists;
- menu entries **only** for rows whose `addToMenu` is true, preserving caller order, duplicates and exact caller titles;
- button-driven discrete paging: up/down, page-forward/back, select returns the **underlying row index**, and Back returns to contents without losing relevant selection;
- zero rows, no menu-eligible rows, all-hidden rows, single-row and oversized page capacities safely handled without overflow;
- no linear-time/per-refresh allocations or copies of the full prayer text; either scan rows lazily or use a bounded static index with a justified bound. No full JSON on device.

Use an API that can support different daily prayers, all four nusachim and dynamic date/nusach/mode conditions **by consuming caller-provided runtime rows**, not guessing the conditions. Preserve the separation between the verified `StaticNavigation` top-level drawer and these conditional prayer rows.

## Tests and integration boundary

Add focused host tests using **clearly synthetic** row fixtures (not invented "Smart Siddur" menus). Cover hidden items, repeated identical titles, empty/fully hidden lists, per-day differing input sequences, returned source indices, page capacities 1/2/5, huge sizes/overflow, stable Back navigation and UTF-8 Hebrew names. Compare model behavior to the supplied fixture **only where the fixture actually has runtime row facts**; it currently has none.

Do **not** change the existing X3 Siddur UI to show a fake dynamic contents list or misleading unimplemented services. Do not port the APK's Android classes, bundled fonts, proprietary prayer text or apps. Don't modify current zmanim behavior, settings, or production firmware in this task.

In `docs/siddur/APK_DYNAMIC_ROWS_STATUS.md`, update the status: the original APK's six Shaharit generator methods contain the title-setter call-site evidence in `APK_SHACHARIT_TITLE_SETTERS.md`; visible row conditions and exact emitted order remain to be verified. Include the next narrow evidence-gathering plan.

## Validation

Run the repository's `./bin/clang-format-fix` with clang-format **21** (use CI when unavailable locally); generator fixtures must remain byte-identical after formatting. Run host tests and all relevant GitHub Actions builds and cppcheck. Report result and final commit. Request a **fresh** `@codex review` on the final pushed commit. Do not merge until checks and review pass.

**Note:** The original APK is available to the owner in this conversation's protected workspace, **not to GitHub-hosted Codex**. This phase implements a correct consumer for future verified runtime rows. Do not falsely claim runtime APK parity.
