# Smart Siddur dynamic prayer rows: verification status

The public repository currently verifies only the static XML navigation recorded in
`APK_STATIC_UI_FIXTURE.json`. It does not contain the private Smart Siddur base or split APKs,
and this implementation did not inspect or decompile them.

## Required evidence for the next audit

An authorized auditor with lawful access to the exact APK identified in `APK_PARITY_AUDIT.md`
must capture the generated `PrayerTextItem` sequence for representative dates and every nusach,
prayer mode, gender setting, Israel/diaspora setting, and applicable special-day option. Each
capture must include item ID, localized title, order, `getAddToMenu`, `getExpand`, and the
conditions that caused the row to be included or omitted.

## Explicit unknowns

- The runtime order and visibility of generated prayer rows.
- Conditional `getAddToMenu` and initial `getExpand` values.
- Per-nusach prayer-body selection and completeness.
- Date, location, mode, gender, and Israel/diaspora effects on those rows.
- Whether Android reorders or hides any static XML menu entry at runtime.

Resource identifiers and the unfinished X3 prototype are not sufficient evidence for any of
these facts. No dynamic section list should be added to the verified static model until the
audit produces a reviewable fixture.
