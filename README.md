# Align & Distribute

Align & Distribute is a third-party OBS Studio dock for positioning multiple
scene items with Photoshop-style alignment, distribution, and match-size tools.

Select scene items in OBS, then use the compact toolbar to align edges, align
centres, distribute spacing, or match dimensions. Actions integrate with OBS
undo/redo.

## Features

- Align selected items left, horizontal centre, right, top, vertical centre, or
  bottom.
- Distribute selected items by left edges, centres, right edges, top edges,
  vertical centres, bottom edges, or equal spacing.
- Match width, height, or both dimensions.
- Full undo/redo integration via `obs_frontend_create_undo_redo_action`.
- Rotation-aware calculations using axis-aligned bounding boxes.
- Respects OBS bounds settings where applicable.
- Skips locked scene items automatically.
- Compact dock UI with icon buttons.

## Requirements

- OBS Studio 30.x, 31.x, or 32.x
- Windows x64 for the packaged release
- Qt 6, provided by OBS Studio

## Installation

Recommended installer:

1. Download the `*-setup.exe` file from the latest GitHub release.
2. Close OBS Studio.
3. Run the installer and select your OBS Studio installation folder.
4. Start OBS Studio again.

Portable or manual installation:

1. Download the release ZIP instead of the setup EXE.
2. Extract the ZIP into your OBS Studio installation folder, or copy the
   included `obs-plugins` and `data` folders into it.
3. Start OBS Studio again.

### Windows

Download the release archive and extract or copy its contents into your OBS
Studio installation directory.

The final layout should include:

```text
obs-plugins/64bit/obs-distribute.dll
data/obs-plugins/obs-distribute/locale/en-US.ini
data/obs-plugins/obs-distribute/locale/de-DE.ini
data/obs-plugins/obs-distribute/icons/*.svg
```

The packaged release also includes `INSTALL.bat`, which can copy the plugin
files into a selected OBS directory.

Restart OBS after installation. The dock appears under:

```text
View -> Docks -> Align & Distribute
```

## Basic Usage

1. Select two or more scene items in OBS.
2. Open `View -> Docks -> Align & Distribute`.
3. Click an align, distribute, or match-size button.
4. Use `Ctrl + Z` / `Ctrl + Y` to undo or redo the operation.

Button availability depends on the current selection:

- Align and match-size actions require two or more selected items.
- Distribute actions require three or more selected items.
- Locked items are ignored.
- Groups are treated as single top-level items.

## Tool Groups

### Align

Align items to the first selected item:

- Left
- Horizontal centre
- Right
- Top
- Vertical centre
- Bottom

### Distribute

Distribute items using the extremes of the current selection:

- Horizontal left edges
- Horizontal centres
- Horizontal right edges
- Horizontal spacing
- Vertical top edges
- Vertical centres
- Vertical bottom edges
- Vertical spacing

### Match Size

Match dimensions to the first selected item:

- Width
- Height
- Width and height

## Building from Source

Requires CMake 3.28 or newer, Qt 6, OBS development headers, and a supported
compiler toolchain.

```bash
git clone https://github.com/kstyer/obs-distribute.git
cd obs-distribute
cmake --preset windows-x64
cmake --build --preset windows-x64 --config RelWithDebInfo
```

The project uses the
[obs-plugintemplate](https://github.com/obsproject/obs-plugintemplate) build
structure.

## Version History

### 1.0.0

- Initial release.
- Added align, distribute, and match-size actions.
- Added OBS undo/redo integration.
- Added rotation-aware bounding box calculations.
- Added locked-item skipping.

## License

Align & Distribute is licensed under GPL-2.0-or-later.

## Disclaimer

Align & Distribute is an unofficial third-party plugin and is not affiliated
with or endorsed by the OBS Project.

AI-assisted tools were used during development and release preparation. The
maintainer is responsible for reviewing, testing, and publishing the released
plugin.
