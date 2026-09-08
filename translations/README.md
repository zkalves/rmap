<!--
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at https://mozilla.org/MPL/2.0/.

  Copyright (c) 2026 Ezequiel Alves. All rights reserved.
-->

# Adding New Languages to rmap (Build-Time Configuration)

In `rmap`, all language configurations are defined strictly at **build time**. Runtime addition, dynamic directory scanning, or runtime removal of languages is intentionally disabled to ensure deterministic, secure, and self-contained builds.

Adding a new language to `rmap` is quick and straightforward following the steps below.

---

## Step-by-Step Guide to Adding a New Language

### 1. Create the Translation JSON File

Use `translations/template.json` as your starting point:

```bash
cp translations/template.json translations/rmap_<lang_code>.json
```

For example, for Italian (`it`):
```bash
cp translations/template.json translations/rmap_it.json
```

Edit `translations/rmap_it.json` and fill in:
- `"code"`: ISO 639-1 language code (e.g., `"it"`).
- `"name"`: English display name (e.g., `"Italian"`).
- `"nativeName"`: Native display name (e.g., `"Italiano"`).
- `"translations"`: Key-value map translating the application strings into your target language.

---

### 2. Embed the File in Qt Resources (`res/resources.qrc`)

All translation files are compiled directly into the binary through Qt resources.

Open `res/resources.qrc` and add your file inside the `<qresource prefix="translations">` block:

```xml
<file alias="rmap_it.json">../translations/rmap_it.json</file>
```

---

### 3. Register the Language in `src/LanguageManager.cpp`

Open `src/LanguageManager.cpp` and add the language entry to the `buildTimeTable` inside `LanguageManager::initLanguages()`:

```cpp
{"it", "Italian", "Italiano", ":/translations/rmap_it.json"},
```

---

### 4. Build and Verify

Rebuild the application:

```bash
make
```

Run unit tests to verify:

```bash
make test-unit
```

Once built, your new language is permanently available:
- In the menu bar under **View &rarr; Language**
- In the **Preferences** dialog (**Edit &rarr; Preferences...** or `Ctrl+,`)
- Via the command-line flag: `rmap --lang it` or `rmap --lang Italian`
