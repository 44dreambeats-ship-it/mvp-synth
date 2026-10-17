# Build MVP VST3 on your MacBook (FL Studio)

MVP — Melodic Vibration Processor. No third-party branding.

## 1. Download + unzip
Download `MVP-Source-VST3.zip` from your Gumroad product, then:
```bash
cd ~/Downloads
unzip -o MVP-Source-VST3.zip
cd mvp-synth
```

## 2. Install tools (first time only)
```bash
xcode-select --install
# If it says already installed, skip.
brew install cmake
# If you don't have Homebrew: /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

## 3. Build (this downloads JUCE ~ first time, takes a few minutes)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j 4
```

If it fails, copy the last 30 lines of the error — do NOT delete `build/`.

## 4. Install VST3 for FL Studio
```bash
mkdir -p ~/Library/Audio/Plug-Ins/VST3/
cp -R "build/MVP_artefacts/Release/VST3/MVP.vst3" ~/Library/Audio/Plug-Ins/VST3/
# AU (optional):
mkdir -p ~/Library/Audio/Plug-Ins/Components/
cp -R "build/MVP_artefacts/Release/AU/MVP.component" ~/Library/Audio/Plug-Ins/Components/ 2>/dev/null || echo "AU not built, VST3 is enough"
```

## 5. Load in FL Studio
1. FL Studio > Options > Manage plugins > Find plugins (or Find more plugins)
2. Search "MVP" — it should appear as MVP (Melodic Vibration Processor)
3. Add it to Channel Rack. Presets folder auto-creates at `~/Documents/MVP/Presets/` with 2,000 presets in ONE bank "Presets" organized by type: Bass / Synth / Pluck / Pad / Keys / Cinematic / Arp / FX
4. Play C3-C5 on a Synth preset, C1-C2 on a Bass preset, turn Master Tune to 432 Hz, move Macro 1.

## If MVP doesn't show up
- FL Studio > Options > Manage plugins > make sure VST3 path `~/Library/Audio/Plug-Ins/VST3` is scanned
- Click "Find plugins" again, check "Verify plugins"
- Make sure `MVP.vst3` is a folder/bundle, not a single file — `ls -lh ~/Library/Audio/Plug-Ins/VST3/MVP.vst3` should show a directory.

## What this build is
VST3 + AU, 64-bit, macOS Intel + Apple Silicon. JSON-first presets, drag-drop Presets folder, Theme button (Manual / Audio / Time / Weather / Timer, all gradual).
