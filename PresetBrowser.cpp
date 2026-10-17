#ifdef __APPLE__
#include "PresetBrowser.h"
#include <algorithm>

static const juce::Colour kBg(0xff0e0f0c);
static const juce::Colour kLime(0xffb6ff2e);
static const juce::Colour kText(0xfff2f2f2);

// --- Bank model ---
int PresetBrowser::BankModel::getNumRows() { return (int)owner->shownBanks_.size(); }
void PresetBrowser::BankModel::paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool selected) {
    if (row < 0 || row >= (int)owner->shownBanks_.size()) return;
    auto& b = owner->shownBanks_[(size_t)row];
    g.fillAll(selected ? kLime : kBg);
    g.setColour(selected ? kBg : kText);
    g.setFont(13.0f);
    g.drawText(juce::String(b.name) + " (" + juce::String(b.presetCount) + ")", 6, 0, w - 12, h, juce::Justification::centredLeft);
}
void PresetBrowser::BankModel::listBoxItemClicked(int row, const juce::MouseEvent&) {
    if (row < 0 || row >= (int)owner->shownBanks_.size()) return;
    owner->selectedBank_ = owner->shownBanks_[(size_t)row].name;
    // "MVP Presets" is the single factory bank — selecting it shows all factory presets.
    // Selecting "All" (if present) clears filter.
    if (owner->selectedBank_ == "All") owner->selectedBank_.clear();
    owner->refreshPresets();
}

// --- Preset model ---
int PresetBrowser::PresetModel::getNumRows() { return (int)owner->shownPresets_.size(); }
void PresetBrowser::PresetModel::paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool selected) {
    if (row < 0 || row >= (int)owner->shownPresets_.size()) return;
    auto& p = owner->shownPresets_[(size_t)row];
    g.fillAll(selected ? kLime : kBg);
    g.setColour(selected ? kBg : kText);
    g.setFont(13.0f);
    g.drawText(juce::String(p.name) + " — " + juce::String(p.category), 6, 0, w - 12, h, juce::Justification::centredLeft);
}
void PresetBrowser::PresetModel::listBoxItemClicked(int row, const juce::MouseEvent&) {
    if (row < 0 || row >= (int)owner->shownPresets_.size()) return;
    if (owner->onPresetSelected) owner->onPresetSelected(owner->shownPresets_[(size_t)row].json);
}

PresetBrowser::PresetBrowser(PresetManager& pm) : pm_(pm) {
    bankModel_.owner = this;
    presetModel_.owner = this;
    setOpaque(true);
    searchBox_.setTextToShowWhenEmpty("Search presets…", kText.withAlpha(0.4f));
    searchBox_.addListener(this);
    addAndMakeVisible(searchBox_);
    for (auto* b : { (juce::Button*)&rescanBtn_, (juce::Button*)&openFolderBtn_, (juce::Button*)&importBtn_,
                    (juce::Button*)&saveBtn_, (juce::Button*)&newBankBtn_, (juce::Button*)&saveBankBtn_ }) {
        b->addListener(this);
        addAndMakeVisible(b);
    }
    bankList_.setColour(juce::ListBox::backgroundColourId, kBg);
    presetList_.setColour(juce::ListBox::backgroundColourId, kBg);
    addAndMakeVisible(bankList_);
    addAndMakeVisible(presetList_);
    statusLabel_.setColour(juce::Label::textColourId, kText);
    statusLabel_.setText("Drop .json / .fxp / .fxb presets or banks anywhere here.", juce::dontSendNotification);
    addAndMakeVisible(statusLabel_);
    refresh();
}

void PresetBrowser::paint(juce::Graphics& g) {
    g.fillAll(kBg);
    if (dragHover_) {
        g.setColour(kLime);
        g.drawRect(getLocalBounds(), 3);
        g.setFont(16.0f);
        g.drawText("Drop to import presets / banks", getLocalBounds(), juce::Justification::centred);
    }
}
void PresetBrowser::resized() {
    auto r = getLocalBounds().reduced(8);
    auto top = r.removeFromTop(28);
    searchBox_.setBounds(top.removeFromLeft(220));
    rescanBtn_.setBounds(top.removeFromLeft(70).reduced(2));
    openFolderBtn_.setBounds(top.removeFromLeft(100).reduced(2));
    importBtn_.setBounds(top.removeFromLeft(80).reduced(2));
    auto mid = r; mid.removeFromBottom(36); mid.removeFromBottom(4);
    bankList_.setBounds(mid.removeFromLeft(180));
    mid.removeFromLeft(8);
    presetList_.setBounds(mid);
    auto bot = r.removeFromBottom(32);
    saveBtn_.setBounds(bot.removeFromLeft(120).reduced(2));
    newBankBtn_.setBounds(bot.removeFromLeft(110).reduced(2));
    saveBankBtn_.setBounds(bot.removeFromLeft(130).reduced(2));
    statusLabel_.setBounds(bot);
}

bool PresetBrowser::isInterestedInFileDrag(const juce::StringArray& files) {
    for (auto& f : files) {
        juce::File file(f);
        if (file.isDirectory()) return true;
        auto ext = file.getFileExtension().toLowerCase();
        if (ext == ".json" || ext == ".fxp" || ext == ".fxb") return true;
    }
    return false;
}
void PresetBrowser::fileDragEnter(const juce::StringArray&, int, int) { dragHover_ = true; repaint(); }
void PresetBrowser::fileDragExit(const juce::StringArray&) { dragHover_ = false; repaint(); }
void PresetBrowser::filesDropped(const juce::StringArray& files, int, int) {
    dragHover_ = false; repaint();
    std::vector<std::filesystem::path> paths;
    for (auto& f : files) paths.emplace_back(f.toStdString());
    int n = pm_.importFiles(paths);
    setStatus("Imported " + juce::String(n) + " preset(s).");
    refresh();
}

void PresetBrowser::buttonClicked(juce::Button* b) {
    if (b == &rescanBtn_) { pm_.scan(); refresh(); setStatus("Rescanned."); }
    else if (b == &openFolderBtn_) {
        juce::File f(pm_.userRoot().string());
        f.revealToUser();
    }
    else if (b == &importBtn_) {
        juce::FileChooser chooser("Import presets / banks", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
                                  "*.json;*.fxp;*.fxb");
        if (chooser.browseForMultipleFilesToOpen()) {
            std::vector<std::filesystem::path> paths;
            for (auto& f : chooser.getResults()) paths.emplace_back(f.getFullPathName().toStdString());
            int n = pm_.importFiles(paths);
            setStatus("Imported " + juce::String(n) + " preset(s).");
            refresh();
        }
    }
    else if (b == &saveBtn_) showSaveDialog();
    else if (b == &newBankBtn_) showNewBankDialog();
    else if (b == &saveBankBtn_) {
        juce::AlertWindow w("Save as Bank", "Name for the new bank:", juce::AlertWindow::QuestionIcon);
        w.addTextEditor("bank", "");
        w.addButton("Create", 1, juce::KeyPress(juce::KeyPress::returnKey));
        w.addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
        if (w.runModalLoop() == 1) {
            auto name = w.getTextEditorContents("bank").trim();
            if (name.isNotEmpty()) {
                std::vector<std::string> jsons;
                for (auto& p : shownPresets_) jsons.push_back(p.json);
                auto path = pm_.createBankFromPresets(name.toStdString(), jsons);
                setStatus(path.empty() ? "Could not create bank." : ("Bank created: " + name));
                refresh();
            }
        }
    }
}

void PresetBrowser::textEditorTextChanged(juce::TextEditor&) { refreshPresets(); }

void PresetBrowser::refresh() {
    pm_.scan();
    shownBanks_ = pm_.banks();
    refreshPresets();
    bankList_.updateContent(); presetList_.updateContent();
}
void PresetBrowser::refreshPresets() {
    auto q = searchBox_.getText().toStdString();
    auto all = q.empty() ? pm_.presets() : pm_.search(q);
    if (!selectedBank_.empty()) {
        std::vector<PresetInfo> f;
        for (auto& p : all) if (p.bank == selectedBank_) f.push_back(p);
        shownPresets_ = f;
    } else shownPresets_ = all;
    presetList_.updateContent();
}
void PresetBrowser::showSaveDialog() {
    juce::AlertWindow w("Save Preset", "Name your preset:", juce::AlertWindow::QuestionIcon);
    w.addTextEditor("name", "");
    w.addTextEditor("bank", "");
    w.getTextEditor("bank")->setTextToShowWhenEmpty("Bank (optional — leave blank for My Presets)", juce::Colours::grey);
    w.addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    w.addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    if (w.runModalLoop() == 1) {
        auto name = w.getTextEditorContents("name").trim();
        auto bank = w.getTextEditorContents("bank").trim();
        if (name.isEmpty()) { setStatus("Give the preset a name first."); return; }
        if (currentJson_.empty()) { setStatus("No current sound to save — tweak a knob first."); return; }
        auto path = pm_.savePreset(name.toStdString(), "Lead", currentJson_, bank.toStdString());
        setStatus(path.empty() ? "Could not save preset." : ("Saved: " + name));
        refresh();
    }
}
void PresetBrowser::showNewBankDialog() {
    juce::AlertWindow w("New Bank", "Name for the new bank:", juce::AlertWindow::QuestionIcon);
    w.addTextEditor("bank", "");
    w.addButton("Create", 1, juce::KeyPress(juce::KeyPress::returnKey));
    w.addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    if (w.runModalLoop() == 1) {
        auto name = w.getTextEditorContents("bank").trim();
        if (name.isEmpty()) return;
        bool ok = pm_.createBank(name.toStdString());
        setStatus(ok ? ("Bank created: " + name) : "That bank already exists.");
        refresh();
    }
}
void PresetBrowser::setStatus(const juce::String& s) { statusLabel_.setText(s, juce::dontSendNotification); }
#endif
