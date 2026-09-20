# Smart Siddur reference baseline

Smart Siddur is the behavioral reference for the X3 Siddur project. The X3 implementation remains an independent C++ implementation for CrossPoint and does not embed decompiled Android code.

## Reference package

The inspected installation is:

- Package: `com.karriapps.smartsiddurlite`
- Base APK plus ARM64 and xxhdpi split APKs
- Prayer content is bundled locally rather than fetched only at runtime
- The base APK contains 2,343 compiled string resources and local prayer/configuration assets

The Android application's four internal nusach values are:

- EDOT
- SFARAD
- ASHKENAZ
- CHABAD

The X3 project continues to target Edot HaMizrach first.

## Generator structure

Static analysis of the APK shows these primary prayer generators:

- `ArvitGenerator`
- `MinchaGenerator`
- `HalelGenerator`
- `OmerGenerator`
- `TorahReadingGenerator`
- `MazonGenerator`
- `SlihotGenerator`
- `MussafGenerator`

The reference Shaharit composition pipeline is:

1. `ShacharitShacharGenerator`
2. `ShacharitOpeningGenerator`
3. `ShacharitZimraGenerator`
4. `ShacharitShmaGenerator`
5. Amidah composition
6. conditional `HalelGenerator`
7. `ShacharitTahanunGenerator`
8. conditional `TorahReadingGenerator`
9. conditional `MussafGenerator`
10. `ShacharitSofGenerator`

This order was verified from the constructor/invocation order in `ShacharitGenerator`, not inferred from class names. It is represented explicitly in `ServiceGeneratorPlan` so the X3 implementation can be ported stage by stage without turning `Composer` into one large rules function.

## Resource families observed

The APK contains distinct or conditional resources for, among other things:

- Modeh Ani and morning blessings
- Netilat yadayim
- Birkot HaTorah
- Tallit and tefillin
- Ma Tovu
- Korbanot and ketoret
- Pesukei DeZimra
- Birkot Keriat Shema
- Shema
- Amidah
- Tahanun and fast-day Selihot
- Torah reading
- Hallel
- Mussaf
- Sefirat HaOmer
- service conclusions and Aleinu

Edot-specific resource names exist in the package, while many common Edot passages use resources whose historical Android resource names contain `Sefarad`. Resource names are therefore treated as implementation labels, not as authoritative nusach labels.

## Porting rule

Smart Siddur is used to answer two questions:

1. What prayer blocks exist and in what order can they appear?
2. Under what calendar/service conditions are those blocks included, omitted, or substituted?

For the X3 app:

- Calendar and rule behavior is reimplemented independently in `SiddurEngine`.
- Text is stored in the X3 content layer and should come from a distributable/verified Edot HaMizrach source.
- Android UI, analytics, billing, location database, and application framework code are not ported.
- The X3 implementation stays offline-first and constrained for ESP32-C3 memory.

## Next implementation slices

The next slices should port behavior in this order:

1. Ordinary weekday Shaharit baseline — implemented in Phase 10 using the verified generator pipeline above.
2. Full weekday Amidah seasonal/insertion rules.
3. Tahanun/no-Tahanun resolution.
4. Minha and Arvit.
5. Rosh Hodesh + Hallel + Mussaf.
6. Hanukkah, Purim, fast days, Omer, and Torah-reading variations.
7. Location-aware sunset/zmanim integration.
