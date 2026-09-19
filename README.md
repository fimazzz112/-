# Rust LOS ESP Lab V2

Standalone Windows laboratory demo. It does not attach to Rust, read Rust memory,
inject into Rust, or bypass EAC.

Features:
- Insert toggles a clickable settings panel.
- ESP toggle.
- Lines toggle.
- Only-visible toggle using line-of-sight against demo walls.
- Distance demo toggle.
- Mouse-clickable controls.

Build:
cmake -S . -B build -A x64
cmake --build build --config Release

The GitHub Actions workflow builds the EXE automatically.
