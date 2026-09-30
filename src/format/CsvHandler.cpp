/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "CsvHandler.hpp"
#include "../LockParser.hpp"
#include "../RegConfigWindow.hpp"
#include "../RegMapTreeItem.hpp"
#include "../RegMapTreeModel.hpp"
#include <QFile>
#include <QMap>
#include <QTextStream>

namespace {

inline bool isNonEmptyRecord(const QStringList &rec) {
  if (rec.isEmpty())
    return false;
  return rec.size() > 1 || !rec[0].isEmpty();
}

QString escapeCsv(const QString &field, char16_t delimiter) {
  bool needsQuotes = false;
  for (const QChar &ch : field) {
    char16_t u = ch.unicode();
    if (u == delimiter || u == '"' || u == '\n' || u == '\r') {
      needsQuotes = true;
      break;
    }
  }
  if (needsQuotes) {
    QString escaped = field;
    escaped.replace('"', "\"\"");
    return "\"" + escaped + "\"";
  }
  return field;
}

std::vector<QStringList> parseCsvLines(const QString &content,
                                       char16_t delimiter) {
  std::vector<QStringList> records;
  QStringList currentRecord;
  QString currentField;
  bool inQuotes = false;

  for (qsizetype i = 0; i < content.size(); ++i) {
    char16_t c = content[i].unicode();

    if (inQuotes) {
      if (c == '"') {
        if (i + 1 < content.size() && content[i + 1] == '"') {
          currentField += '"';
          i++; // skip escaped quote
        } else {
          inQuotes = false;
        }
      } else {
        currentField += c;
      }
    } else {
      if (c == '"') {
        inQuotes = true;
      } else if (c == delimiter) {
        currentRecord.append(currentField.trimmed());
        currentField.clear();
      } else if (c == '\n' || c == '\r') {
        if (c == '\r' && i + 1 < content.size() && content[i + 1] == '\n') {
          i++; // skip \n in CRLF
        }
        currentRecord.append(currentField.trimmed());
        currentField.clear();
        if (isNonEmptyRecord(currentRecord)) {
          records.push_back(currentRecord);
        }
        currentRecord.clear();
      } else {
        currentField += c;
      }
    }
  }

  if (!currentField.isEmpty() || !currentRecord.isEmpty()) {
    currentRecord.append(currentField.trimmed());
    if (isNonEmptyRecord(currentRecord)) {
      records.push_back(currentRecord);
    }
  }

  return records;
}

} // anonymous namespace

QString CsvHandler::formatName() const {
  return QStringLiteral("CSV Spreadsheet");
}
QStringList CsvHandler::supportedExtensions() const {
  static const QStringList exts = {QStringLiteral("csv"),
                                   QStringLiteral("tsv")};
  return exts;
}
QString CsvHandler::fileFilter() const {
  return QStringLiteral("CSV Table (*.csv *.tsv)");
}

FormatResult CsvHandler::read(const QString &filepath, RegMapTreeModel *model,
                              RegConfigWindow *config) {
  FormatResult result;
  QFile file(filepath);
  if (!file.open(QIODevice::ReadOnly)) {
    result.success = false;
    result.errorMessage =
        QString("Cannot open CSV file: %1").arg(file.errorString());
    return result;
  }

  QString content = QTextStream(&file).readAll();
  file.close();

  char16_t delimiter =
      filepath.endsWith(".tsv", Qt::CaseInsensitive) ? u'\t' : u',';
  auto rows = parseCsvLines(content, delimiter);
  if (rows.empty()) {
    result.success = false;
    result.errorMessage = "CSV file is empty.";
    return result;
  }

  // Column definitions
  QVector<QString> cols = {
      "Type",        "Offset/LSB",  "Size/Width", "Name",       "SW Access",
      "HW Access",   "Reset Value", "Is Rand",    "Volatile",   "Has Reset",
      "Description", "Write Lock",  "Read Lock",  "Decode Only"};
  QVariantMap rootData;
  for (const QString &c : cols)
    rootData[c] = c;
  rootData["Lock"] = "Write Lock";
  RegMapTreeItem *rootItem =
      new RegMapTreeItem(RegMapTreeItem::e_rmmKind::root, rootData);

  QHash<QString, RegMapTreeItem *> blockMap;
  QHash<QString, RegMapTreeItem *> regMap;

  // Header column index mapping
  int colType = 0, colBlk = 1, colReg = 2, colFld = 3, colOffset = 4,
      colWidth = 5;
  int colAccess = 6, colReset = 7, colRand = 8, colVol = 9, colHasReset = 10,
      colDesc = 11;
  int colWrLock = 12, colRdLock = 13, colLegacyLock = -1, colDecodeOnly = -1;

  size_t startRow = 0;
  if (!rows.empty() &&
      (rows[0][0].toLower() == "type" || rows[0][0].toLower() == "block")) {
    startRow = 1;
    const QStringList &hdr = rows[0];
    for (int i = 0; i < hdr.size(); ++i) {
      QString h = hdr[i].toLower().trimmed();
      if (h == "type")
        colType = i;
      else if (h == "block")
        colBlk = i;
      else if (h == "register")
        colReg = i;
      else if (h == "field")
        colFld = i;
      else if (h == "offset/lsb" || h == "offset" || h == "lsb")
        colOffset = i;
      else if (h == "width" || h == "size/width" || h == "size")
        colWidth = i;
      else if (h == "access" || h == "sw access")
        colAccess = i;
      else if (h == "reset" || h == "reset value")
        colReset = i;
      else if (h == "isrand" || h == "is rand")
        colRand = i;
      else if (h == "volatile")
        colVol = i;
      else if (h == "hasreset" || h == "has reset")
        colHasReset = i;
      else if (h == "description" || h == "desc")
        colDesc = i;
      else if (h == "write lock" || h == "writelock" || h == "lock_wr")
        colWrLock = i;
      else if (h == "read lock" || h == "readlock" || h == "lock_rd")
        colRdLock = i;
      else if (h == "decode only" || h == "decode_only" || h == "decodeonly")
        colDecodeOnly = i;
      else if (h == "lock")
        colLegacyLock = i;
    }
    if (colWrLock == -1 && colLegacyLock != -1) {
      colWrLock = colLegacyLock;
    }
  }

  for (size_t r = startRow; r < rows.size(); ++r) {
    const QStringList &row = rows[r];
    if (row.size() < 4)
      continue;

    auto getCol = [&](int idx, const QString &defVal = QString()) -> QString {
      if (idx >= 0 && idx < row.size())
        return row[idx].trimmed();
      return defVal;
    };

    QString type = getCol(colType).toLower();
    QString blkName = getCol(colBlk, "TOP_BLK");
    QString regName = getCol(colReg);
    QString fldName = getCol(colFld);
    QString offsetLsb = getCol(colOffset, "0");
    QString width = getCol(colWidth, "1");
    QString access = getCol(colAccess, "RW");
    QString reset = getCol(colReset, "0x0");
    QString isRand = getCol(colRand, "true");
    QString isVol = getCol(colVol, "false");
    QString hasReset = getCol(colHasReset, "true");
    QString desc = getCol(colDesc);
    QString wrLock = getCol(colWrLock);
    QString rdLock = getCol(colRdLock);
    if (wrLock.isEmpty() && colLegacyLock >= 0) {
      wrLock = getCol(colLegacyLock);
    }
    QString decodeOnly = getCol(colDecodeOnly, "false");

    if (blkName.isEmpty())
      blkName = "TOP_BLK";

    // Ensure Block exists
    RegMapTreeItem *blkItem = nullptr;
    if (!blockMap.contains(blkName)) {
      QVariantMap blkData;
      blkData["Type"] = "blk";
      blkData["Offset/LSB"] = "0x0";
      blkData["Name"] = blkName;
      blkData["Description"] = "";
      blkItem =
          new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, blkData, rootItem);
      rootItem->appendChild(blkItem);
      blockMap[blkName] = blkItem;
    } else {
      blkItem = blockMap[blkName];
    }

    if (type == "blk") {
      if (!wrLock.isEmpty()) {
        blkItem->setData("Write Lock", wrLock);
        blkItem->setData("Lock", wrLock);
      }
      if (!rdLock.isEmpty()) {
        blkItem->setData("Read Lock", rdLock);
        if (wrLock.isEmpty()) {
          blkItem->setData("Lock", LockParser::formatLockString(LockScope::Read,
                                                                "", rdLock));
        }
      }
      if (!wrLock.isEmpty() && !rdLock.isEmpty()) {
        blkItem->setData("Lock", LockParser::formatLockString(
                                     LockScope::Independent, wrLock, rdLock));
      }
      continue;
    }

    // Ensure Register exists
    QString regKey = blkName + "::" + regName;
    RegMapTreeItem *regItem = nullptr;
    if (!regName.isEmpty()) {
      if (!regMap.contains(regKey)) {
        QVariantMap regData;
        regData["Type"] = "reg";
        regData["Offset/LSB"] = (type == "reg") ? offsetLsb : "0x0";
        QString regWidth = (type == "reg" && width.toUInt() > 0)
                               ? width
                               : (config && config->registerWidth() > 0
                                      ? QString::number(config->registerWidth())
                                      : "32");
        regData["Size/Width"] = regWidth;
        regData["Name"] = regName;
        regData["SW Access"] = access;
        regData["HW Access"] = "RO";
        regData["Reset Value"] = "0x0";
        regData["Description"] = (type == "reg") ? desc : "";
        regData["Decode Only"] = (type == "reg") ? decodeOnly : "false";
        if (type == "reg") {
          if (!wrLock.isEmpty()) {
            regData["Write Lock"] = wrLock;
            regData["Lock"] = wrLock;
          }
          if (!rdLock.isEmpty()) {
            regData["Read Lock"] = rdLock;
          }
        }
        regItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, regData,
                                     blkItem);
        blkItem->appendChild(regItem);
        regMap[regKey] = regItem;
      } else {
        regItem = regMap[regKey];
      }
    }

    if (type == "fld" && regItem && !fldName.isEmpty()) {
      QVariantMap fldData;
      fldData["Type"] = "fld";
      fldData["Offset/LSB"] = offsetLsb;
      fldData["Size/Width"] = width;
      fldData["Name"] = fldName;
      fldData["SW Access"] = access;
      fldData["HW Access"] = (access == "RO") ? "WO" : "RO";
      fldData["Reset Value"] = reset;
      fldData["Is Rand"] = isRand;
      fldData["Volatile"] = isVol;
      fldData["Has Reset"] = hasReset;
      fldData["Decode Only"] = decodeOnly;
      fldData["Description"] = desc;
      if (!wrLock.isEmpty()) {
        fldData["Write Lock"] = wrLock;
        fldData["Lock"] = wrLock;
      }
      if (!rdLock.isEmpty()) {
        fldData["Read Lock"] = rdLock;
      }
      RegMapTreeItem *fldItem =
          new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, fldData, regItem);
      regItem->appendChild(fldItem);
    }
  }

  if (model) {
    model->setRootItem(rootItem);
  }

  result.success = true;
  return result;
}

FormatResult CsvHandler::write(const QString &filepath, RegMapTreeModel *model,
                               RegConfigWindow *config) {
  FormatResult result;
  if (!model || !model->getRootItem()) {
    result.success = false;
    result.errorMessage = "No register map model available to export.";
    return result;
  }

  QFile file(filepath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    result.success = false;
    result.errorMessage =
        QString("Cannot write to CSV file: %1").arg(file.errorString());
    return result;
  }

  char16_t delimiter =
      filepath.endsWith(".tsv", Qt::CaseInsensitive) ? u'\t' : u',';
  QTextStream out(&file);

  // Header row
  QStringList headers = {"Type",       "Block",     "Register", "Field",
                         "Offset/LSB", "Width",     "Access",   "Reset",
                         "IsRand",     "Volatile",  "HasReset", "Description",
                         "Write Lock", "Read Lock", "Lock",     "Decode Only"};
  for (int i = 0; i < headers.size(); ++i) {
    out << escapeCsv(headers[i], delimiter)
        << (i + 1 < headers.size() ? QChar(delimiter) : QChar('\n'));
  }

  RegMapTreeItem *root = model->getRootItem();
  for (RegMapTreeItem *blk : root->getChildItems()) {
    if (!blk || blk->kind() != RegMapTreeItem::e_rmmKind::blk)
      continue;

    QString blkName = blk->data("Name").toString().trimmed();
    QString blkOffset = blk->data("Offset/LSB").toString().trimmed();
    QString blkDesc = blk->data("Description").toString().trimmed();
    QString blkWrLock = blk->data("Write Lock").toString().trimmed();
    QString blkRdLock = blk->data("Read Lock").toString().trimmed();
    QString blkLegacyLock = blk->data("Lock").toString().trimmed();
    if (blkWrLock.isEmpty() && !blkLegacyLock.isEmpty())
      blkWrLock = blkLegacyLock;

    if (!blkWrLock.isEmpty() || !blkRdLock.isEmpty()) {
      QStringList blkRow = {"blk",     blkName,   "",        "",
                            blkOffset, "",        "",        "",
                            "false",   "false",   "false",   blkDesc,
                            blkWrLock, blkRdLock, blkWrLock, "false"};
      for (int i = 0; i < blkRow.size(); ++i) {
        out << escapeCsv(blkRow[i], delimiter)
            << (i + 1 < blkRow.size() ? QChar(delimiter) : QChar('\n'));
      }
    }

    for (RegMapTreeItem *reg : blk->getChildItems()) {
      if (!reg || reg->kind() != RegMapTreeItem::e_rmmKind::reg)
        continue;

      QString regName = reg->data("Name").toString().trimmed();
      QString regOffset = reg->data("Offset/LSB").toString().trimmed();
      QString regDesc = reg->data("Description").toString().trimmed();
      QString regWrLock = reg->data("Write Lock").toString().trimmed();
      QString regRdLock = reg->data("Read Lock").toString().trimmed();
      QString regLegacyLock = reg->data("Lock").toString().trimmed();
      if (regWrLock.isEmpty() && !regLegacyLock.isEmpty())
        regWrLock = regLegacyLock;
      QString regWidth = reg->data("Size/Width").toString().trimmed();
      if (regWidth.isEmpty() || regWidth.toUInt() == 0) {
        regWidth = (config && config->registerWidth() > 0)
                       ? QString::number(config->registerWidth())
                       : "32";
      }
      QString regDecodeOnly = reg->data("Decode Only").toString().trimmed();
      if (regDecodeOnly.isEmpty())
        regDecodeOnly = "false";

      QStringList regRow = {"reg",     blkName,   regName,   "",
                            regOffset, regWidth,  "RW",      "0x0",
                            "false",   "false",   "false",   regDesc,
                            regWrLock, regRdLock, regWrLock, regDecodeOnly};
      for (int i = 0; i < regRow.size(); ++i) {
        out << escapeCsv(regRow[i], delimiter)
            << (i + 1 < regRow.size() ? QChar(delimiter) : QChar('\n'));
      }

      auto fields = reg->getChildItems();
      for (RegMapTreeItem *fld : fields) {
        if (!fld || fld->kind() != RegMapTreeItem::e_rmmKind::fld)
          continue;

        QString fldName = fld->data("Name").toString().trimmed();
        QString lsb = fld->data("Offset/LSB").toString().trimmed();
        QString width = fld->data("Size/Width").toString().trimmed();
        QString access = fld->data("SW Access").toString().trimmed();
        QString reset = fld->data("Reset Value").toString().trimmed();
        QString isRand = fld->data("Is Rand").toString().trimmed();
        QString isVol = fld->data("Volatile").toString().trimmed();
        QString hasReset = fld->data("Has Reset").toString().trimmed();
        QString fldDesc = fld->data("Description").toString().trimmed();
        QString fldWrLock = fld->data("Write Lock").toString().trimmed();
        QString fldRdLock = fld->data("Read Lock").toString().trimmed();
        QString fldLegacyLock = fld->data("Lock").toString().trimmed();
        if (fldWrLock.isEmpty() && !fldLegacyLock.isEmpty())
          fldWrLock = fldLegacyLock;
        QString fldDecodeOnly = fld->data("Decode Only").toString().trimmed();
        if (fldDecodeOnly.isEmpty())
          fldDecodeOnly = "false";

        QStringList row = {"fld",     blkName,   regName,   fldName,
                           lsb,       width,     access,    reset,
                           isRand,    isVol,     hasReset,  fldDesc,
                           fldWrLock, fldRdLock, fldWrLock, fldDecodeOnly};
        for (int i = 0; i < row.size(); ++i) {
          out << escapeCsv(row[i], delimiter)
              << (i + 1 < row.size() ? QChar(delimiter) : QChar('\n'));
        }
      }
    }
  }

  file.close();
  result.success = true;
  return result;
}
