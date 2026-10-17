#!/bin/bash
# MVP — Direct to FL Studio installer (macOS)
# Run on your MacBook: bash install_mvp.sh
# What it does:
#   1. Finds MVP.vst3 in this folder (or in build/MVP_artefacts/Release/VST3/)
#   2. Copies it to ~/Library/Audio/Plug-Ins/VST3/  (FL Studio VST3 folder)
#   3. Copies MVP.component (if present) to ~/Library/Audio/Plug-Ins/Components/
#   4. Copies Presets/ to ~/Documents/MVP/Presets/ (ONE bank, by type)
#   5. Prints FL Studio rescan steps
# No third-party branding. MVP only.
set -e
cd "$(dirname "$0")"

VST3_SRC=""
if [ -d "MVP.vst3" ]; then VST3_SRC="MVP.vst3";
elif [ -d "build/MVP_artefacts/Release/VST3/MVP.vst3" ]; then VST3_SRC="build/MVP_artefacts/Release/VST3/MVP.vst3";
fi

if [ -z "$VST3_SRC" ]; then
  echo "MVP.vst3 not found."
  echo "Build it first: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release -j 4"
  echo "Then run this script again."
  exit 1
fi

echo "Installing MVP.vst3 → ~/Library/Audio/Plug-Ins/VST3/"
mkdir -p ~/Library/Audio/Plug-Ins/VST3/
rm -rf ~/Library/Audio/Plug-Ins/VST3/MVP.vst3
cp -R "$VST3_SRC" ~/Library/Audio/Plug-Ins/VST3/MVP.vst3

if [ -d "MVP.component" ]; then
  echo "Installing MVP.component → ~/Library/Audio/Plug-Ins/Components/"
  mkdir -p ~/Library/Audio/Plug-Ins/Components/
  rm -rf ~/Library/Audio/Plug-Ins/Components/MVP.component
  cp -R "MVP.component" ~/Library/Audio/Plug-Ins/Components/MVP.component
elif [ -d "build/MVP_artefacts/Release/AU/MVP.component" ]; then
  echo "Installing MVP.component → ~/Library/Audio/Plug-Ins/Components/"
  mkdir -p ~/Library/Audio/Plug-Ins/Components/
  rm -rf ~/Library/Audio/Plug-Ins/Components/MVP.component
  cp -R "build/MVP_artefacts/Release/AU/MVP.component" ~/Library/Audio/Plug-Ins/Components/MVP.component
fi

# Presets — ONE folder, by type
if [ -d "Presets" ]; then
  echo "Installing Presets → ~/Documents/MVP/Presets/"
  mkdir -p ~/Documents/MVP/Presets/
  cp -R Presets/* ~/Documents/MVP/Presets/ 2>/dev/null || true
elif [ -f "MVP_Presets_2000.json" ]; then
  echo "Installing MVP_Presets_2000.json → ~/Documents/MVP/Presets/"
  mkdir -p ~/Documents/MVP/Presets/
  cp MVP_Presets_2000.json ~/Documents/MVP/Presets/
fi

echo ""
echo "Done. MVP is now in your FL Studio plugins."
echo "In FL Studio: Options > Manage plugins > Find plugins > search MVP"
ls -lh ~/Library/Audio/Plug-Ins/VST3/MVP.vst3 2>/dev/null | head -n 2 || true
