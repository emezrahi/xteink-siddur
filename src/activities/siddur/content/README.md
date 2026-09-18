# Edot HaMizrach content

The initial liturgical text source for the X3 Siddur project is Sefaria's **Siddur Edot HaMizrach, Shaliehsaboo Edition**.

- Source: Sefaria
- Edition: Shaliehsaboo Edition
- License: CC0
- Initial implemented block: Preparatory Prayers → Morning Blessings → `על נטילת ידים`

The content layer is intentionally kept separate from UI/activity code so later phases can replace static compiled blocks with the date-aware composer without rewriting the reader screen.

Syrian / Aram Soba-specific variants are not included yet. They will be added as a separate nusach/minhag layer after the Edot HaMizrach base is working.
