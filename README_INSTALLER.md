# MVP Installer — Self-Installing .pkg (No Terminal for Buyers)

Buyer flow (what people do):
1. Download `MVP-Installer.pkg` from Gumroad
2. Double-click it → Continue → Install
3. Open FL Studio → Manage plugins → Find plugins → MVP pops up

No Terminal, no scripts, no copying files by hand.

## What the .pkg installs
- `MVP.vst3` → `~/Library/Audio/Plug-Ins/VST3/` (FL Studio VST3 folder)
- `MVP.component` → `~/Library/Audio/Plug-Ins/Components/` (if built)
- `Presets/` (2,000, by type) → `~/Documents/MVP/Presets/`

## How the .pkg is made (one-time, on Mac, after MVP.vst3 is built)
The .pkg is built from the `installer/` folder in this repo:
- `payload/` holds the files to install (MVP.vst3 + Presets)
- `Distribution.xml` + Welcome / Conclusion / License are the installer screens
- Built with macOS `productbuild` / `pkgbuild` (standard Apple installer tools)

The .pkg itself contains no user-visible scripts. The buyer just double-clicks it.

## Status
The installer project is ready. The final `MVP-Installer.pkg` file can be built once `MVP.vst3` exists from a Mac build. Until then, the Gumroad file is the source zip — buyers will get the .pkg once the first Mac build is done.
