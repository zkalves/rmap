<!--
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at https://mozilla.org/MPL/2.0/.

  Copyright (c) 2026 Ezequiel Alves. All rights reserved.
-->

# rmap Colour Schemes & Themes

`rmap` features a modular, JSON-driven styling and colour scheme engine. All themes—including the 8 built-in palettes (**Solarized 8 Dark/Light**, **Nord**, **Dracula**, **Monokai**, **Classic**, and **High Contrast Dark/Light**)—are defined using declarative JSON files, making colours easily configurable, tweakable, and extensible. In addition, comprehensive Colour Vision Deficiency (CVD) profiles provide tailored accessibility for Deuteranopia, Protanopia, Tritanopia, Achromatopsia, and Universal Okabe-Ito barrier-free palettes.

---

## 1. Quickstart: Customizing or Creating Themes

### Option A: User Custom Themes (`~/.config/rmap/themes/`)
1. Place any `.json` theme file in `~/.config/rmap/themes/` (e.g. `~/.config/rmap/themes/my_theme.json`).
2. You can also open this folder directly from **Edit &rarr; Preferences... (`Ctrl+,`)** by clicking **Open Themes Folder...**.
3. `rmap` automatically detects all themes placed in this directory.
4. To override an existing built-in theme, name your file with the same ID (e.g. `solarized8.json`).

### Option B: Local Project Themes (`themes/`)
1. Create or edit `.json` files inside the repository's `themes/` directory.
2. Built-in themes are embedded directly into the binary at build time via Qt resources (`:/themes/`).

---

## 2. Theme JSON Schema Reference

A complete theme file defines:
- **`id`**: Unique alphanumeric theme identifier (e.g. `"solarized8"`, `"my_theme"`).
- **`name`**: Human-readable display name shown in menus and preferences.
- **`isDark`**: Boolean (`true` for dark themes, `false` for light themes).
- **`base`**: Core UI element colours (backgrounds, text, headers, selections, inputs, buttons).
- **`reserved`**: Graphical bitfield visualizer reserved slot background, border, diagonal stripe, and text colours.
- **`accessPolicies`**: Badge and slice visualizer colours for each hardware/software access mode (`rw`, `ro`, `wo`, `w1c`, `rc`, `na`).
- **`colorBlind`**: High-contrast CVD barrier-free palette overrides (Okabe-Ito / Wong standard).

```json
{
  "id": "my_theme",
  "name": "My Custom Theme",
  "isDark": true,
  "base": {
    "windowBg": "#1e1e2e",
    "panelBg": "#181825",
    "altRowBg": "#313244",
    "textColor": "#cdd6f4",
    "textMuted": "#6c7086",
    "headerBg": "#313244",
    "headerText": "#cdd6f4",
    "border": "#45475a",
    "selectionBg": "#585b70",
    "selectionText": "#f5e0dc",
    "buttonBg": "#313244",
    "buttonHover": "#45475a",
    "inputBg": "#181825",
    "inputBorder": "#45475a",
    "errorBg": "#45222a",
    "errorBorder": "#f38ba8",
    "errorText": "#f5e0dc"
  },
  "reserved": {
    "bg": "#313244",
    "border": "#45475a",
    "stripe": "#1e1e2e",
    "text": "#cdd6f4",
    "rulerText": "#6c7086"
  },
  "accessPolicies": {
    "rw":  { "bg": "#22382c", "border": "#a6e3a1", "text": "#a6e3a1" },
    "ro":  { "bg": "#1f3348", "border": "#89b4fa", "text": "#89b4fa" },
    "wo":  { "bg": "#3d2d24", "border": "#fab387", "text": "#fab387" },
    "w1c": { "bg": "#3b3823", "border": "#f9e2af", "text": "#f9e2af" },
    "rc":  { "bg": "#332644", "border": "#cba6f7", "text": "#cba6f7" },
    "na":  { "bg": "#313244", "border": "#45475a", "text": "#cdd6f4" }
  },
  "colorBlind": {
    "ro":  { "bg": "#9fa8da", "border": "#1a237e", "text": "#0f1450" },
    "wo":  { "bg": "#ce93d8", "border": "#4a148c", "text": "#320a50" },
    "w1c": { "bg": "#ff8a65", "border": "#bf360c", "text": "#641400" },
    "rw":  { "bg": "#80deea", "border": "#006064", "text": "#00323c" },
    "na":  { "bg": "#cfd8dc", "border": "#37474f", "text": "#282828" }
  }
}
```

---

## 3. Selecting Themes

- **GUI Menu**: Choose **View &rarr; Colour Scheme** from the main menu bar.
- **Preferences Dialog**: Select in **Edit &rarr; Preferences... (`Ctrl+,`)**.
- **CLI Parameter**: Launch with `-t, --theme, --colour-scheme <idOrName>` (e.g. `rmap -t nord` or `rmap --theme "Solarized 8 (Light)"`).
