#pragma once

#include <SiddurEngine/PrayerBlockId.h>

#include <algorithm>
#include <iterator>

namespace SiddurContent::EdotHamizrach {

struct PrayerTextBlock {
  SiddurEngine::PrayerBlockId id;
  const char* sectionTitle;
  const char* title;
  const char* text;
};

// Source: Sefaria, Siddur Edot HaMizrach, Shaliehsaboo Edition (CC0).
inline constexpr char kShaharitTitle[] = "שחרית";
inline constexpr char kMorningBlessingsTitle[] = "ברכות השחר";

inline constexpr PrayerTextBlock kPrayerBlocks[] = {
    {
        SiddurEngine::PrayerBlockId::NetilatYadayim,
        kMorningBlessingsTitle,
        "על נטילת ידים",
        "בָּרוּךְ אַתָּה יְהוָֹה, אֱלֹהֵֽינוּ מֶֽלֶךְ הָעוֹלָם, "
        "אֲשֶׁר קִדְּשָׁנוּ בְּמִצְוֹתָיו וְצִוָּנוּ עַל נְטִילַת יָדָיִם:",
    },
    {
        SiddurEngine::PrayerBlockId::AsherYatzar,
        kMorningBlessingsTitle,
        "אשר יצר",
        "בָּרוּךְ אַתָּה יְהוָֹה, אֱלֹהֵֽינוּ מֶֽלֶךְ הָעוֹלָם, "
        "אֲשֶׁר יָצַר אֶת הָאָדָם בְּחָכְמָה, וּבָרָא בוֹ נְקָבִים נְקָבִים, "
        "חֲלוּלִים חֲלוּלִים, גָּלוּי וְיָדוּעַ לִפְנֵי כִסֵּא כְבוֹדֶךָ, "
        "שֶׁאִם יִסָּתֵם אֶחָד מֵהֶם, אוֹ אִם יִפָּתֵח אֶחָד מֵהֶם, "
        "אֵי אֶפְשָׁר לְהִתְקַיֵּם אַפִלּוּ שָׁעָה אֶחָת. "
        "בָּרוּךְ אַתָּה יְהֹוָה, רוֹפֵא כָל־בָּשָׂר וּמַפְלִיא לַעֲשׂוֹת:",
    },
    {
        SiddurEngine::PrayerBlockId::ElohaiNeshama,
        kMorningBlessingsTitle,
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

[[nodiscard]] inline const PrayerTextBlock* findBlock(const SiddurEngine::PrayerBlockId id) {
  const auto it = std::find_if(std::begin(kPrayerBlocks), std::end(kPrayerBlocks),
                               [id](const PrayerTextBlock& block) { return block.id == id; });
  return it == std::end(kPrayerBlocks) ? nullptr : &*it;
}

}  // namespace SiddurContent::EdotHamizrach
