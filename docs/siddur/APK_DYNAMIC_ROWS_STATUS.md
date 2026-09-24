# Smart Siddur dynamic prayer rows: verification status

The public repository verifies the static XML navigation recorded in
`APK_STATIC_UI_FIXTURE.json`; this branch also provides a **generic, synthetic-tested**
navigator that accepts caller-supplied generated prayer rows and `addToMenu` flags.
Neither part proves the runtime prayer-section list of the Android application.

## Static APK evidence recovered privately

The owner's exact Smart Siddur 7.5.286 base APK was inspected for title-setter
call sites within six Shaharit generators. The resulting **candidate inventory** is
recorded in `APK_SHACHARIT_TITLE_SETTERS.md`. It is linear bytecode evidence:
branch-dependent calls may never occur together; neither emitted order nor
`getAddToMenu` values follow from their offsets.

The original Android APK, bundled fonts, proprietary text and decompiled code
must **not** be committed to the public repository.

## Required evidence for the next audit

An authorized auditor with lawful access to the exact APK in
`APK_PARITY_AUDIT.md` should trace `PrayerTextItem` creation and its
`getTitle`, `getAddToMenu`, `getExpand`, and ID values. Capture the **actual
generated sequence** for representative dates under each of the four nusachim,
plus prayer modes and relevant gender, location, Israel/diaspora, holiday,
mourning-house and no-Tahanun settings. For each capture record a reproducible
input context and row-level output fixture, with no copyrighted prayer body or
APK asset. Cross-check with Android UI screenshots or instrumented runtime
outputs when legally and technically feasible.

## Explicit unknowns

- Runtime order and visibility of generated prayer rows; exact contents headings.
- Conditional `getAddToMenu` and initial `getExpand` values.
- Per-nusach prayer-body selection and completeness.
- Date, location, mode, gender and Israel/diaspora effects.
- Whether Android reorders or hides static XML menu entries at runtime.

The new dynamic-navigation model contains **no guessed prayer sections**.
It will consume verified rows only when a separate APK parity fixture exists.
