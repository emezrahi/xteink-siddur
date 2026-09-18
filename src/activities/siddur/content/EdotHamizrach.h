#pragma once

#include <cstddef>

namespace SiddurContent::EdotHamizrach {

struct PrayerBlock {
  const char* title;
  const char* text;
};

// Source: Sefaria, Siddur Edot HaMizrach, Shaliehsaboo Edition (CC0).
// Preparatory Prayers -> Morning Blessings.
inline constexpr char kShaharitTitle[] = "שחרית";
inline constexpr char kMorningBlessingsTitle[] = "ברכות השחר";

inline constexpr PrayerBlock kShaharitMorningBlessings[] = {
    {
        "על נטילת ידים",
        "בָּרוּךְ אַתָּה יְהוָֹה, אֱלֹהֵֽינוּ מֶֽלֶךְ הָעוֹלָם, "
        "אֲשֶׁר קִדְּשָׁנוּ בְּמִצְוֹתָיו וְצִוָּנוּ עַל נְטִילַת יָדָיִם:",
    },
    {
        "אשר יצר",
        "בָּרוּךְ אַתָּה יְהוָֹה, אֱלֹהֵֽינוּ מֶֽלֶךְ הָעוֹלָם, "
        "אֲשֶׁר יָצַר אֶת הָאָדָם בְּחָכְמָה, וּבָרָא בוֹ נְקָבִים נְקָבִים, "
        "חֲלוּלִים חֲלוּלִים, גָּלוּי וְיָדוּעַ לִפְנֵי כִסֵּא כְבוֹדֶךָ, "
        "שֶׁאִם יִסָּתֵם אֶחָד מֵהֶם, אוֹ אִם יִפָּתֵח אֶחָד מֵהֶם, "
        "אֵי אֶפְשָׁר לְהִתְקַיֵּם אַפִלּוּ שָׁעָה אֶחָת. "
        "בָּרוּךְ אַתָּה יְהֹוָה, רוֹפֵא כָל־בָּשָׂר וּמַפְלִיא לַעֲשׂוֹת:",
    },
    {
        "אלהי נשמה",
        "אֱלֹהַי, נְשָׁמָה שֶׁנָּתַתָּ בִּי טְהוֹרָה, אַתָּה בְרָאתָהּ, "
        "אַתָּה יְצַרְתָּהּ, אַתָּה נְפַחְתָּהּ בִּי, וְאַתָּה מְשַׁמְּרָהּ בְּקִרְבִּי, "
        "וְאַתָּה עָתִיד לִטְּלָהּ מִמֶּנִּי, וּלְהַחֲזִירָהּ בִּי לֶעָתִיד לָבוֹא, "
        "כָּל־זְמַן שֶׁהַנְּשָׁמָה בְקִרְבִּי, מוֹדֶה אֲנִי לְפָנֶיךָ "
        "יְהֹוָה אֱלֹהַי וֵאלֹהֵי אֲבוֹתַי, רִבּוֹן כָּל־הַמַּעֲשִׂים "
        "אֲדוֹן כָּל־הַנְּשָׁמוֹת. בָּרוּךְ אַתָּה יְהֹוָה, "
        "הַמַּחֲזִיר נְשָׁמוֹת לִפְגָרִים מֵתִים:",
    },
};

inline constexpr std::size_t kShaharitMorningBlessingsCount =
    sizeof(kShaharitMorningBlessings) / sizeof(kShaharitMorningBlessings[0]);

}  // namespace SiddurContent::EdotHamizrach
