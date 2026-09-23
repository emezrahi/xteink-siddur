# Smart Siddur 7.5.286: source-grounded X3 parity audit

**Status:** Packaged APK navigation, localized labels, settings, font inventory and candidate prayer-title resources have been decoded. Runtime prayer-row order, conditional branches, screen appearance and default font still require verification. This is an evidence-based audit, **not** a claim that the Android app has been fully reproduced.

**Owner requirement:** reproduce the provided Smart Siddur's actual menus, all four nusachim, zmanim, choices, wording, font options and behavior on the Xteink X3. The sole intentional reading/navigation change is replacing Android vertical scrolling with button-controlled selection and discrete pages. Do not invent or reorganize chapter titles.

## Exact source identification

- **Correct package:** `com.karriapps.smartsiddurlite`, version **7.5.286**, versionCode **286**.
- Source APKs, held privately outside this repository:
  - Base SHA-256: `e5e36e41d9e317ad920f173e4383c04ca9aa4b39aade4ba65ec64882147e35ac`
  - ARM64 split: `7f9a4516983150ce215be0d773485387fe1510677ed1b8bcbb796ff688cf3101`
  - xxhdpi split: `765063a19a39641c99863c27265050fe7ebeaf0bd701b032d6cb8798c1b8c8cb`
- **Important correction:** a separate file misleadingly named `smart-siddur.apk` is **Aptoide**, package `cm.aptoide.pt`, version **9.22.5.3**; it must never be used as a Smart Siddur reference.
- Internal-generator baseline: `docs/siddur/SMART_SIDDUR_REFERENCE.md` and the separately held `smart_siddur_resource_map.md`. Neither by itself verifies the actual on-screen chapter list.

## 1. Static navigation, in the APK's XML order

Derived from `res/menu/navigation.xml` and `resources.arsc` English and Hebrew (`iw`) localizations. The user can select a language; a localized resource's existence is **not** proof that it appears under every runtime configuration. The actual running app might alter visibility/order dynamically.

| Main category | Items in source XML, in order |
|---|---|
| **לוח שנה וזמנים** / Calendar and Zmanim | לוח שנה וזמנים (Calendar and Zmanim); תזכורות (Reminders) |
| **מועדים מיוחדים** / Special Holidays | ספירת העומר; עולו אושפיזין; לג בעומר; ברכת הלבנה; הבדלה; הדלקת נר חנוכה; ברכת האילנות; קינות; סליחות; התרת נדרים |
| **תפילות יומיות** / Daily Prayers | שחרית; מנחה; ערבית; אשר יצר; קריאת שמע שעל המיטה; תיקון חצות |
| **ברכות** / Blessings | ברכת המזון; מעין שלוש; הפרשת חלה; ברכות נוספות; הפרשת תרומות ומעשרות; תפילת הדרך; ברית מילה; שבע ברכות |
| **Footer** | צא מהחשבון (Logout); מיקומים (Locations); הגדרות (Settings); עזרה ומשוב (Help and Feedback) |

The Android drawer header, from `res/layout/fragment_navigation_drawer.xml`, has Hebrew date, weekday, holiday and location fields, plus a compass and optional ad-banner view. The app contains calendar, luach, location, reminder, compass, account, purchase and notification activities; presence in the APK does not mean every activity should be assumed always visible.

## 2. Four exact nusach choices

The user-facing choices are present in the packaged `array/nusach`; `array/nusach_values` contains `0,1,2,3`.

| Stored value | Hebrew label | English label |
|---|---|---|
| 0 | עדות המזרח | Edot HaMizrach |
| 1 | ספרד | Sfarad |
| 2 | אשכנז | Ashkenaz |
| 3 | אר"י (חב"ד) | Ari (Chabad) |

The XML does not declare a default `nusach` value; do not guess. The compiled generator resources contain multiple nusach-specific text variants. Current X3 text covers only the Edot weekday Shaharit baseline; the other three must not be represented as complete until verified, suitably licensed text is installed.

## 3. Actual prayer reader design and controls

- `res/layout/fragment_tfila.xml` contains an Android `RecyclerView`, a toolbar and a pinch-to-change-text-size overlay.
- `res/layout/tfila_row.xml` contains a title, expand arrow and `ExpandableLayout` with prayer text. The model has `getTitle`, `getText`, `getExpand`, `getAddToMenu` and `getId`.
- `res/menu/tfila_menu.xml` exposes **Normal**, **Chazan** and **Yahid** modes; **Mourning House** and **No Tahanun** special choices; Theme and Share actions; and a nusach submenu declared initially hidden in XML (possible runtime visibility).
- **Do not use the prototype's manually grouped chapter headings.** The actual selectable section sequence must come from generated visible `PrayerTextItem` rows and their `addToMenu` settings for the selected date/nusach/mode.
- X3 substitution: retain actual item titles, item order and section choices; use buttons to choose items and turn discrete text pages. Prayer reading pages should not display the prototype's added Shaharit/section/prayer triple heading or page-number overlay. Keep Back/Previous/Next discoverable.

The earlier resource-map audit contains **223 title-resource occurrences across 12 generator families**: six Shaharit-related generators, Mussaf, Mincha, Arvit, Hallel, Omer and Torah reading. These are *candidate titles in compiled code*, not a complete or proven visible table of contents; conditional branches still need verification.

## 4. Packaged settings, including declared defaults

Based on `res/xml/preferences.xml`. Blank means **no default in that XML**; code or persisted settings can still set one.

| Category | Key | On-screen setting | XML default |
|---|---|---|---|
| General | `lange` | בחר שפה / Language | not declared |
| General | `track` | Allow Tracking | true |
| General | `push` | Allow Push | true |
| General | `silent` | Silent Mode | 1 |
| Siddur | `woman` | אני אשה / I am a woman | not declared |
| Siddur | `nusach` | בחר נוסח / Choose Nusach | not declared |
| Siddur | `tal` | Show Morid HaTal | true |
| Siddur | `mode` | Normal / Yahid / Chazan | 0 (Normal) |
| Siddur | `pasuk` | Enter Your Pasuk | not declared |
| Siddur | `sick` | Prayer For the Sick | not declared |
| Siddur | `mukaf_pref` | Purim Date | 0 |
| Siddur | `mizrochnik` | Israel National Holiday prayers | not declared |
| Siddur | `vanenu` | Add Va'anenu | false |
| Siddur | `nachem` | Show Nachem | false |
| Siddur | `is_in_israel` | Force Israel prayers and holidays | false |
| Appearance | `port` | Keep Portrait | false |
| Appearance | `awake` | Keep Screen On | true |
| Appearance | `longPress` | Long tap to switch to other parts | true |
| Zmanim | `pref_zmanim` | Open Jewish times/calendar settings | — |

Other packaged choices: languages English/עברית/français/español/Deutsch/system; silent mode Always/Never/Ask; Purim 14 Adar/15 Adar/both; `mode` Normal/Yahid/Chazan.

## 5. Zmanim: exact selectable methods and XML defaults

`res/xml/zmanim_preferences.xml` declares:

| Key | Setting | Default | Ordered choices |
|---|---|---:|---|
| `_24` | 24-hour format | true | off/on |
| `shabbat_start` | Candle-lighting lead time | 20 | minutes before sunset |
| `shabbat_ends` | Havdala | 1 | 20 min after sunset; three small stars; 40 min after; 72 min after |
| `dawn` | Alot HaShachar | 1 | 90 min before sunrise; 72 min before; 72 min by degrees (16.1°) |
| `sunrise` | Sunrise/sunset | 1 | Elevation-adjusted; level horizon |
| `zamnKlali` | General shita | 1 | Magen Avraham; Gra/Baal Tanya |
| `tzet` | Tzet HaKochavim | 2 | Hacham Ovadia 13.5 min; GRA 18 min; Baal Tanya 24 min; Chazon Ish 40 min; Rabenu Tam 72 min |
| `change` | Hebrew-date rollover | 1 | sunset; 5 minutes after sunset; tzet |

The included string resources also name Alot, earliest tzitzit/tefillin, sunrise, latest Shema and Shaharit, midday/midnight, Mincha Gedola, Plag, sunset, Tzet, Rabenu Tam Tzet, fast beginning/end, candle lighting and Havdala. Inclusion/precise calculation for a particular location/date must be checked against runtime behavior and independently tested. **Our current X3 engine only implements a small fraction of these adjustable zmanim methods.**

Location assets include `cities1000.json` and `locations.db`; the Android package requests coarse/fine location and contains a Locations activity. On X3, coordinate entry/offline city lookup and accurate local timezone/DST are needed; Android GPS cannot simply be assumed available.

## 6. Typography: actual packaged choices and fonts

- `res/layout/view_theme_chooser.xml`: Dark Mode, **Show Titles**, Line Spacing slider, Font selector, Save.
- `array/font_name`: **Default, Shofar, Keter, Alef**; `array/theme`: Light, Dark.
- Font assets: `Alef-Regular.ttf`, `Alef-Bold.ttf`, `KeterYG-Medium.ttf`, `KeterYG-Bold.ttf`, `Mekorot-Vilna.ttf`, `ShofarRegular.ttf`, `ShofarDemi-Bold.ttf`, `TaameyFrankCLM-Medium.ttf`, `TaameyFrankCLM-Bold.ttf`, `fontawesome.ttf`.
- The Keter YG and Taamey Frank CLM font metadata records indicate GPL-2.0 for the font and MIT for OpenType layout logic. Do **not** redistribute embedded files from the APK without checking each license and the redistribution obligations. Font files must remain private; use legally sourced alternatives or authorized licensed originals when packaging.
- The actual mapping and default for the UI's `Default` option has not been verified from running Android UI or method-level font selection code. Do not infer it solely from the presence of assets.

## 7. Parity work: verified versus still open

**Verified from the supplied APK:** app identity and splits, static navigation/menu labels (both languages), settings keys/defaults/arrays, prayer-row UI primitives, four nusach labels, embedded font inventory and some license metadata, 14 Zmanim-title string resources, named generator classes and candidate section labels.

**Still requires dynamic or deeper code validation:** every generated prayer item's actual `addToMenu` and conditional visibility; exact hierarchy for each nusach/service/date; initial screen and runtime menu reordering; actual default prayer font and font sizes; full liturgical text differences; precise calendar and zmanim computations; Android-specific settings/notification/billing behavior.

**No-silent-substitutions rule:** exactly preserve observed UI labels and functions where technically/licensably possible. The only requested deviation is replacing scroll with button-operated pages. Because the X3 is ESP32-C3, the original Android APK cannot execute there; a clean independent port is required, and irreducible Android/hardware or asset-license deviations must be documented and approved.

## 8. Implementation gates

1. Recover dynamic visible prayer rows (`getAddToMenu`, `getExpand`), conditions and per-nusach content selection from *observed* APK behavior; do not fabricate sections from resource names.
2. Create reference screen/section fixtures by nusach and representative dates, including zmanim preference combinations, and write X3 behavioral parity tests against them.
3. Build the Android-derived static navigation hierarchy in a paged X3 selector and the actual expandable-row workflow without scrolling. Keep all content/licensed font files outside the public GitHub repository until redistribution is cleared.
4. Port the remaining nusachim, full text and configurable zmanim methods incrementally, testing date rollover, Israel/diaspora, mode and gender cases.
5. Confirm actual X3 rendering and navigation on hardware against the Android reference before merging new UI.

**Previous prototype:** PR #12 was closed unmerged; its manually constructed chapter groupings are invalid as the new spec. Existing `master` remains intact as a historical recovery point. Do not destructively reset code during the audit.
