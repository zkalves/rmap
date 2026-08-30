# CLI Reference & Automation

**rmap** includes built-in headless CLI capabilities allowing hardware build environments, EDA scripts, and CI/CD pipelines to validate register maps, run lint checks, compute semantic diffs, convert formats, and generate code without launching a graphical window.

---

## Command Line Syntax

```bash
rmap [OPTIONS]
```

### Options

| Option | Long Option | Description |
| :--- | :--- | :--- |
| `-f <file>` | `--file <file>` | Path to register map file to load (`.svd`, `.rdl`, `.xml`, `.json`, `.csv`, `.rmt`, `.rmb`). |
| `-c <file>` | `--convert <file>` | Headlessly convert the loaded register map into another format (e.g. `--convert out.svd`). |
| `-e` | `--export` | Run in **headless mode** and generate all configured template outputs. |
| `-l` | `--lint` | Run automated linter validation check on the loaded register map. |
| | `--strict` | Enable strict linting rules (enforce non-empty descriptions, address alignment). |
| | `--report-format <fmt>`| Report format for `--lint` (`text`, `json`, `sarif`, `junit`) or `--diff` (`text`, `markdown`). |
| `-d <file2>` | `--diff <file2>` | Perform semantic register map diff against another file. |
| `-t <scheme>`| `--theme, --colour-scheme <scheme>` | Set active colour scheme (`solarized8`, `solarized8_light`, `nord`, `dracula`, `monokai`, `classic`). |
| `-o <path>` | `--out <path>` | Override output destination directory for code generation, or output file path for lint/diff reports. |
| `-h` | `--help` | Display command-line help and usage. |
| `-v` | `--version` | Display application version. |

---

## Relative Paths & Environment Variables

**rmap** supports relative paths, absolute paths, and environment variable expansion across all CLI options, configuration files, and template mappings:

- **Environment Variable Syntax**: Supports POSIX `$VAR`, `${VAR}`, Windows `%VAR%`, and `~` (user home directory). E.g., `rmap -f $PROJECT_ROOT/registers/spi.rmt -o ${BUILD_DIR}/gen`.
- **Relative Path Resolution**: Relative paths are resolved relative to the loaded register map file's directory first, with fallback to the Current Working Directory (CWD).
- **Portable Repository Standard**: All paths saved within repository files (`.rmt`, `.rmb`, `.json`) are stored as clean relative paths for maximum portability across teams and CI environments.

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
rmap -f examples/systemRdl/atxmega_spi.rdl --convert spi.svd

# Convert ARM CMSIS-SVD to IP-XACT IEEE 1685 XML
rmap -f spi.svd --convert spi.xml

# Convert IP-XACT to CSV spreadsheet
rmap -f spi.xml --convert spi.csv

# Convert CSV back to SystemRDL
rmap -f spi.csv --convert spi.rdl

# Convert Protobuf to JSON schema
rmap -f examples/spi.rmt --convert spi.json
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

[Next: Templates & Code Generation &rarr;](templates-and-codegen.html)
