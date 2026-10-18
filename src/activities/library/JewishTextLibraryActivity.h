#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "activities/Activity.h"

class JewishTextLibraryActivity final : public Activity {
 public:
  enum class View {
    Root,
    Tehillim,
    MishnahSedarim,
    MishnahTractates,
    MishnahChapters,
    Shnayim,
    Reading,
    MissingPack,
  };

 private:
  struct Entry {
    std::string label;
    std::string secondary;
    std::string path;
    int number = 0;
    int count = 0;
  };

  View view = View::Root;
  View returnView = View::Root;
  std::vector<Entry> entries;
  std::size_t selected = 0;

  std::string selectedSederFolder;
  std::string selectedTractateFolder;
  std::string selectedTractateName;

  std::string readerTitle;
  std::string readerSubtitle;
  std::string readerText;
  std::size_t textOffset = 0;
  std::size_t nextTextOffset = 0;
  std::size_t textPageIndex = 0;
  bool hasNextTextPage = false;
  bool cleanRefreshPending = true;

  static constexpr const char* kContentRoot = "/jewish-texts";

  void showRoot();
  void showTehillim();
  void showMishnahSedarim();
  bool showMishnahTractates(const std::string& sederFolder);
  void showMishnahChapters(const Entry& tractate);
  bool showShnayim();

  bool loadIndex(const std::string& path, std::vector<std::vector<std::string>>& rows) const;
  bool loadReaderFile(const std::string& path);
  void openReader(const std::string& title, const std::string& subtitle, const std::string& path, View backTo);
  void activateSelected();
  void goBack();

  void resetTextPage();
  void previousTextPage();
  void nextTextPage();

  void renderList();
  void renderReader();

 public:
  explicit JewishTextLibraryActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("JewishTextLibrary", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
