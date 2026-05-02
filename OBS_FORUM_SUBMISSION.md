# OBS Forum Submission Draft: Align & Distribute

## Resource Title

Align & Distribute

## Version

1.0.0

## Category

OBS Studio Plugins

## Tags

align, distribute, scene items, layout, dock, positioning, match size, workflow

## Short Tagline

Photoshop-style align, distribute, and match-size tools for OBS scene items.

## Supported Bit Versions

64-bit

## Supported Platforms

Windows

## Minimum OBS Studio Version

30.0.0

## Source Code URL

https://github.com/KSTYER1/obs-distribute

## Download URL

https://github.com/KSTYER1/obs-distribute/releases/tag/v1.0.0

## Overview

Align & Distribute is an unofficial third-party dock for OBS Studio. It adds a
compact toolbar for positioning multiple scene items with familiar
Photoshop-style alignment, distribution, and match-size tools.

Select scene items in OBS, then use the dock to align edges, align centres,
distribute spacing, or match dimensions. Operations integrate with OBS
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

## Installation

Download the Windows x64 release archive and extract or copy its contents into
your OBS Studio installation directory.

The final layout should include:

```text
obs-plugins/64bit/obs-distribute.dll
data/obs-plugins/obs-distribute/locale/en-US.ini
data/obs-plugins/obs-distribute/locale/de-DE.ini
data/obs-plugins/obs-distribute/icons/*.svg
```

The release archive also includes `INSTALL.bat`, which can copy the plugin into
a selected OBS directory.

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

## What's New in 1.0.0

- Initial release.
- Added align, distribute, and match-size actions.
- Added OBS undo/redo integration.
- Added rotation-aware bounding box calculations.
- Added locked-item skipping.

## Support / Bugs

Please report issues in the resource discussion thread or in the GitHub issue
tracker once the repository is published.

## License

GPL-2.0-or-later.

## Disclaimer

Align & Distribute is an unofficial third-party plugin and is not affiliated
with or endorsed by the OBS Project.

AI-assisted tools were used during development and release preparation. The
maintainer is responsible for reviewing, testing, and publishing the released
plugin.

## Pre-Submit Checklist

- [x] Public GitHub repository exists.
- [x] README is visible on GitHub.
- [x] GPL license is visible on GitHub.
- [x] Source Code URL field points to the repository.
- [x] Release ZIP is attached to GitHub Releases or uploaded to the forum.
- [ ] At least one screenshot/GIF is added to the resource description.
- [ ] Description is in English.
- [ ] No OBS logo is used as resource icon or marketing artwork.
