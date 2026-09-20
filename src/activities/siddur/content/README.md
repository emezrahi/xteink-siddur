# Edot HaMizrach content

The initial liturgical text source for the X3 Siddur project is Sefaria's **Siddur Edot HaMizrach, Shaliehsaboo Edition**.

- Source: Sefaria
- Edition: Shaliehsaboo Edition
- License: CC0
- Initial implemented Shaharit blocks: `על נטילת ידים`, `אשר יצר`, `אלהי נשמה`

The content layer is intentionally kept separate from UI/activity code so later phases can replace static compiled blocks with the date-aware composer without rewriting the reader screen.

Syrian / Aram Soba-specific variants are not included yet. They will be added as a separate nusach/minhag layer after the Edot HaMizrach base is working.


## Phase 5 Amidah rule test

The content set now also includes the Retzeh opening, the Rosh Hodesh Yaaleh VeYavo insertion, the Retzeh conclusion, and Modim from the same Edot HaMizrach source.

These blocks are deliberately split at the insertion boundary so the composer can include or omit Yaaleh VeYavo without hardcoding liturgical logic in the UI or duplicating the surrounding blessing.


## Phase 10 weekday corpus

`EdotWeekdayShaharit.h` contains the ordinary weekday Shaharit corpus used by the device reader. The text is generated from the same Sefaria Shaliehsaboo Edition, whose source metadata declares CC0.

The corpus is split into major prayer sections rather than one large document so `Composer` can later insert, omit, or substitute sections based on the resolved prayer context. Smart Siddur supplies the behavioral section/rule reference; its decompiled code and application assets are not included.

The first weekday baseline includes the morning preparation, korbanot/ketoret, Hodu, Pesukei DeZimra, Shema and its blessings, weekday Amidah, Tahanun, the Monday/Thursday Torah-service frame, Ashrei/Uva LeSion, the weekday-specific Shir Shel Yom, Kaveh, and Aleinu.

Special-day substitutions and the dynamic Torah-reading text remain separate rule slices.
