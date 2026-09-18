/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "FormatManager.hpp"
#include "CmsisSvdHandler.hpp"
#include "CsvHandler.hpp"
#include "IpxactHandler.hpp"
#include "JsonHandler.hpp"
#include "PathUtils.hpp"
#include "ProtobufHandler.hpp"
#include "SystemRdlHandler.hpp"
#include <QFile>
#include <QFileInfo>

IFormatHandler::~IFormatHandler() = default;

FormatManager &FormatManager::instance() {
  static FormatManager s_instance;
  return s_instance;
}

FormatManager::FormatManager() { registerDefaultHandlers(); }

void FormatManager::registerDefaultHandlers() {
  registerHandler(std::make_shared<ProtobufHandler>());
  registerHandler(std::make_shared<SystemRdlHandler>());
  registerHandler(std::make_shared<IpxactHandler>());
  registerHandler(std::make_shared<CmsisSvdHandler>());
  registerHandler(std::make_shared<JsonHandler>());
  registerHandler(std::make_shared<CsvHandler>());
}

void FormatManager::clearHandlers() { m_handlers.clear(); }

void FormatManager::registerHandler(std::shared_ptr<IFormatHandler> handler) {
  if (handler && m_handlers.size() < 128) {
    m_handlers.push_back(handler);
  }
}

const std::vector<std::shared_ptr<IFormatHandler>> &
FormatManager::handlers() const {
  return m_handlers;
}

std::shared_ptr<IFormatHandler>
FormatManager::handlerForFile(const QString &filepath) const {
  QString expanded = PathUtils::expandEnvVars(filepath);
  QFileInfo info(expanded);
  QString ext = info.suffix().toLower();

  // 1. If extension is ambiguous (.xml), missing, or unrecognized, perform
  // content inspection
  const bool isXml = (ext == QStringLiteral("xml"));
  const bool isKnownUnambiguous =
      (!isXml && !ext.isEmpty() &&
       (ext == QStringLiteral("svd") || ext == QStringLiteral("rdl") ||
        ext == QStringLiteral("systemrdl") || ext == QStringLiteral("ipxact") ||
        ext == QStringLiteral("json") || ext == QStringLiteral("csv") ||
        ext == QStringLiteral("tsv") || ext == QStringLiteral("rmt") ||
        ext == QStringLiteral("rmb")));

  if (!isKnownUnambiguous) {
    QFile file(expanded);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
      QByteArray header = file.read(4096);
      file.close();

      // (a) XML format disambiguation (CMSIS-SVD vs. IP-XACT)
      if (header.contains("<device") || header.contains("<device>") ||
          header.contains("<device ") || header.contains("<device\n") ||
          header.contains("<device\r")) {
        auto h = handlerByName(QStringLiteral("CMSIS-SVD"));
        if (h) {
          return h;
        }
      }
      if (header.contains("<ipxact:") || header.contains("<spirit:") ||
          header.contains("http://www.accellera.org/XMLSchema/IPXACT") ||
          header.contains("http://www.spiritconsortium.org/XMLSchema/SPIRIT")) {
        auto h = handlerByName(QStringLiteral("IP-XACT"));
        if (h) {
          return h;
        }
      }

      // (b) JSON schema format
      QByteArray trimmed = header.trimmed();
      if ((trimmed.startsWith('{') || trimmed.startsWith('[')) &&
          (header.contains("\"blocks\"") || header.contains("\"registers\"") ||
           header.contains("\"project_name\"") ||
           header.contains("\"regmap\""))) {
        auto h = handlerByName(QStringLiteral("JSON"));
        if (h) {
          return h;
        }
      }

      // (c) SystemRDL format
      if (header.contains("addrmap ") || header.contains("addrmap\t") ||
          header.contains("addrmap\n") || header.contains("regfile ") ||
          header.contains("// SystemRDL") || header.contains("/* SystemRDL")) {
        auto h = handlerByName(QStringLiteral("SystemRDL"));
        if (h) {
          return h;
        }
      }

      // (d) CSV / TSV format
      if (trimmed.startsWith("Type,Offset") ||
          trimmed.startsWith("\"Type\",\"Offset\"") ||
          trimmed.startsWith("Offset,Size,Name") ||
          trimmed.startsWith("Type\tOffset")) {
        auto h = handlerByName(QStringLiteral("CSV"));
        if (h) {
          return h;
        }
      }

      // (e) Protobuf Text Format
      if (header.contains("config {") || header.contains("item {") ||
          header.contains("reg_model {") ||
          (header.contains("blocks {") && !trimmed.startsWith('{'))) {
        auto h = handlerByName(QStringLiteral("Protobuf"));
        if (h) {
          return h;
        }
      }
    }
  }

  // 2. Extension-based matching
  for (const auto &handler : m_handlers) {
    for (const QString &supported : handler->supportedExtensions()) {
      if (supported.toLower() == ext) {
        return handler;
      }
    }
  }

  // 3. Default fallback to ProtobufHandler if unknown
  for (const auto &handler : m_handlers) {
    if (std::dynamic_pointer_cast<ProtobufHandler>(handler)) {
      return handler;
    }
  }

  return nullptr;
}

std::shared_ptr<IFormatHandler>
FormatManager::handlerByName(const QString &name) const {
  for (const auto &handler : m_handlers) {
    if (handler->formatName().contains(name, Qt::CaseInsensitive)) {
      return handler;
    }
  }
  return nullptr;
}

FormatResult FormatManager::loadFile(const QString &filepath,
                                     RegMapTreeModel *model,
                                     RegConfigWindow *config) {
  QString expanded = PathUtils::expandEnvVars(filepath);
  auto handler = handlerForFile(expanded);
  if (!handler) {
    return {false,
            QString("No format handler found for file: %1").arg(filepath),
            {}};
  }
  return handler->read(expanded, model, config);
}

FormatResult FormatManager::saveFile(const QString &filepath,
                                     RegMapTreeModel *model,
                                     RegConfigWindow *config) {
  QString expanded = PathUtils::expandEnvVars(filepath);
  auto handler = handlerForFile(expanded);
  if (!handler) {
    return {false,
            QString("No format handler found for file: %1").arg(filepath),
            {}};
  }
  return handler->write(expanded, model, config);
}

QString FormatManager::allFilterString() const {
  QStringList allExts;
  QStringList filterParts;

  for (const auto &handler : m_handlers) {
    for (const QString &ext : handler->supportedExtensions()) {
      QString pattern = QString("*.%1").arg(ext);
      if (!allExts.contains(pattern)) {
        allExts.append(pattern);
      }
    }
    filterParts.append(handler->fileFilter());
  }

  QString composite =
      QString("All Supported Formats (%1)").arg(allExts.join(" "));
  filterParts.prepend(composite);
  return filterParts.join(";;");
}
