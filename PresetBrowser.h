#pragma once
// MVP Preset Browser — JUCE UI for the drag-drop Presets folder.
// Only compiled in the Mac JUCE build. PresetManager itself has no JUCE dep.
// Layout (locked 2nd look: off-black + acid lime + hot red, tall sidebar):
//   [Search ...] [Rescan] [Open Folder] [Import]
//   [Banks list] | [Presets list]
//   [Save Preset...] [New Bank...] [Save as Bank...]
// Drag & drop: drop .json/.fxp/.fxb files or folders anywhere on this component.
// Two separate ListBoxModels (BankModel / PresetModel) — no shared-model ambiguity.
#ifdef __APPLE__
#include <JuceHeader.h>
#include "PresetManager.h"
#include <functional>
#include <string>
#include <vector>

class PresetBrowser : public juce::Component,
                      public juce::FileDragAndDropTarget,
                      public juce::Button::Listener,
                      public juce::TextEditor::Listener
{
public:
    explicit PresetBrowser(PresetManager& pm);
    ~PresetBrowser() override = default;

    void resized() override;
    void paint(juce::Graphics& g) override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;

    void buttonClicked(juce::Button* b) override;
    void textEditorTextChanged(juce::TextEditor&) override;

    void setCurrentPresetJson(const std::string& json) { currentJson_ = json; }
    std::function<void(const std::string& presetJson)> onPresetSelected;

private:
    struct BankModel : public juce::ListBoxModel {
        PresetBrowser* owner = nullptr;
        int getNumRows() override;
        void paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool selected) override;
        void listBoxItemClicked(int row, const juce::MouseEvent&) override;
    };
    struct PresetModel : public juce::ListBoxModel {
        PresetBrowser* owner = nullptr;
        int getNumRows() override;
        void paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool selected) override;
        void listBoxItemClicked(int row, const juce::MouseEvent&) override;
    };

    PresetManager& pm_;
    std::string currentJson_;
    bool dragHover_ = false;

    juce::TextEditor searchBox_;
    BankModel bankModel_;
    PresetModel presetModel_;
    juce::ListBox bankList_{ "Banks", &bankModel_ };
    juce::ListBox presetList_{ "Presets", &presetModel_ };
    juce::TextButton rescanBtn_{ "Rescan" };
    juce::TextButton openFolderBtn_{ "Open Folder" };
    juce::TextButton importBtn_{ "Import…" };
    juce::TextButton saveBtn_{ "Save Preset…" };
    juce::TextButton newBankBtn_{ "New Bank…" };
    juce::TextButton saveBankBtn_{ "Save as Bank…" };
    juce::Label statusLabel_;

    std::string selectedBank_; // "" = All / Presets
    std::vector<PresetInfo> shownPresets_;
    std::vector<BankInfo> shownBanks_;

    void refresh();
    void refreshPresets();
    void showSaveDialog();
    void showNewBankDialog();
    void setStatus(const juce::String& s);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowser)
};
#endif
