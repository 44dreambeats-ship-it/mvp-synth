# MVP Themes — Color Scheme + Theme Button

## Theme button
Top bar: **🎨 THEME**. Tap to cycle:
`Manual → Audio Reactive → Time of Day → Weather → Timer → Manual`

Dropdowns next to it pick details for each mode.

## Modes
- **Manual (default):** Locked 2nd look — off-black + acid lime + hot red.
  Presets: MVP / Midnight / Dawn / Day / Dusk / Forest.
- **Audio Reactive:** Color shifts with your sound. Pitch moves hue, loudness moves
  brightness. Smoothed (tau 0.8s) so it breathes, never strobes.
- **Time of Day:** Follows your Mac clock. Keyframes Night 0h → Dawn 6h → Noon 12h →
  Dusk 18h → Night 24h, cosine-interpolated. Day→night is gradual because time
  itself moves slowly + colors lerp with tau 30s. No shock.
- **Weather:** Clear / Cloudy / Rain / Snow / Thunder / Fog / Night Clear.
  Auto = Open-Meteo free API (no key) `current=weather_code,is_day`, maps code → condition.
  Falls back to manual if offline. Smoothed tau 5s.
- **Timer:** Hue slowly drifts over your timer (default 60 min, 5–120 min).
  Start / Pause. Progress loops. Tau 3s.

## Gradual rule (no shock to the eye)
Every frame: `current = lerp(current, target, 1 - exp(-dt / tau))`
- 1 frame @60fps toward Day: max channel step ~0.012 (tested < 0.05)
- 11:59 → 12:01 time-of-day: step ~0.0 (tested < 0.02)
- Manual switch: ~1.2s fade, not a cut.

## Code
- `Source/ThemeManager.h/.cpp` — no JUCE dep, pure color math, tested.
- `Source/ThemeControls.h/.cpp` — JUCE top-bar controls, 30fps update, day/night gradient preview strip.
- Editor wires `onColorsChanged` → repaint with `ThemeColors`.

## Test
`g++ -std=c++17 Source/ThemeManager.cpp tests/test_theme_manager.cpp -o /tmp/test_theme && /tmp/test_theme` → PASS
