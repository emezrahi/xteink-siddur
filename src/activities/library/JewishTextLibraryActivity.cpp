#include "JewishTextLibraryActivity.h"

#include <HalDisplay.h>
#include <HalStorage.h>
#include <JewishTextPager.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include "components/UITheme.h"
#include "fontIds.h"

namespace {
constexpr int kSideMargin = 24;
constexpr int kHeaderY = 24;
constexpr int kListTop = 78;
constexpr int kListRowHeight = 62;
constexpr int kListGap = 5;
constexpr int kReaderTop = 78;
constexpr int kReaderGap = 7;
constexpr std::size_t kMaxReaderBytes = 256 * 1024;

std::vector<std::string> splitTabs(const std::string& line) {
  std::vector<std::string> result;
  std::size_t start = 0;
  while (start <= line.size()) {
    const auto tab = line.find('\t', start);
    if (tab == std::string::npos) {
      result.emplace_back(line.substr(start));
      break;
    }
    result.emplace_back(line.substr(start, tab - start));
    start = tab + 1;
  }
  return result;
}

int parseInt(const std::string& value) {
  return std::atoi(value.c_str());
}

const char* viewTitle(const JewishTextLibraryActivity::View view) {
  switch (view) {
    case JewishTextLibraryActivity::View::Root: return "JEWISH TEXTS";
    case JewishTextLibraryActivity::View::Tehillim: return "TEHILLIM";
    case JewishTextLibraryActivity::View::MishnahSedarim: return "MISHNAYOT - SEDER";
    case JewishTextLibraryActivity::View::MishnahTractates: return "MISHNAYOT - MASECHET";
    case JewishTextLibraryActivity::View::MishnahChapters: return "MISHNAYOT - PEREK";
    case JewishTextLibraryActivity::View::Shnayim: return "SHNAYIM MIKRA";
    case JewishTextLibraryActivity::View::Reading:
    case JewishTextLibraryActivity::View::MissingPack:
      break;
  }
  return "";
}
}  // namespace

void JewishTextLibraryActivity::onEnter() {
  Activity::onEnter();
  cleanRefreshPending = true;
  if (!Storage.exists("/jewish-texts/SOURCES.json")) {
    view = View::MissingPack;
    requestUpdate();
    return;
  }
  showRoot();
}

void JewishTextLibraryActivity::showRoot() {
  view = View::Root;
  selected = 0;
  entries = {
      {"Tehillim", "150 chapters", "", 0, 0},
      {"Mishnayot", "6 sedarim / 63 masechtot", "", 0, 0},
      {"Shnayim Mikrah Echad Targum", "54 weekly parashot", "", 0, 0},
  };
  cleanRefreshPending = true;
  requestUpdate();
}

void JewishTextLibraryActivity::showTehillim() {
  view = View::Tehillim;
  selected = 0;
  entries.clear();
  entries.reserve(150);
  for (int chapter = 1; chapter <= 150; ++chapter) {
    char path[64];
    std::snprintf(path, sizeof(path), "%s/tehillim/%03d.txt", kContentRoot, chapter);
    entries.push_back({"Tehillim " + std::to_string(chapter), "", path, chapter, 0});
  }
  requestUpdate();
}

void JewishTextLibraryActivity::showMishnahSedarim() {
  view = View::MishnahSedarim;
  selected = 0;
  entries = {
      {"Zeraim", "זרעים", std::string(kContentRoot) + "/mishnah/01-zeraim", 1, 0},
      {"Moed", "מועד", std::string(kContentRoot) + "/mishnah/02-moed", 2, 0},
      {"Nashim", "נשים", std::string(kContentRoot) + "/mishnah/03-nashim", 3, 0},
      {"Nezikin", "נזיקין", std::string(kContentRoot) + "/mishnah/04-nezikin", 4, 0},
      {"Kodashim", "קדשים", std::string(kContentRoot) + "/mishnah/05-kodashim", 5, 0},
      {"Tahorot", "טהרות", std::string(kContentRoot) + "/mishnah/06-tahorot", 6, 0},
  };
  requestUpdate();
}

bool JewishTextLibraryActivity::loadIndex(const std::string& path,
                                          std::vector<std::vector<std::string>>& rows) const {
  rows.clear();
  const String raw = Storage.readFile(path.c_str());
  if (raw.length() == 0) return false;
  const std::string data(raw.c_str(), raw.length());

  std::size_t pos = 0;
  while (pos < data.size()) {
    const auto end = data.find('\n', pos);
    const auto lineEnd = end == std::string::npos ? data.size() : end;
    if (lineEnd > pos) {
      auto row = splitTabs(data.substr(pos, lineEnd - pos));
      if (!row.empty()) rows.push_back(std::move(row));
    }
    if (end == std::string::npos) break;
    pos = end + 1;
  }
  return !rows.empty();
}

bool JewishTextLibraryActivity::showMishnahTractates(const std::string& sederFolder) {
  std::vector<std::vector<std::string>> rows;
  if (!loadIndex(sederFolder + "/index.tsv", rows)) return false;

  selectedSederFolder = sederFolder;
  entries.clear();
  for (const auto& row : rows) {
    if (row.size() < 4) continue;
    Entry entry;
    entry.label = row[1];
    entry.secondary = row[2];
    entry.path = sederFolder + "/" + row[0];
    entry.count = parseInt(row[3]);
    entries.push_back(std::move(entry));
  }
  if (entries.empty()) return false;
  view = View::MishnahTractates;
  selected = 0;
  requestUpdate();
  return true;
}

void JewishTextLibraryActivity::showMishnahChapters(const Entry& tractate) {
  selectedTractateFolder = tractate.path;
  selectedTractateName = tractate.label;
  entries.clear();
  entries.reserve(static_cast<std::size_t>(std::max(tractate.count, 0)));
  for (int chapter = 1; chapter <= tractate.count; ++chapter) {
    char file[24];
    std::snprintf(file, sizeof(file), "/%02d.txt", chapter);
    entries.push_back({"Perek " + std::to_string(chapter), "", tractate.path + file, chapter, 0});
  }
  view = View::MishnahChapters;
  selected = 0;
  requestUpdate();
}

bool JewishTextLibraryActivity::showShnayim() {
  std::vector<std::vector<std::string>> rows;
  if (!loadIndex(std::string(kContentRoot) + "/shnayim/index.tsv", rows)) return false;

  entries.clear();
  for (const auto& row : rows) {
    if (row.size() < 7) continue;
    Entry entry;
    entry.number = parseInt(row[0]);
    entry.label = row[1];
    entry.path = std::string(kContentRoot) + "/shnayim/" + row[2];
    entry.secondary = row[3] + " " + row[4] + "-" + row[5];
    entry.count = parseInt(row[6]);
    entries.push_back(std::move(entry));
  }
  if (entries.empty()) return false;
  view = View::Shnayim;
  selected = 0;
  requestUpdate();
  return true;
}

bool JewishTextLibraryActivity::loadReaderFile(const std::string& path) {
  HalFile file;
  if (!Storage.openFileForRead("JTXT", path, file)) return false;
  const auto bytes = file.fileSize();
  if (bytes == 0 || bytes > kMaxReaderBytes) {
    file.close();
    return false;
  }

  readerText.assign(bytes, '\0');
  std::size_t total = 0;
  while (total < bytes) {
    const int read = file.read(readerText.data() + total, bytes - total);
    if (read <= 0) break;
    total += static_cast<std::size_t>(read);
  }
  file.close();
  readerText.resize(total);
  return !readerText.empty();
}

void JewishTextLibraryActivity::openReader(const std::string& title, const std::string& subtitle,
                                           const std::string& path, const View backTo) {
  if (!loadReaderFile(path)) {
    readerTitle = "Unable to open text";
    readerSubtitle = path;
    readerText = "The selected text file is missing or invalid. Re-copy the jewish-texts folder to the microSD card.";
  } else {
    readerTitle = title;
    readerSubtitle = subtitle;
  }
  returnView = backTo;
  resetTextPage();
  view = View::Reading;
  cleanRefreshPending = true;
  requestUpdate();
}

void JewishTextLibraryActivity::activateSelected() {
  if (entries.empty() || selected >= entries.size()) return;
  const auto entry = entries[selected];

  switch (view) {
    case View::Root:
      if (selected == 0) {
        showTehillim();
      } else if (selected == 1) {
        showMishnahSedarim();
      } else if (selected == 2) {
        if (!showShnayim()) {
          view = View::MissingPack;
          requestUpdate();
        }
      }
      break;

    case View::Tehillim:
      openReader(entry.label, "", entry.path, View::Tehillim);
      break;

    case View::MishnahSedarim:
      if (!showMishnahTractates(entry.path)) {
        view = View::MissingPack;
        requestUpdate();
      }
      break;

    case View::MishnahTractates:
      showMishnahChapters(entry);
      break;

    case View::MishnahChapters:
      openReader(selectedTractateName + " - " + entry.label, "", entry.path, View::MishnahChapters);
      break;

    case View::Shnayim:
      openReader(entry.label, "Shnayim Mikrah Echad Targum", entry.path, View::Shnayim);
      break;

    case View::Reading:
    case View::MissingPack:
      break;
  }
}

void JewishTextLibraryActivity::goBack() {
  switch (view) {
    case View::Root:
    case View::MissingPack:
      finish();
      return;
    case View::Tehillim:
    case View::MishnahSedarim:
    case View::Shnayim:
      showRoot();
      return;
    case View::MishnahTractates:
      showMishnahSedarim();
      return;
    case View::MishnahChapters:
      if (!showMishnahTractates(selectedSederFolder)) showMishnahSedarim();
      return;
    case View::Reading:
      view = returnView;
      cleanRefreshPending = true;
      requestUpdate();
      return;
  }
}

void JewishTextLibraryActivity::resetTextPage() {
  textOffset = 0;
  nextTextOffset = 0;
  textPageIndex = 0;
  hasNextTextPage = false;
}

void JewishTextLibraryActivity::previousTextPage() {
  if (textOffset == 0 || readerText.empty()) return;
  const auto safe = UITheme::getInstance().getScreenSafeArea(renderer, true, false);
  const int pageLabelTop = safe.y + safe.height - renderer.getTextHeight(UI_10_FONT_ID) - 12;
  const int glyphHeight = renderer.getTextHeight(SIDDUR_HEBREW_16_FONT_ID);
  const int advance = std::max(renderer.getLineHeight(SIDDUR_HEBREW_16_FONT_ID), glyphHeight) + kReaderGap;
  const int lines = JewishTextLibrary::PageGeometry::visibleLines(safe.y + kReaderTop, pageLabelTop - 14,
                                                                  glyphHeight, advance);
  const int width = renderer.getScreenWidth() - kSideMargin * 2;
  auto measure = [this](const char* text) {
    return renderer.getTextWidth(SIDDUR_HEBREW_16_FONT_ID, text, EpdFontFamily::REGULAR,
                                 BidiUtils::BidiBaseDir::RTL);
  };
  textOffset = JewishTextLibrary::Utf8Pager::previousOffset(readerText, textOffset, width, lines, measure);
  if (textPageIndex > 0) --textPageIndex;
  requestUpdate();
}

void JewishTextLibraryActivity::nextTextPage() {
  if (!hasNextTextPage || nextTextOffset <= textOffset) return;
  textOffset = nextTextOffset;
  ++textPageIndex;
  requestUpdate();
}

void JewishTextLibraryActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    goBack();
    return;
  }

  if (view == View::Reading) {
    if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
      goBack();
      return;
    }
    if (mappedInput.wasReleased(MappedInputManager::Button::NavPrevious)) {
      previousTextPage();
      return;
    }
    if (mappedInput.wasReleased(MappedInputManager::Button::NavNext)) {
      nextTextPage();
      return;
    }
    return;
  }

  if (view == View::MissingPack) return;
  if (entries.empty()) return;

  if (mappedInput.wasReleased(MappedInputManager::Button::NavPrevious)) {
    if (selected > 0) --selected;
    requestUpdate();
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::NavNext)) {
    if (selected + 1 < entries.size()) ++selected;
    requestUpdate();
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) activateSelected();
}

void JewishTextLibraryActivity::renderList() {
  const auto safe = UITheme::getInstance().getScreenSafeArea(renderer, true, false);
  const int width = renderer.getScreenWidth() - kSideMargin * 2;
  renderer.drawCenteredText(UI_12_FONT_ID, safe.y + kHeaderY, viewTitle(view), true, EpdFontFamily::BOLD);

  if (entries.empty()) {
    renderer.drawCenteredText(UI_10_FONT_ID, safe.y + 180, "No entries found");
  } else {
    const int pitch = kListRowHeight + kListGap;
    const int availableBottom = safe.y + safe.height - 34;
    const int visible = std::max(1, (availableBottom - (safe.y + kListTop)) / pitch);
    const std::size_t first =
        (selected / static_cast<std::size_t>(visible)) * static_cast<std::size_t>(visible);

    for (int row = 0; row < visible && first + static_cast<std::size_t>(row) < entries.size(); ++row) {
      const std::size_t index = first + static_cast<std::size_t>(row);
      const int y = safe.y + kListTop + row * pitch;
      const bool active = index == selected;
      renderer.fillRect(kSideMargin, y, width, kListRowHeight, active);
      renderer.drawRect(kSideMargin, y, width, kListRowHeight);

      const auto title = renderer.truncatedText(UI_10_FONT_ID, entries[index].label.c_str(), width - 24,
                                                EpdFontFamily::BOLD);
      renderer.drawText(UI_10_FONT_ID, kSideMargin + 12, y + 10, title.c_str(), !active, EpdFontFamily::BOLD);
      if (!entries[index].secondary.empty()) {
        const auto secondary =
            renderer.truncatedText(UI_10_FONT_ID, entries[index].secondary.c_str(), width - 24);
        renderer.drawText(UI_10_FONT_ID, kSideMargin + 12, y + 34, secondary.c_str(), !active);
      }
    }

    char position[32];
    std::snprintf(position, sizeof(position), "%u / %u", static_cast<unsigned>(selected + 1),
                  static_cast<unsigned>(entries.size()));
    renderer.drawCenteredText(UI_10_FONT_ID, safe.y + safe.height - 28, position);
  }

  const auto labels = mappedInput.mapLabels("Back", "Open", "Up", "Down");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}

void JewishTextLibraryActivity::renderReader() {
  const auto safe = UITheme::getInstance().getScreenSafeArea(renderer, true, false);
  const int width = renderer.getScreenWidth() - kSideMargin * 2;

  const auto title = renderer.truncatedText(UI_10_FONT_ID, readerTitle.c_str(), width, EpdFontFamily::BOLD);
  renderer.drawCenteredText(UI_10_FONT_ID, safe.y + 14, title.c_str(), true, EpdFontFamily::BOLD);
  if (!readerSubtitle.empty()) {
    const auto subtitle = renderer.truncatedText(UI_10_FONT_ID, readerSubtitle.c_str(), width);
    renderer.drawCenteredText(UI_10_FONT_ID, safe.y + 39, subtitle.c_str());
  }

  const int bodyTop = safe.y + kReaderTop;
  const int pageLabelTop = safe.y + safe.height - renderer.getTextHeight(UI_10_FONT_ID) - 12;
  const int glyphHeight = renderer.getTextHeight(SIDDUR_HEBREW_16_FONT_ID);
  const int advance = std::max(renderer.getLineHeight(SIDDUR_HEBREW_16_FONT_ID), glyphHeight) + kReaderGap;
  const int lines =
      JewishTextLibrary::PageGeometry::visibleLines(bodyTop, pageLabelTop - 14, glyphHeight, advance);

  auto measure = [this](const char* text) {
    return renderer.getTextWidth(SIDDUR_HEBREW_16_FONT_ID, text, EpdFontFamily::REGULAR,
                                 BidiUtils::BidiBaseDir::RTL);
  };
  const auto page = JewishTextLibrary::Utf8Pager::paginate(readerText, textOffset, width, lines, measure);
  nextTextOffset = page.nextOffset;
  hasNextTextPage = page.hasNext;

  int y = bodyTop;
  for (const auto& line : page.lines) {
    if (!line.empty()) {
      const int lineWidth = renderer.getTextWidth(SIDDUR_HEBREW_16_FONT_ID, line.c_str(), EpdFontFamily::REGULAR,
                                                  BidiUtils::BidiBaseDir::RTL);
      renderer.drawText(SIDDUR_HEBREW_16_FONT_ID, renderer.getScreenWidth() - kSideMargin - lineWidth, y,
                        line.c_str(), true, EpdFontFamily::REGULAR, BidiUtils::BidiBaseDir::RTL);
    }
    y += advance;
  }

  char pageLabel[32];
  std::snprintf(pageLabel, sizeof(pageLabel), "page %u", static_cast<unsigned>(textPageIndex + 1));
  renderer.drawCenteredText(UI_10_FONT_ID, pageLabelTop, pageLabel);

  const auto labels = mappedInput.mapLabels("Back", "Chapters", "Previous", "Next");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}

void JewishTextLibraryActivity::render(RenderLock&&) {
  renderer.clearScreen();

  if (view == View::MissingPack) {
    const auto safe = UITheme::getInstance().getScreenSafeArea(renderer, true, false);
    renderer.drawCenteredText(UI_12_FONT_ID, safe.y + 60, "JEWISH TEXTS", true, EpdFontFamily::BOLD);
    const Rect messageRect{kSideMargin, safe.y + 130, renderer.getScreenWidth() - kSideMargin * 2, 360};
    UITheme::drawCenteredWrappedText(renderer, messageRect, UI_10_FONT_ID,
                                     "Text pack not found. Copy the jewish-texts folder from "
                                     "x3-jewish-texts.zip to the root of the microSD card.",
                                     8);
    const auto labels = mappedInput.mapLabels("Back", "", "", "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  } else if (view == View::Reading) {
    renderReader();
  } else {
    renderList();
  }

  renderer.displayBuffer(cleanRefreshPending ? HalDisplay::HALF_REFRESH : HalDisplay::FAST_REFRESH);
  cleanRefreshPending = false;
}
