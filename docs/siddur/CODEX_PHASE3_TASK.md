# Codex Phase 3: Put the verified Smart Siddur menu on the actual X3

**Owner-authorized implementation task.** Work on this branch `codex/siddur-x3-static-ui-phase3` only, based on `master` merge `b70d7e4c88aed88342bad96a52e8efa779c0a2c8`. Commit code and tests, publish to THIS PR branch if permitted, then request `@codex review`. Do not merge or deploy. Never reset, rewrite or force push. Keep existing firmware working.

## Goal

We have *two complete, validated models* in `lib/SiddurEngine/StaticNavigation.h` (PR #16) and `lib/SiddurEngine/DynamicPrayerNavigation.h` (PR #17). **Neither is used by the physical X3 screen yet.** The X3 `SiddurActivity` still presents one English "Shaharit" button. Replace that placeholder with the APK-verified static menu on the X3, with up/down, Confirm and Back buttons and discrete (non-scrolling) pages.

**Critical product requirement:** The owner wants an exact independent Smart Siddur Lite clone, with no intentional behavioral change except paging instead of scrolling. The verified fixture describes *static XML structure only*, not observed runtime hiding/reordering or actual prayer subchapters. Do not invent, expose or imply completed features. The prior unmerged PR #12 fabricated chapter groupings and MUST NOT be reused.

## Read first

- `AGENTS.md`: CPU/RAM constraints, button conventions, HAL, i18n, formatting wrapper `./bin/clang-format-fix -g`; clang-format **21**.
- `docs/siddur/APK_PARITY_AUDIT.md`, `docs/siddur/APK_STATIC_UI_FIXTURE.json`, `docs/siddur/APK_DYNAMIC_ROWS_STATUS.md`.
- `lib/SiddurEngine/StaticNavigation.h`, `lib/SiddurEngine/DynamicPrayerNavigation.h`.
- `src/activities/siddur/SiddurActivity.{h,cpp}`, `src/activities/siddur/content/EdotWeekdayShaharit.h`, `test/siddur_engine/CMakeLists.txt`.
- Search the repository for how real UI Activities use `MappedInputManager::Button::NavPrevious`, `NavNext`, `Confirm`, `Back`, Hebrew RTL rendering, `GUI.drawButtonMenu`, renderer locks and translatable strings. Cite exact paths/lines before code decisions; don't guess APIs.

## Implementation

1. Replace the **one-button main Siddur placeholder** with the 4 verified top-level categories, nested lists containing exactly the 26 verified leaf menu items in their original order, and discrete pages with a realistic row count for the e-ink screen. Use `SiddurStaticNavigation::NavigationSelection` and its `MenuEntry` data, not another hardcoded menu.
2. Draw Hebrew labels in proper RTL with the existing tested Siddur Hebrew font and safely render `std::string_view` by explicit lengths / a bounded NUL-terminated buffer at any C-style `GfxRenderer` boundary. Respect existing i18n policy; for exact APK-derived labels, source from the static fixture generated table and document any specific exception to `tr()` instead of inventing translated labels.
3. Make the **one already functional path**, `daily_menu` > `shacharit_menu`, open the existing **Edot HaMizrach Shaharit baseline** unchanged. Preserve the working date/zmanim preview, page-by-page reading, previous/next, and existing settings. On Back from reading, return to the same category/selection rather than losing navigation context; Back from category returns to root and from root exits Activity. Up/down and page navigation should behave consistently with four physical X3 keys. The current `NavPrevious`/`NavNext` handlers should have context-sensitive behavior when showing menus versus prayer pages.
4. **Do NOT add fake functional destinations.** The other 25 XML leaves are static XML menu evidence only; choosing one must clearly indicate it is not yet implemented (not blank, not a crash, not misleading prayer text). If exposing nonworking leaves at all creates a user-experience risk, propose an alternative in the PR but keep this PR's code grounded in verified labels. Don't claim that all four nusachim or services work: the existing prayer text is only Edot weekday Shaharit.
5. The full Smart Siddur dynamic `PrayerTextItem` emitted order and `getAddToMenu` flags are still unverified. **Do not connect the dynamic contents navigator to the current hand-composed Edot blocks and label them APK headings.** That will be a separate, source-verified task after an APK runtime/DEX control-flow audit. The current prayer page-turning remains unchanged.
6. No bundled APK/font assets or proprietary Android code in the public repo. New RAM allocations must be justified and no unbounded allocations added in render/loop hot paths.

## Tests & verification

Add focused host tests for mapping the exact fixture's 4 groups/26 leaves to navigation actions, previous/next, page boundaries, Back selection retention, Shaharit leaf dispatch and unsupported leaf dispatch. Keep the device state machine independently testable where feasible; if full UI requires mocked hardware, limit unit tests to the controller and verify rendering through CI firmware builds. Do not mark untested physical X3 behavior as proven.

Run clang-format 21 repository wrapper (format only files changed by this PR), all host tests, cppcheck and all four firmware CI builds. When local PlatformIO or formatter is unavailable, rely on GitHub Actions and fix its actual failures. Report output SHA, updated CI run and precise limitations. Request Codex review on the final pushed implementation. Never self-merge or deploy.

## Next distinct workstream, NOT in scope

The owner's exact private Smart Siddur APK **is preserved outside the public repo**. Analyze actual dynamically emitted prayer-row title, ID, `addToMenu`, `expand` and context before integrating the already-merged `DynamicPrayerNavigation` into the X3. See `docs/siddur/APK_SHACHARIT_TITLE_SETTERS.md`: bytecode title-setter offsets are *not* a verified runtime row sequence.
