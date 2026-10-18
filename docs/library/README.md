# X3 Jewish Text Library

This branch pivots the X3 project away from the Siddur/zmanim feature and into an offline Hebrew text reader.

## Libraries

### Tehillim
- 150 chapters.
- Hebrew source: Sefaria export version `Tanach with Ta'amei Hamikra`.
- Upstream source: tanach.us.
- License asserted by the exported version: Public Domain.

### Mishnayot
- All six Sedarim.
- 63 masechtot, including Pirkei Avot.
- Navigation: Seder → Masechet → Perek.
- Hebrew source: Sefaria export version `Torat Emet 357`.
- License asserted by each exported version: Public Domain.

### Shnayim Mikrah Echad Targum
- All 54 weekly parashot.
- Every Torah verse is emitted twice, followed by the corresponding Onkelos verse once.
- Torah source: `Tanach with Ta'amei Hamikra`, Public Domain.
- Targum source: Sefaria versions `Onkelos Genesis` through `Onkelos Deuteronomy`, Public Domain.
- Weekly parashah boundaries: `hebcal/hebcal-leyning` `src/aliyot.json`, BSD-2-Clause.

## Source integrity

`tools/jewish_texts/build_pack.py` downloads only exact version titles from the official Sefaria Export index. It refuses a source whose exported license is not Public Domain/PD. It validates:

- 150 Tehillim chapters.
- 63 Mishnah tractates.
- Torah/Onkelos chapter and verse alignment.
- 54 Hebcal weekly parashot.
- Every Shnayim verse has both a Torah and Onkelos value.

The generated `SOURCES.json` records the URL, version title, license, upstream source and SHA-256 of every downloaded source file.

## microSD layout

The text corpus is deliberately kept off the ESP32 firmware. The generated pack is approximately 7.6 MB uncompressed and is read from microSD.

Unzip `x3-jewish-texts.zip` and copy the resulting folder to:

```text
/jewish-texts/
```

The app checks for `/jewish-texts/SOURCES.json` when it opens.

## Reader

The Home screen entry is **Jewish Texts**.

Top level:
1. Tehillim
2. Mishnayot
3. Shnayim Mikrah Echad Targum

Reading is offline. The firmware does not require zmanim or a network connection.
