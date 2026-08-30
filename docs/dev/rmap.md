# rmap Application Entry Point

### A. Overview

The `rmap.cpp` translation unit serves as the main application entry point for the **rmap** hardware register map designer, validator, and code generation tool. It handles startup initialization, headless vs. GUI mode auto-detection, command-line argument processing via `QCommandLineParser`, batch CLI orchestration (linter execution, semantic diffing, format conversion, code generation), and main window lifecycle management before launching the Qt event loop.

### B. Qt Application Setup

The entry point instantiates a standard `QApplication` instance with command-line arguments. Prior to GUI initialization, the startup logic inspects display environment variables (`DISPLAY`, `WAYLAND_DISPLAY`) and CLI flags. If no graphical display server is detected or if a batch CLI command (`--export`, `--convert`, `--lint`, `--diff`, `--help`, `--version`) is supplied, the application automatically sets the `QT_QPA_PLATFORM` environment variable to `offscreen` to ensure headless CI/CD execution without graphical server dependencies.

Application metadata is configured on the `QApplication` instance:
- `setApplicationName("rmap")`
- `setApplicationDisplayName("rmap")`
- `setDesktopFileName("rmap")`
- `setApplicationVersion("v0.2.0")`
- `setWindowIcon(...)` configured with multi-resolution icon sizes (16x16 up to 512x512) from embedded Qt resources (`:/icons/app_icon.png`).

Resource initialization is triggered at startup via `Q_INIT_RESOURCE(resources)`.

### C. Command-Line Handling

Command-line arguments are parsed using Qt's `QCommandLineParser`. The following options are recognized:

| Option Flag(s) | Value Name | Default | Description |
|---|---|---|---|
| `-f`, `--file` | `file` | Empty | Specifies the register map file to load at startup (`.svd`, `.rdl`, `.xml`, `.json`, `.csv`, `.rmt`, `.rmb`). |
| `-e`, `--export` | None | Disabled | Runs headlessly in batch mode to export all configured Inja templates to target source files. |
| `-o`, `--out` | `path` | Empty | Overrides the target output directory for code generation, or the destination file path for lint and diff reports. |
| `-c`, `--convert` | `out_file` | Empty | Converts the loaded register map to the target format (inferred by file extension) and exits. |
| `-l`, `--lint` | None | Disabled | Executes architectural validation checks on the loaded register map without opening a GUI window. |
| `--strict` | None | Disabled | Enables strict linting rules, treating missing descriptions and address alignment violations as errors. |
| `--report-format` | `format` | `text` | Sets the output format for `--lint` (`text`, `json`, `sarif`, `junit`) or `--diff` (`text`, `markdown`). |
| `-d`, `--diff` | `compare_file` | Empty | Performs a semantic architectural comparison between the primary loaded register map and a second file. |
| `-t`, `--theme`, `--colour-scheme`, `--color-scheme` | `scheme` | `solarized8` | Sets the active UI theme (`solarized8`, `solarized8_light`, `nord`, `dracula`, `monokai`, `classic`). |
| `-h`, `--help` | None | N/A | Displays the standard Qt command-line help description. |
| `-v`, `--version` | None | N/A | Displays the application version string. |

All input paths passed through CLI arguments undergo environment variable expansion via `PathUtils::expandEnvVars()`.

### D. Top-Level Object Creation

In interactive GUI mode or standard single-file headless modes, the entry point instantiates the primary `RegMapWindow` instance, passing the expanded input file path:

- `RegMapWindow *mainWin = new RegMapWindow(regmap_file)`: Owns the tree model, configuration dialog, preferences dialog, undo stack, and visualizer widgets.

For semantic diff execution (`--diff`), the static method `RegMapWindow::semanticDiff(regmap_file, file2, format, out_path)` is executed directly without allocating a top-level window.

### E. Wiring and Connections

Top-level object configuration includes:
- Applying theme overrides: `mainWin->setColourScheme(...)` if the theme flag is supplied.
- Routing CLI action dispatch: executing `headlessLint`, `fileSave` (conversion), or `headlessExport` on the window instance before freeing the object and returning the process exit code.

### F. Event Loop

In interactive GUI mode, `mainWin->show()` is invoked and the Qt event loop is started with `app.exec()`. The process returns `0` upon normal application exit or `1` upon headless validation/conversion failure.

### G. Dependencies

- `QApplication` (QtWidgets) — Core GUI application controller and main event loop.
- `QCommandLineParser`, `QCommandLineOption` (QtCore) — CLI option specification, argument parsing, and help/version formatting.
- `RegMapWindow` (Project) — Main application window and headless controller.
- `ThemeManager` (Project) — Theme definitions and application styling.
- `PathUtils` (Project) — Environment variable expansion and path normalization.
