# Codex task: source-verified Smart Siddur navigation, phase 1

**Owner authorization:** use Codex to implement the next focused step of the Xteink X3 Siddur project. Work only on `codex/siddur-apk-navigation-phase1` against the current `master` baseline. Submit a PR for human review. Do not merge, reset, rewrite or delete existing work.

## Read first

- `AGENTS.md` (embedded constraints, formatting wrapper and test conventions).
- `docs/siddur/APK_PARITY_AUDIT.md` (source provenance, verified facts, outstanding unknowns and licensing limits).
- `docs/siddur/APK_STATIC_UI_FIXTURE.json` (verified *static* Android navigation, exact labels, ordered items, nusach values and zmanim choices).
- `docs/siddur/SMART_SIDDUR_REFERENCE.md` (generator-stage evidence, not screen layout).
- `lib/SiddurEngine/Composer.h`, `src/activities/siddur/SiddurActivity.cpp`, `test/siddur_engine/CMakeLists.txt` (current unfinished prototype).

The owner explicitly rejected the invented chapters from closed, unmerged PR #12. **Never revive them.** Do not invent dynamic prayer-section order from resource identifier names.

## This task's deliverable (repo-only, no private APK required)

Implement a compact, reusable *static navigation model plus page-selection logic* for the reference APK's actual four navigation categories and their 26 XML-defined leaf items. Preserve the JSON fixture's **exact source order, IDs, Hebrew/English labels**; do not mix in current manually composed prayer blocks. Use heap-free/predictable embedded-friendly constants and compile-time tables where appropriate. The page-selection logic must support discrete, non-scrolling pages on the X3 and be independent of GfxRenderer so it can be tested on the host.

Define a small independent API that returns (a) the categories and each category's indexed children, (b) the selected item when choosing/up/down/page forward/back, (c) page boundaries for an arbitrary positive number of visible rows, (d) navigation behavior at first/last item, (e) language-selected string labels. Integrate into the existing Siddur module only if you can do so **without showing unverified screens or breaking the currently flashable firmware**. Prefer model+tests first; X3 UI rendering is a follow-up after review.

Store the precise XML-backed navigation fixture outside runtime C++ if feasible as input to a minimal table generation script, or use a transparent manually translated constexpr table and exhaustive tests against the JSON. Do not bloat C3 RAM by loading the entire JSON on device.

### Required tests

- Four top-level categories, the exact 26 leaves and each category's ordering/ID/English/Hebrew labels from the current fixture.
- Every nusach choice exact value/label remains available as reference metadata (even though content is not ported).
- Test a 1-row, 2-row and 5-row page capacity, category and item selection, first/last page, return-to-menu and changing category.
- Zero rows must fail safely (no division by zero); empty selection must fail safely.
- Test that **unverified dynamic prayer rows cannot enter this verified-static menu** by accidentally reusing prototype labels.
- No static interface claims about actual runtime visibility of conditional Android menu entries.

### Scope boundary

The private Smart Siddur base/split APKs **are not in public GitHub**. This task must not claim to have inspected or decompiled them. Dynamic prayer `PrayerTextItem` row ordering, conditional `getAddToMenu` flags and per-nusach prayer-body selections remain **unverified** and are the *next* separate audit task requiring APK access. Create `docs/siddur/APK_DYNAMIC_ROWS_STATUS.md` noting exactly what is needed, with explicit unknowns and no invented section list. Do not embed or commit the APK, proprietary code, prayer text or embedded font files.

Do not change nusach-specific prayer content, date/zmanim calculations, firmware flashing or physical-button mapping in this task. Do not introduce automatic task/API billing or credentials.

### Validation & finish

Run host tests and relevant builds when tooling permits; run repository formatting wrapper, cppcheck and CI. Explain the result and any blocker. Post the final branch diff for human inspection. Request `@codex review` after coding; do not self-merge or deploy. Keep one PR scoped to the model, tests and evidence documentation. Distinguish passing existing CI tests from newly added tests.
