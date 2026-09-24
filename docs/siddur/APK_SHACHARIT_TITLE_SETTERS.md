# Smart Siddur Shaharit row-setter evidence (static DEX inspection)

Source: owner-provided Smart Siddur **7.5.286**, package `com.karriapps.smartsiddurlite`, base APK SHA-256 `e5e36e41d9e317ad920f173e4383c04ca9aa4b39aade4ba65ec64882147e35ac`; `classes3.dex`, the named Shaharit generators' `a()` methods. Original APK and extracted prayer bodies remain private and **must not be committed**.

The rows below are **linear Dalvik bytecode call-site evidence**, not a verified runtime row order or contents menu. An entry `offset=resource_key` means a nearby compiled resource reference was found before a `TfilaRow.n(String)` title-setting call. `? ` means no unambiguous nearby title constant. These calls occur in mutually exclusive or conditional branches, so many entries **will not appear** together on any given date/nusach. Compiler layout is not a liturgical sequence; runtime generator behavior and the add-to-menu boolean remain unverified. Do not build a selectable section list directly from this document.

| Generator | `TfilaRow.n` call sites | Matched nearby title resources |
|---|---:|---:|
| `ShacharitShacharGenerator` | 15 | 12 |
| `ShacharitOpeningGenerator` | 13 | 13 |
| `ShacharitZimraGenerator` | 22 | 16 |
| `ShacharitShmaGenerator` | 11 | 10 |
| `ShacharitTahanunGenerator` | 13 | 4 |
| `ShacharitSofGenerator` | 25 | 16 |

## Linear code-unit offsets and candidate title resource keys

- `ShacharitShacharGenerator`: 31=mode_title; 335=zizitKatanTitle; 405=netilaTitle; 688=?; 833=?; 1054=?; 1152=patachTitle; 1225=kadishDerTitle; 1310=zizitTitle; 1485=tfilinTitle; 1671=afterTfilinTitle; 1840=adonTitle; 1901=hanaTitle; 1952=pyutTitle; 2034=bShahar_title.
- `ShacharitOpeningGenerator`: 31=leshemTitle; 105=pyutTitle; 148=akedaTitle; 355=eluDvarimTitle; 396=leolamTitle; 600=kyorTitle; 674=deshenTitle; 715=tamidTitle; 937=ktoretTitle; 1166=abbayeTitle; 1254=anaTitle; 1410=korbanotTitle; 1645=kalTitle.
- `ShacharitZimraGenerator`: 49=zimra; 141=?; 198=?; 240=?; 438=menora; 531=?; 621=?; 771=mizmorTodaTitle; 867=?; 905=yeheKevodTitle; 948=ashreiTitle; 1039=hal1Title; 1080=hal2Title; 1121=hal3Title; 1162=hal4Title; 1203=hal5Title; 1244=davidTitle; 1362=azYashirTitle; 1427=nishmatTitle; 1515=yishtabachTitle; 1690=mimakimTitle; 1759=barechuTitle.
- `ShacharitShmaGenerator`: 37=tfilaZmanTitle; 187=yotserTitle; 433=ahavaRabaTitle; 448=ahavatOlamTitle; 599=shmaZmanTitle; 796=?; 840=shmaTitle; 1202=yazivTitle; 1354=sunriseZmanTitle; 1521=yeshemTitle; 1648=avinuTitle.
- `ShacharitTahanunGenerator`: 74=?; 274=?; 525=?; 707=?; 896=tahanunTitle; 2535=?; 2635=tahanun25Title; 2761=tahanunTitle; 2955=tahanun25Title; 3160=?; 3251=?; 3301=?; 3396=?.
- `ShacharitSofGenerator`: 20=ashreiTitle; 159=yaanchaTitle; 200=uvaTitle; 444=torahHahzaraTitle; 575=tfilaLedavidTitle; 657=betYaakovTitle; 743=?; 800=?; 898=?; 1261=specialMizmorTitle; 1521=avelTitle; 1613=hoshienuTitle; 1720=?; 1849=?; 1933=kaveTitle; 2082=ktoretTitle; 2237=tanaEliyahuTitle; 2411=?; 2535=?; 2735=?; 2809=afterAleinuChabadTitle; 2902=ledavidTitle; 3002=?; 3203=ikarimTitle; 3342=sheshZchirotTitle.

## Next evidence needed for actual APK parity

Recover runtime emitted prayer-row order and every row's `addToMenu` decision for **each nusach**, representative weekdays/Rosh Hodesh/holidays, individual vs chazan, mourning-house and no-Tahanun settings, gender variants, location and time. Verify how localized labels, optional expandable rows, and section navigation actually behave. Until those are verified, this is a candidate inventory only. See `docs/siddur/APK_DYNAMIC_ROWS_STATUS.md`.
