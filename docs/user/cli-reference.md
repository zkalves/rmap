# CLI Reference & Automation

**rmap** includes built-in headless CLI capabilities allowing hardware build environments, EDA scripts, and CI/CD pipelines to validate register maps, run lint checks, compute semantic diffs, convert formats, and generate code without launching a graphical window.

> [!NOTE]
> Qt 6 is a required dependency for all builds of **rmap**. Headless CLI operations run automatically offscreen without requiring an X11/Wayland display server by initializing the offscreen platform plugin (`QT_QPA_PLATFORM=offscreen`).

---

## Command Line Syntax

```bash
rmap [OPTIONS] [file]
```

### Options

| Option | Long Option | Description |
| :--- | :--- | :--- |
| `-f <file>` | `--file <file>` | Path to register map file to load (`.svd`, `.rdl`, `.systemrdl`, `.xml`, `.json`, `.csv`, `.tsv`, `.rmt`, `.rmb`). Can also be passed directly as a positional argument `[file]`. |
| `-c <file>` | `--convert <file>` | Headlessly convert the loaded register map into another format (e.g. `--convert out.svd`). |
| `-e` | `--export` | Run in **headless mode** and generate all configured template outputs. |
| `-l` | `--lint` | Run automated linter validation check on the loaded register map. |
| | `--strict` | Enable strict linting rules (enforce non-empty descriptions, address alignment). |
| | `--report-format <fmt>`| Report format for `--lint` (`text`, `json`, `sarif`, `junit`) or `--diff` (`text`, `markdown`). |
| `-d <file2>` | `--diff <file2>` | Perform semantic register map diff against another file. |
| `-t <scheme>`| `--theme, --colour-scheme, --color-scheme <scheme>` | Set active colour scheme (`solarized8`, `solarized8_light`, `nord`, `dracula`, `monokai`, `classic`, `high_contrast_dark`, `high_contrast_light`). |
| | `--lang, --language <lang>` | Set application language (`en`, `es`, `de`, `fr`, `zh_CN`, `ja`, `pt_BR`). |
| `-o <path>` | `--out <path>` | Override output destination directory for code generation, or output file path for lint/diff reports. |
| `-h` | `--help` | Display command-line help and usage. |
| | `--help-all` | Display command-line help including generic Qt options. |
| `-v` | `--version` | Display application version. |

### Headless Action Precedence & Execution

When invoked in headless mode, multiple actions are evaluated sequentially in the following priority order, executing the first matching action and terminating execution with its respective exit status code:

1. **Semantic Diff** (`-d` / `--diff`): Compares loaded register map against another file. Exits `0` if models are structurally identical, `1` if differences are found or an error occurs.
2. **Linter Validation** (`-l` / `--lint`): Audits the register map against structural and alignment rules. Exits `0` on clean pass, `1` on lint failure.
3. **Format Conversion** (`-c` / `--convert`): Converts the loaded register map into the target format specified in `<file>`. Exits `0` on success, `1` on failure.
4. **Template Export** (`-e` / `--export`): Generates code across all enabled templates into the specified output directory (`-o`). Exits `0` on success, `1` on failure.

Because each headless action exits immediately upon completion, specifying multiple action flags in a single command invocation (for instance, `rmap -f spi.rmt --lint --export`) executes only the highest-priority action (`--lint`). To execute multiple operations, invoke them as separate steps in your build system or CI pipeline.

---

## Relative Paths & Environment Variables

**rmap** supports relative paths, absolute paths, and environment variable expansion across all CLI options, configuration files, and template mappings:

- **Environment Variable Syntax**: Supports POSIX `$VAR`, `${VAR}`, Windows `%VAR%`, and `~` (user home directory). E.g., `rmap -f $PROJECT_ROOT/registers/spi.rmt -o ${BUILD_DIR}/gen`.
- **Relative Path Resolution**: Relative paths are resolved relative to the loaded register map file's directory first, with fallback to the Current Working Directory (CWD).
- **Portable Repository Standard**: All paths saved within repository files (`.rmt`, `.rmb`, `.json`) are stored as clean relative paths for maximum portability across teams and CI environments.

### Supported Environment Variables

| Variable | Description |
| :--- | :--- |
| `RMAP_CONFIG_FILE` | Explicit path overriding the default configuration file (`~/.config/rmap/rmap.conf` or `$XDG_CONFIG_HOME/rmap/rmap.conf`). |
| `RMAP_THEMES_PATH` / `RMAP_THEME_DIR` | Colon-separated (Linux/macOS) or semicolon-separated (Windows) search path list for custom theme JSON definitions. |
| `RMAP_TRANSLATIONS_PATH` / `RMAP_TRANSLATION_DIR` | Colon-separated (Linux/macOS) or semicolon-separated (Windows) search path list for runtime JSON translation catalogs (`rmap_*.json`). |
| `RMAP_TEMPLATES_DIR` | Explicit filesystem path overriding the default templates discovery and installation directory. |
| `RMAP_EXAMPLES_DIR` | Explicit filesystem path overriding the default bundled examples directory. |
| `RMAP_DOCS_DIR` | Explicit filesystem path overriding the default offline documentation directory. |
| `RMAP_PYTHON` | Custom Python interpreter path/binary (defaults to `python3` or `python` discovered on `PATH`). |
| `PYTHON` | Fallback Python interpreter path/binary checked if `RMAP_PYTHON` is not set. |
| `RMAP_PYTHON_TIMEOUT` | Execution timeout in milliseconds (default: `60000` ms / 60 seconds) for custom Python generation scripts and post-processing filters. |
| `RMAP_TMPDIR` | Explicit temporary directory path for intermediate JSON context files generated during script execution (defaults to system temp directory). |

---

## CI/CD Linter & Verification (`--lint`)

The automated linter checks for register address collisions, bitfield overlaps, bit-width overflows, and (in `--strict` mode) missing documentation:

```bash
# 1. Standard text output to stdout
rmap -f rtl/spi.rmt --lint

# 2. Strict lint with SARIF report for GitHub PR Code Scanning annotations
rmap -f rtl/spi.rmt --lint --strict --report-format sarif -o work/lint.sarif

# 3. JUnit XML report for Jenkins/GitLab CI dashboard integration
rmap -f rtl/spi.rmt --lint --strict --report-format junit -o work/junit.xml
```

When errors are detected (or warnings in strict mode), `rmap` returns a non-zero exit code (`1`), immediately failing CI pipeline stages before erroneous register maps reach tapeout.

---

## Semantic Register Map Diffing (`--diff`)

Compare two register maps and output structural changes (added, removed, modified registers and bitfields):

```bash
# Terminal human-readable diff
rmap -f v1.0.rmt --diff v2.0.rmt --report-format text

# GitHub PR description Markdown table
rmap -f v1.0.rmt --diff v2.0.rmt --report-format markdown -o pr_diff.md
```

---

## Multi-Format Conversion Examples

Translate between any register map format headlessly:

```bash
# Convert SystemRDL to ARM CMSIS-SVD (.svd)
rmap -f examples/systemrdl/atxmega_spi.rdl --convert spi.svd

# Convert ARM CMSIS-SVD to IP-XACT IEEE 1685 XML
rmap -f spi.svd --convert spi.xml

# Convert IP-XACT to CSV spreadsheet
rmap -f spi.xml --convert spi.csv

# Convert CSV back to SystemRDL
rmap -f spi.csv --convert spi.rdl

# Convert Protobuf to JSON schema
rmap -f examples/rmt/peripherals/spi.rmt --convert spi.json
```

---

## Makefile Integration Example

```makefile
REGMAP = registers/soc_map.rmt
OUT_DIR = generated

.PHONY: all check codegen

all: check codegen

check:
	@echo "Linting Register Map..."
	@rmap -f $(REGMAP) --lint --strict

codegen:
	@echo "Generating RTL, UVM, and Firmware Models..."
	@rmap -f $(REGMAP) --export -o $(OUT_DIR)
```

---

## CI/CD GitHub Actions Workflow Example

```yaml
- name: Lint Register Maps
  run: |
    ./build/bin/rmap -f registers/soc_map.rmt --lint --strict --report-format sarif --out rmap-lint.sarif

- name: Upload SARIF Results
  uses: github/codeql-action/upload-sarif@v3
  with:
    sarif_file: rmap-lint.sarif

- name: Generate RTL & Verification Models
  run: |
    ./build/bin/rmap -f registers/soc_map.rmt --export --out ./build/generated
```

In headless Linux server environments (where `$DISPLAY` is unset), **rmap** automatically initializes Qt in offscreen mode (`QT_QPA_PLATFORM=offscreen`), eliminating `Cannot connect to X server` failures.

[Next: Templates & Code Generation &rarr;](templates-and-codegen.md)
