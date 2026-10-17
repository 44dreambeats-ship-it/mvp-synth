#pragma once
// MVP PresetManager — JSON-first, no-issues design.
// JSON is the native format. .fxp / .fxb are supported ONLY as import:
// on drag-drop they are converted to JSON and stored as JSON.
// Internal engine + browser only ever deal with JSON, so there is one code path.
// No JUCE dependency — testable on Linux/Mac with std::filesystem.
// Layout (user-visible, drag-drop target) — ONE presets folder, by type:
//   ~/Documents/MVP/Presets/   (bank name: "Presets")
//     Bass/ Synth/ Pluck/ Pad/ Keys/ Cinematic/ Arp/ FX/
//     Factory/            (single-file bank MVP_Presets_2000.json, read-only)
//     My Banks/           (your own banks: <Bank>/bank.json + preset .json files)
// Saving: atomic write (temp + rename), never overwrites Factory, unique filenames.
// Banks: a bank is a folder. Create bank = create folder + manifest. Add preset = copy file in.
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

struct PresetInfo {
    std::string name;
    std::string category = "Lead";
    std::string bank = "";      // bank folder name, or "" for My Presets
    std::string filePath;       // full path
    std::string format = "json"; // json (fxp/fxb are converted on import)
    std::string json;           // raw preset JSON object (single preset)
    bool isFactory = false;
    bool isValid = false;
};

struct BankInfo {
    std::string name;
    std::string folderPath;
    int presetCount = 0;
    bool isFactory = false;
};

class PresetManager {
public:
    // Init with user root (~/Documents/MVP/Presets) and factory root (bundled).
    // Creates folders if missing. Returns false only if folders cannot be created.
    bool init(const fs::path& userRoot, const fs::path& factoryRoot = {});

    fs::path userRoot() const { return userRoot_; }
    fs::path factoryRoot() const { return factoryRoot_; }
    fs::path myPresetsFolder() const { return userRoot_ / "My Presets"; }
    fs::path myBanksFolder() const { return userRoot_ / "My Banks"; }
    fs::path factoryFolder() const { return userRoot_ / "Factory"; }

    // Scan everything (Factory + User). Tolerant: bad files are skipped, never throws.
    std::vector<PresetInfo> scan();
    std::vector<PresetInfo> presets() const { return cache_; }
    std::vector<BankInfo> banks() const;
    std::vector<PresetInfo> presetsInBank(const std::string& bankName) const;
    std::vector<PresetInfo> search(const std::string& query) const; // name/category/bank substring, case-insensitive

    // --- Drag & drop / Import ---
    // Accepts files or folders dropped from Finder / DAW. Copies into User folder:
    //  .json single preset -> My Presets/
    //  .json bank (has "presets": [...]) -> My Banks/<BankName>/ + split presets
    //  .fxp -> converted to JSON -> My Presets/
    //  .fxb -> converted to JSON bank -> My Banks/<BankName>/
    //  folder -> recursively imported
    // Returns number of presets imported. Never crashes on bad files.
    int importFiles(const std::vector<fs::path>& dropped);

    // --- Saving custom presets ---
    // Save current synth state as JSON. json must be a single-preset JSON object.
    // If bankName empty -> My Presets/<Name>.json. Else -> My Banks/<bankName>/<Name>.json
    // (creates bank if needed). Returns path written, or "" on failure.
    std::string savePreset(const std::string& name, const std::string& category,
                           const std::string& presetJson, const std::string& bankName = "");

    // --- Banks ---
    // Create an empty bank folder + manifest. Returns false if bank already exists.
    bool createBank(const std::string& bankName);
    // Add an existing preset JSON to a bank (copies it in). Creates bank if needed.
    bool addPresetToBank(const std::string& bankName, const std::string& presetName,
                         const std::string& presetJson);
    // Build a bank from a list of preset JSON objects. Returns folder path or "".
    std::string createBankFromPresets(const std::string& bankName,
                                      const std::vector<std::string>& presetJsons);
    bool deleteBank(const std::string& bankName);   // User banks only, never Factory
    bool deletePreset(const std::string& filePath); // User presets only, never Factory
    bool renamePreset(const std::string& filePath, const std::string& newName);

    // --- Low-level helpers (also used by tests) ---
    static std::string sanitizeFileName(const std::string& name);
    static std::string extractJsonString(const std::string& json, const std::string& key);
    static bool isBankJson(const std::string& json); // true if contains "presets": [
    static std::string bankNameFromJson(const std::string& json);
    static std::vector<std::string> splitBankJson(const std::string& bankJson); // preset objects
    static std::string wrapAsBankJson(const std::string& bankName,
                                      const std::vector<std::string>& presetJsons);
    static std::string makePresetJson(const std::string& name, const std::string& category,
                                       const std::string& bodyJson);
    // .fxp/.fxb -> JSON conversion (import path only)
    static bool fxpToJson(const fs::path& fxpPath, std::string& outJson);
    static bool fxbToBankJson(const fs::path& fxbPath, std::string& outBankJson, std::string& outBankName);
    static bool jsonToFxp(const std::string& presetJson, const fs::path& outPath); // export only, optional
    static std::string readFile(const fs::path& p);
    static bool writeFileAtomic(const fs::path& p, const std::string& content);

private:
    fs::path userRoot_;
    fs::path factoryRoot_;
    std::vector<PresetInfo> cache_;

    void scanFolder(const fs::path& folder, const std::string& bank, bool isFactory,
                    std::vector<PresetInfo>& out);
    PresetInfo parseSinglePresetFile(const fs::path& p, const std::string& bank, bool isFactory);
    int importSingleFile(const fs::path& p);
    int importBankJsonFile(const fs::path& p, const std::string& bankJson);
    std::string uniquePath(const fs::path& dir, const std::string& baseName, const std::string& ext);
};
