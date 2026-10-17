# MVP Presets Folder — Implementation

JSON-first. One code path. No issues.

## Layout — ONE presets folder, organized by type
```
~/Documents/MVP/Presets/      (userRoot — created on first run)
  Bass/ 300, Synth/ 260, Pluck/ 230, Pad/ 320, Keys/ 320, Cinematic/ 300, Arp/ 200, FX/ 70
  Factory/                    (single-file bank MVP_Presets_2000.json, read-only)
  My Banks/<Bank>/            (your own banks: bank.json + preset .json files)
```
Bank name is just "Presets" — no Rage / Expanded banks. Type = preset "category".

## Code
- `Source/PresetManager.h/.cpp` — no JUCE dep, std::filesystem, testable.
  - `init(userRoot, factoryRoot)` — creates folders, copies factory banks if Factory/ empty.
  - `scan()` — tolerant, skips bad files, dedupes by (bank, name).
  - `importFiles(dropped)` — drag-drop: .json single/bank, .fxp→JSON, .fxb→JSON bank, folders recursive.
  - `savePreset(name, category, json, bankName)` — atomic write (temp + rename), unique filenames.
  - `createBank / addPresetToBank / createBankFromPresets / deleteBank / deletePreset / renamePreset`
  - `.fxp/.fxb` helpers are import/export only — engine never reads them directly.
- `Source/PresetBrowser.h/.cpp` — JUCE UI: FileDragAndDropTarget, bank list, preset list,
  Search, Save Preset, New Bank, Import, Open Folder, Rescan.
- `Source/PluginProcessor.h/.cpp` — owns a PresetManager, exposes `presetManager()`,
  `saveCurrentPreset()`, `currentPresetJson()` (stub returns current params as JSON in real build).

## Why JSON-first avoids issues
- VST2 `.fxp/.fxb` are legacy, big-endian, chunk-based, and FL Studio / VST3 don't use them natively.
  Keeping them as the main format would add a second code path + byte-order bugs.
- JSON is human-readable, atomic-writable, and already the format of the 2,000 factory presets.
- `.fxp/.fxb` still import fine — they are converted to JSON on drop, then everything is JSON.

## Testing
`g++ -std=c++17 Source/PresetManager.cpp tests/test_preset_manager.cpp -o /tmp/test_pm && /tmp/test_pm`
Covers: init, scan factory (2000), import .json/.fxp/.fxb, save preset, create bank, add to bank, delete/rename, bad-file tolerance.
