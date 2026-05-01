# Align & Distribute

Align & Distribute is a third-party OBS Studio dock for positioning multiple
scene items with Photoshop-style alignment, distribution, and match-size tools.

Select scene items in OBS, then use the compact toolbar to align edges, align
centres, distribute spacing, or match dimensions. Actions integrate with OBS
undo/redo.

## Features

- Align selected items left, horizontal centre, right, top, vertical centre, or bottom.
- Distribute selected items by left edges, centres, right edges, top edges, vertical centres, bottom edges, or equal spacing.
- Match width, height, or both dimensions.
- Full undo/redo integration via `obs_frontend_create_undo_redo_action`.
- Rotation-aware calculations using axis-aligned bounding boxes.
- Skips locked scene items automatically.
- Compact dock UI with icon buttons.

## Requirements

- OBS Studio 30.x, 31.x, or 32.x
- Windows x64 for the packaged release
- Qt 6, provided by OBS Studio

## Installation

### Windows

Download the release archive and extract its contents into your OBS Studio
installation directory. Restart OBS after installation.

The dock appears under:

```
View -> Docks -> Align & Distribute
```

## Building from Source

```bash
git clone https://github.com/KSTYER1/obs-distribute.git
cd obs-distribute
cmake --preset windows-x64
cmake --build --preset windows-x64 --config RelWithDebInfo
```

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
