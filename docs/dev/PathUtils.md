# PathUtils

### A. Overview

`PathUtils` is a cross-platform path resolution and normalization utility library for **rmap**. It expands user home directories (`~`), POSIX environment variables (`$VAR`, `${VAR}`), and Windows environment variables (`%VAR%`), normalizes path separators, converts absolute paths into clean relative paths anchored to a base directory, and performs multi-tiered fallback path resolution for template lookup and output artifact placement.

### B. Namespaces

#### PathUtils
Groups free functions and project-wide default path constants for environment variable expansion, path relativization, separator normalization, and fallback resolution.

### C. Constants & Default Path Accessors

#### `constexpr const char* DEFAULT_OUTPUT_DIR = "./work"`
Standard application-wide default output directory.

#### `constexpr const char* DEFAULT_TEMPLATES_DIR = "./templates"`
Standard application-wide default templates directory.

#### `inline QString defaultOutputDir()`
Returns `DEFAULT_OUTPUT_DIR` as a `QString` (`"./work"`).

#### `QString defaultTemplatesDir()`
Returns the application default templates directory using a smart multi-tiered resolution hierarchy:
1. `RMAP_TEMPLATES_DIR` environment variable (if set and non-empty).
2. Local `./templates` if it exists and contains template subfolders or files (for in-tree development and testing).
3. Application-relative relocatable directory: `<bin_dir>/../share/rmap/templates` (for standalone/relocatable installs).
4. Configured compile-time installation path: `RMAP_INSTALL_TEMPLATES_DIR` (e.g. `/usr/local/share/rmap/templates`).
5. Fallback relative path `"./templates"`.

#### `QString defaultExamplesDir()`
Returns the application default examples directory using a similar multi-tier resolution hierarchy:
1. `RMAP_EXAMPLES_DIR` environment variable.
2. Local `./examples` if it exists.
3. Application-relative relocatable directory: `<bin_dir>/../share/rmap/examples`.
4. Configured compile-time installation path: `RMAP_INSTALL_EXAMPLES_DIR`.
5. Fallback relative path `"./examples"`.

#### `QString defaultDocsDir()`
Returns the application default documentation directory:
1. `RMAP_DOCS_DIR` environment variable.
2. Local `./docs` if it exists.
3. Application-relative relocatable directory: `<bin_dir>/../share/doc/rmap`.
4. Configured compile-time installation path: `RMAP_INSTALL_DOCDIR`.
5. Fallback relative path `"./docs"`.

### D. Functions

#### QString expandEnvVars(const QString &path)
#### std::string expandEnvVars(const std::string &path)
#### inline QString expandEnvVars(const char *path)
Expands tilde home references (`~` or `~/` → user home directory), POSIX syntax (`$VAR`, `${VAR}`), and Windows syntax (`%VAR%`) within `path`. Normalizes backslashes to forward slashes.

#### QString toRelativePath(const QString &targetPath, const QString &baseDir = QString())
#### std::string toRelativePath(const std::string &targetPath, const std::string &baseDir = "")
#### inline QString toRelativePath(const char *targetPath, const char *baseDir = nullptr)
Converts an absolute or relative `targetPath` into a clean relative path anchored to `baseDir` (defaulting to current working directory if empty). If `targetPath` begins with an environment variable (`$` or `%`), it is left un-relativized.

#### QString resolvePath(const QString &path, const QString &primaryBaseDir = QString(), const QString &secondaryBaseDir = QString())
#### std::string resolvePath(const std::string &path, const std::string &primaryBaseDir = "", const std::string &secondaryBaseDir = "")
#### inline QString resolvePath(const char *path, const char *primaryBaseDir = nullptr, const char *secondaryBaseDir = nullptr)
Resolves a path using a deterministic search hierarchy:
1. Expands environment variables and home directory shortcuts (`~`).
2. If expanded path is absolute and exists, returns canonical absolute path.
3. If relative, tests existence inside `primaryBaseDir`.
4. If not found, tests existence inside `secondaryBaseDir` (automatically handling template prefix stripping when `secondaryBaseDir` is a template directory).
5. If not found, tests existence inside current working directory (`CWD`).
6. If the target file does not yet exist (e.g. creating an output file), anchors it against `primaryBaseDir`, `secondaryBaseDir`, or `CWD`.

#### QString normalizeSeparators(const QString &path)
#### std::string normalizeSeparators(const std::string &path)
#### inline QString normalizeSeparators(const char *path)
Replaces Windows backslashes (`\`) with standardized forward slashes (`/`) and removes redundant duplicate slashes.

### D. Dependencies

- `QString`, `QDir`, `QFileInfo` (QtCore) — Qt path inspection and directory manipulation.
- Standard Library (`<string>`, `<cstdlib>`, `<algorithm>`) — Environment variable querying.

### E. Usage Example

```cpp
#include "PathUtils.hpp"
#include <iostream>

void demonstratePathUtils()
{
    // Expand environment variables
    QString expanded = PathUtils::expandEnvVars("$HOME/projects/rmap/examples/rmt/peripherals/spi.rmt");

    // Relativize against project root
    QString rel = PathUtils::toRelativePath("/home/user/project/build/output.sv", "/home/user/project");
    // Returns: "build/output.sv"

    // Resolve template file with fallback folders
    QString resolved = PathUtils::resolvePath(
        "reg_map.h.inja",
        "/custom/templates",
        "./templates"
    );
}
```
