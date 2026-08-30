# FormatManager and Multi-Format Engine

### 1. Overview

`FormatManager` is the serialization dispatcher and format registry in **rmap**. It enables reading, writing, and cross-converting register map specifications across 6 industry-standard electronic design automation (EDA) and serialization formats: ARM CMSIS-SVD, Accellera SystemRDL 1.0/2.0, IP-XACT IEEE 1685, JSON, CSV/TSV, and Google Protocol Buffers (text `.rmt` and binary `.rmb`).

### 2. Project Structure and Dependencies

Declared in `src/format/FormatManager.hpp` and `src/format/IFormatHandler.hpp`, with implementations in `src/format/*.cpp`.
- Directly updates and serializes `RegMapTreeModel` and `RegConfigWindow`.
- Invoked by `RegMapWindow::fileOpen()`, `RegMapWindow::fileSave()`, and CLI `--convert`.

Build Requirements:
- Qt 6 modules: `QtCore`, `QtXml`
- Protocol Buffers runtime library (`libprotobuf`)
- Embedded `nlohmann::json`

### 3. Core Interface and Result Types

#### FormatResult
Status report returned by read and write operations:

| Member | Type | Description |
|---|---|---|
| `success` | `bool` | `true` if file operation succeeded without critical errors. |
| `errorMessage` | `QString` | Diagnostic error message if `success` is `false`. |
| `warnings` | `QStringList` | Non-fatal warnings encountered during parsing. |

#### IFormatHandler
Abstract base interface for all format serializers and deserializers:
- `virtual QString formatName() const = 0`: Returns descriptive name (e.g. `"ARM CMSIS-SVD"`).
- `virtual QStringList supportedExtensions() const = 0`: Returns file extensions (e.g. `{"svd", "xml"}`).
- `virtual QString fileFilter() const = 0`: Returns `QFileDialog` filter string.
- `virtual FormatResult read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) = 0`: Deserializes file into tree model and config.
- `virtual FormatResult write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) = 0`: Serializes tree model and config into target format.

### 4. Registered Format Handlers

| Handler Class | Target Format | Extensions | Capabilities |
|---|---|---|---|
| `ProtobufHandler` | Google Protocol Buffers | `.rmt`, `.rmb` | Native format; high-performance binary and human-readable text format round-trip serialization. |
| `SystemRdlHandler` | Accellera SystemRDL 1.0/2.0 | `.rdl` | Complete lexer and parser supporting `addrmap`, `regfile`, `reg`, `field`, access properties, reset values, and documentation strings. |
| `IpxactHandler` | IP-XACT IEEE 1685-2009/2014/2022 | `.xml`, `.ipxact` | High-speed streaming XML parser/generator for `spirit:` and `ipxact:` memory maps, register definitions, and bitfields. |
| `CmsisSvdHandler` | ARM CMSIS-SVD | `.svd`, `.xml` | ARM Cortex-M peripheral description XML parser and emitter (`<peripheral>`, `<register>`, `<field>`). |
| `JsonHandler` | Standard JSON Schema | `.json` | Structured JSON schema export and import, preserving blocks, registers, fields, memories, and metadata. |
| `CsvHandler` | RFC 4180 Table | `.csv`, `.tsv` | Spreadsheet table parser and exporter with automatic delimiter detection and header alignment. |

### 5. FormatManager API

#### static FormatManager& instance()
Returns the global `FormatManager` singleton reference.

#### void registerHandler(std::shared_ptr<IFormatHandler> handler)
Registers a new format handler in the registry.

#### std::shared_ptr<IFormatHandler> handlerForFile(const QString &filepath) const
#### std::shared_ptr<IFormatHandler> handlerForFile(const QString &filepath) const
Inspects `filepath` extension (and optionally content headers) to return the matching format handler (`nullptr` if unsupported).

#### std::shared_ptr<IFormatHandler> handlerByName(const QString &name) const
#### std::shared_ptr<IFormatHandler> handlerByName(const QString &name) const
Returns the handler matching format identifier string `name`.

#### FormatResult loadFile(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config)
Dispatches reading of `filepath` to the matching handler, populating `model` and `config`.

#### FormatResult saveFile(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config)
Dispatches saving to the matching format handler inferred from `filepath` extension.

#### QString allFilterString() const / QString allFilterString() const
Generates a combined `QFileDialog` filter string encompassing all supported extensions.

#### const std::vector<std::shared_ptr<IFormatHandler>>& handlers() const
Returns the collection of registered handlers.

### 6. Ownership and Lifecycle

`FormatManager` is a process-lifetime singleton that instantiates default handlers upon construction.

### 7. Thread Safety

`FormatManager` registry queries are thread-safe. `loadFile` and `saveFile` calls mutate the provided `RegMapTreeModel` and should run on the thread owning the model.

### 8. Usage Example

```cpp
#include "format/FormatManager.hpp"
#include "RegMapTreeModel.hpp"
#include "RegConfigWindow.hpp"
#include <iostream>

void convertRmtToSvd(const QString &inputFile, const QString &outputFile)
{
    RegMapTreeModel model;
    RegConfigWindow config;

    // Load Protobuf RMT file
    FormatResult loadRes = FormatManager::instance().loadFile(inputFile, &model, &config);
    if (!loadRes.success) {
        std::cerr << "Failed to load: " << loadRes.errorMessage.toStdString() << std::endl;
        return;
    }

    // Save as ARM CMSIS-SVD XML
    FormatResult saveRes = FormatManager::instance().saveFile(outputFile, &model, &config);
    if (!saveRes.success) {
        std::cerr << "Failed to convert: " << saveRes.errorMessage.toStdString() << std::endl;
    }
}
```
