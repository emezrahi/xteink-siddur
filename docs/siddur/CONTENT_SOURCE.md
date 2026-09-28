# Prayer text provenance and outstanding APK comparison

The X3 currently uses **Sefaria's Siddur Edot HaMizrach, Shaliehsaboo Edition**, which the existing corpus records as CC0. Its compiled source is `src/activities/siddur/content/EdotWeekdayShaharit.h`. The chapter menu's observed English titles and approximate sequence use the Smart Siddur screenshot as a *reference*, not a text source.

**Do not describe the current corpus as the Smart Siddur APK's text.** The previously produced `SMART_SIDDUR_APK_AUDIT.zip` identified Smart Siddur 7.5.286 and candidate chapter titles, but did not establish an exact, reusable Edot HaMizrach prayer-text export or verify the complete dynamic chapter order. The original APK/text export is not present in this repository.

To satisfy an exact APK-content match:
1. Obtain an authorized original APK or full Edot HaMizrach prayer-text export, plus the permissions needed to redistribute any proprietary application text in a public project.
2. Compare individual chapters (starting with Shma and Putting on Talit), vocalization, paragraph breaks, and dynamic insertions against the current CC0 corpus.
3. Keep the clean-room boundary: never commit decompiled app code or proprietary assets. Where the APK wording is protected, use a separately licensed or public-domain matching source rather than treating the entire APK as open content.

The continuation-page fix in this branch only changes layout and preserves the existing prayer text unchanged.
