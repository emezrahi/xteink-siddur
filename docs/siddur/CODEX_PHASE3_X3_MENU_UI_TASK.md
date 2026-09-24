# Phase 3: Put verified Smart Siddur navigation on the physical X3

Owner-authorized Codex coding task. Base: merged `master` at `b70d7e4c88aed88342bad96a52e8efa779c0a2c8`. Implement directly on this PR's branch and publish the changes. Keep the PR unmerged until review and CI pass. Do not rewrite prior commits.

## Product intent and strict evidence boundary

The end goal is a faithful native X3 port of Smart Siddur Lite 7.5.286, with **button-operated pages replacing scrolling**. Do not claim the rest of the APK has been ported. The user previously rejected an invented chapter hierarchy in closed, unmerged PR #12.

The APK's original *static* drawer structure and wording are authoritatively recorded in `docs/siddur/APK_STATIC_UI_FIXTURE.json`. Phase 1's `lib/SiddurEngine/StaticNavigation.h` holds its 4 ordered groups and 26 ordered leaf entries. Phase 2's `lib/SiddurEngine/DynamicPrayerNavigation.h` can paginate *actual caller-provided* dynamic prayer rows. The **APK's runtime emitted row order and addToMenu conditions remain unknown**; see `docs/siddur/APK_DYNAMIC_ROWS_STATUS.md` and `docs/siddur/APK_SHACHARIT_TITLE_SETTERS.md`. Never turn bytecode resource-title references or existing X3 prototype block names into alleged Android menu chapters.

## Deliver an actual working X3 firmware UI

Replace the single-item `Shaharit` landing menu in `src/activities/siddur/SiddurActivity.{h,cpp}` with a button-driven rendering of the **verified static APK drawer**.

- Show the 4 categories and, after selection, their original source-ordered child entries from `SiddurStaticNavigation::categories()`. Favor Hebrew UI for the X3 milestone, preserving exact original text with RTL rendering and diacritics as supported; English labels remain in the model. Do not hardcode a second menu copy or translate Hebrew from English. Footer actions may be deferred if they cannot be wired accurately, but document deferrals, not silently present them as complete.
- Use existing physical input mapping: `Back` returns to category list, then exits; `Confirm` enters category or activates child; `NavPrevious`/`NavNext` move the current selection with discrete page boundaries. Show visible selection and an unambiguous page position. Use at most 4 visible menu rows and renderer APIs actually available in this repository. Use `std::string_view` responsibly at C-string boundaries; no temporary-string dangling pointers.
- **Only** `shacharit_menu` routes to the already supported baseline `openShaharit()`, preserving date calculation and the current paginated Edot HaMizrach text. All other original menu items should appear by their exact XML labels, but must **clearly indicate unavailable/not ported when opened**, never pretend to be functional. A temporary X3-specific unavailable screen is explicitly a development-status departure from the target APK, not a permanent redesign. Its Back returns to the same child selection.
- The existing Shaharit prayer-body page-turning and Back-to-menu behavior must survive. Back from Shaharit should return to its selected original drawer child entry, not lose context. Preserve currently functioning RTC/zmanim preview and text pagination. Don't change content/date logic or insert source-unverified sections.
- Do **not** present the generic `DynamicPrayerNavigation` as though it contains recovered Android data: leave it for the separately verified runtime row fixture. No unsupported font assets, Android bytecode, copied proprietary prayer bodies, extra storage dependencies or new billing.
- C3 constraints: no new per-frame heap churn, no large stack buffers, predictable small state. Reuse existing renderer Hebrew font and `MappedInputManager`, and follow `AGENTS.md` constraints for `tr()` user-facing program strings; original fixture's language-selected text is sourced navigation data. Do not change CrossPoint's other reader modes.

## Tests and verifiable completion

Extract pure route logic if needed so host tests prove **only Shaharit** can be opened, all 25 other entries remain visible but unavailable, original category/item orders are unchanged, back behavior preserves category+selected item, page capacity 4 handles the 10-item holiday group, and invalid/zero capacity is safe. If hardware UI isn't host-testable, separate navigation/dispatch tests from physical-render verification.

Run the project formatter wrapper using clang-format 21, the full host-test suite, cppcheck and `default` X3 plus other firmware CI builds. No extra allocations justified only by convenience. Explain any unverified Hebrew glyph/RTL and actual X3 button-mapping behavior pending physical hardware verification.

Document the actual status in `docs/siddur/X3_UI_PARITY_STATUS.md`: source-verified static labels, routed Shaharit baseline, visible-but-unimplemented screens and the still-missing dynamic prayer rows/nusachim/zmanim/fonts. This is a development milestone, **not a parity release**.

When finished, **publish code to this PR's branch** if GitHub write access permits, supply final SHA, request `@codex review` on the implementation commit, and leave PR unmerged. If Codex's workspace cannot push automatically, state that explicitly in the PR task response; do not claim the implementation is on GitHub when only the workspace has it.
