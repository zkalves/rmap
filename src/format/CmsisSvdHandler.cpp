/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "CmsisSvdHandler.hpp"
#include "../RegConfigWindow.hpp"
#include "../RegMapTreeItem.hpp"
#include "../RegMapTreeModel.hpp"
#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

CmsisSvdHandler::~CmsisSvdHandler() = default;
QString CmsisSvdHandler::formatName() const {
  return QStringLiteral("ARM CMSIS-SVD");
}
QStringList CmsisSvdHandler::supportedExtensions() const {
  return {QStringLiteral("svd"), QStringLiteral("xml")};
}
QString CmsisSvdHandler::fileFilter() const {
  return QStringLiteral("ARM CMSIS-SVD (*.svd *.xml)");
}

namespace {

QString svdAccessToUvm(const QString &acc) {
  QString a = acc.toLower().trimmed();
  if (a == "read-write" || a == "rw")
    return "RW";
  if (a == "read-only" || a == "ro" || a == "r")
    return "RO";
  if (a == "write-only" || a == "wo" || a == "w")
    return "WO";
  if (a == "writeonce" || a == "wo1")
    return "WO1";
  if (a == "read-writeonce" || a == "w1")
    return "W1";
  if (a == "w1c")
    return "W1C";
  if (a == "w0c")
    return "W0C";
  if (a == "w1s")
    return "W1S";
  if (a == "w0s")
    return "W0S";
  if (a == "w1t")
    return "W1T";
  if (a == "w0t")
    return "W0T";
  if (a == "rc")
    return "RC";
  if (a == "rs")
    return "RS";
  if (a == "wrc")
    return "WRC";
  if (a == "wrs")
    return "WRS";
  if (a == "wc")
    return "WC";
  if (a == "ws")
    return "WS";
  if (a == "w1src")
    return "W1SRC";
  if (a == "w1crs")
    return "W1CRS";
  if (a == "w0src")
    return "W0SRC";
  if (a == "w0crs")
    return "W0CRS";
  if (a == "woc")
    return "WOC";
  if (a == "wos")
    return "WOS";
  if (a == "noaccess" || a == "na")
    return "NOACCESS";
  return "RW";
}

QString svdPropsToUvm(const QString &acc, const QString &mwv,
                      const QString &ra) {
  QString m = mwv.toLower().trimmed();
  QString r = ra.toLower().trimmed();
  QString a = acc.toLower().trimmed();

  if (m == "onetoclear") {
    if (r == "set")
      return "W1CRS";
    return "W1C";
  }
  if (m == "zerotoclear") {
    if (r == "set")
      return "W0CRS";
    return "W0C";
  }
  if (m == "onetoset") {
    if (r == "clear")
      return "W1SRC";
    return "W1S";
  }
  if (m == "zerotoset") {
    if (r == "clear")
      return "W0SRC";
    return "W0S";
  }
  if (m == "onetotoggle")
    return "W1T";
  if (m == "zerototoggle")
    return "W0T";
  if (m == "clear") {
    if (a == "write-only" || a == "wo")
      return "WOC";
    return "WC";
  }
  if (m == "set") {
    if (a == "write-only" || a == "wo")
      return "WOS";
    return "WS";
  }

  if (r == "clear") {
    if (a == "read-write" || a == "rw")
      return "WRC";
    return "RC";
  }
  if (r == "set") {
    if (a == "read-write" || a == "rw")
      return "WRS";
    return "RS";
  }

  if (a == "read-writeonce")
    return "W1";
  if (a == "writeonce")
    return "WO1";

  return svdAccessToUvm(acc);
}

QString uvmAccessToSvd(const QString &acc) {
  QString a = acc.toUpper().trimmed();
  if (a == "RO" || a == "RC" || a == "RS")
    return "read-only";
  if (a == "WO" || a == "WOC" || a == "WOS")
    return "write-only";
  if (a == "WO1")
    return "writeOnce";
  if (a == "W1")
    return "read-writeOnce";
  if (a == "NOACCESS")
    return "read-only";
  return "read-write";
}

QString uvmToSvdModifiedWriteValues(const QString &acc) {
  QString a = acc.toUpper().trimmed();
  if (a == "W1C" || a == "W1CRS")
    return "oneToClear";
  if (a == "W0C" || a == "W0CRS")
    return "zeroToClear";
  if (a == "W1S" || a == "W1SRC")
    return "oneToSet";
  if (a == "W0S" || a == "W0SRC")
    return "zeroToSet";
  if (a == "W1T")
    return "oneToToggle";
  if (a == "W0T")
    return "zeroToToggle";
  if (a == "WC" || a == "WOC")
    return "clear";
  if (a == "WS" || a == "WOS")
    return "set";
  return QString();
}

QString uvmToSvdReadAction(const QString &acc) {
  QString a = acc.toUpper().trimmed();
  if (a == "RC" || a == "WRC" || a == "W1SRC" || a == "W0SRC")
    return "clear";
  if (a == "RS" || a == "WRS" || a == "W1CRS" || a == "W0CRS")
    return "set";
  return QString();
}

uint64_t parseSvdNum(const QString &str) {
  QString s = str.trimmed().toLower();
  if (s.startsWith("0x"))
    return s.mid(2).toULongLong(nullptr, 16);
  if (s.startsWith("#"))
    return s.mid(1).toULongLong(nullptr, 16);
  if (s.startsWith("0b"))
    return s.mid(2).toULongLong(nullptr, 2);
  return s.toULongLong(nullptr, 10);
}

RegMapTreeItem *cloneTreeItem(const RegMapTreeItem *src,
                              RegMapTreeItem *parent) {
  if (!src)
    return nullptr;
  static const QVector<QString> cols = {
      "Type",        "Offset/LSB",    "Size/Width", "Name",     "SW Access",
      "HW Access",   "Reset Value",   "Is Rand",    "Volatile", "Has Reset",
      "Description", "Access Policy", "Offset",     "Size"};
  QVariantMap data;
  for (const QString &col : cols) {
    QVariant val = src->data(col);
    if (val.isValid() && !val.isNull()) {
      data[col] = val;
    }
  }
  RegMapTreeItem *clone = new RegMapTreeItem(src->kind(), data, parent);
  for (RegMapTreeItem *child : src->getChildItems()) {
    RegMapTreeItem *childClone = cloneTreeItem(child, clone);
    if (childClone) {
      clone->appendChild(childClone);
    }
  }
  return clone;
}

QStringList parseDimIndex(const QString &dimIndexStr, uint32_t dim) {
  QStringList result;
  QString s = dimIndexStr.trimmed();
  if (!s.isEmpty()) {
    if (s.contains(',')) {
      QStringList parts = s.split(',', Qt::SkipEmptyParts);
      for (const QString &p : parts) {
        result.append(p.trimmed());
      }
    } else if (s.contains('-')) {
      QStringList parts = s.split('-');
      if (parts.size() == 2) {
        QString startStr = parts[0].trimmed();
        QString endStr = parts[1].trimmed();
        bool okStart = false, okEnd = false;
        qlonglong startNum = startStr.toLongLong(&okStart);
        qlonglong endNum = endStr.toLongLong(&okEnd);
        if (okStart && okEnd && startNum <= endNum) {
          for (qlonglong n = startNum; n <= endNum; ++n) {
            result.append(QString::number(n));
          }
        } else if (startStr.length() == 1 && endStr.length() == 1 &&
                   startStr[0] <= endStr[0]) {
          char startChar = startStr[0].toLatin1();
          char endChar = endStr[0].toLatin1();
          for (char c = startChar; c <= endChar; ++c) {
            result.append(QString(QChar(c)));
          }
        }
      }
    } else {
      result.append(s);
    }
  }
  for (uint32_t i = result.size(); i < dim; ++i) {
    result.append(QString::number(i));
  }
  return result;
}

} // anonymous namespace

FormatResult CmsisSvdHandler::read(const QString &filepath,
                                   RegMapTreeModel *model,
                                   RegConfigWindow *config) {
  FormatResult result;
  QFile file(filepath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    result.success = false;
    result.errorMessage =
        QString("Cannot open CMSIS-SVD file: %1").arg(file.errorString());
    return result;
  }

  QXmlStreamReader xml(&file);

  QVector<QString> cols = {"Type",      "Offset/LSB", "Size/Width",  "Name",
                           "SW Access", "HW Access",  "Reset Value", "Is Rand",
                           "Volatile",  "Has Reset",  "Description"};
  QVariantMap rootData;
  for (const QString &c : cols)
    rootData[c] = c;
  RegMapTreeItem *rootItem =
      new RegMapTreeItem(RegMapTreeItem::e_rmmKind::root, rootData);

  RegMapTreeItem *currentBlock = nullptr;
  RegMapTreeItem *currentReg = nullptr;
  RegMapTreeItem *currentField = nullptr;

  QString currentRegAccess;
  QString currentRegMwv;
  QString currentRegReadAction;

  QString currentFieldAccess;
  QString currentFieldMwv;
  QString currentFieldReadAction;

  uint32_t currentRegDim = 1;
  uint64_t currentRegDimInc = 0;
  QString currentRegDimIndex;
  QVector<QPair<RegMapTreeItem *, QString>> pendingDerived;

  QString deviceName = "MCU_Device";
  uint32_t globalWidth = 32;

  while (!xml.atEnd() && !xml.hasError()) {
    QXmlStreamReader::TokenType token = xml.readNext();
    if (token == QXmlStreamReader::StartElement) {
      QString name = xml.name().toString();

      if (name == "device") {
        // Device root
      } else if (name == "name" && !currentBlock && !currentReg &&
                 !currentField) {
        deviceName = xml.readElementText();
      } else if (name == "size" && !currentBlock && !currentReg &&
                 !currentField) {
        uint32_t w = xml.readElementText().toUInt();
        if (w > 0)
          globalWidth = w;
      } else if (name == "peripheral") {
        // Block container
        QVariantMap blkData;
        blkData["Type"] = "blk";
        blkData["Offset/LSB"] = "0x0";
        blkData["Name"] = "PERIPHERAL";
        blkData["Description"] = "";
        currentBlock = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk,
                                          blkData, rootItem);
        rootItem->appendChild(currentBlock);

        if (xml.attributes().hasAttribute("derivedFrom")) {
          QString target =
              xml.attributes().value("derivedFrom").toString().trimmed();
          if (!target.isEmpty()) {
            pendingDerived.append({currentBlock, target});
          }
        }
      } else if (name == "name" && currentBlock && !currentReg &&
                 !currentField) {
        currentBlock->setData("Name", xml.readElementText());
      } else if (name == "baseAddress" && currentBlock && !currentReg &&
                 !currentField) {
        QString addr = xml.readElementText();
        currentBlock->setData("Offset/LSB",
                              QString("0x%1").arg(parseSvdNum(addr), 0, 16));
      } else if (name == "description" && currentBlock && !currentReg &&
                 !currentField) {
        currentBlock->setData("Description", xml.readElementText());
      } else if (name == "register") {
        // Register container
        currentRegDim = 1;
        currentRegDimInc = 0;
        currentRegDimIndex.clear();
        currentRegAccess.clear();
        currentRegMwv.clear();
        currentRegReadAction.clear();

        QVariantMap regData;
        regData["Type"] = "reg";
        regData["Offset/LSB"] = "0x0";
        regData["Size/Width"] = QString::number(globalWidth);
        regData["Name"] = "REG";
        regData["SW Access"] = "RW";
        regData["HW Access"] = "RO";
        regData["Reset Value"] = "0x0";
        regData["Description"] = "";
        currentReg = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, regData,
                                        currentBlock ? currentBlock : rootItem);
        if (currentBlock)
          currentBlock->appendChild(currentReg);
        else
          rootItem->appendChild(currentReg);
      } else if (name == "dim" && currentReg && !currentField) {
        currentRegDim = xml.readElementText().toUInt();
      } else if (name == "dimIncrement" && currentReg && !currentField) {
        currentRegDimInc = parseSvdNum(xml.readElementText());
      } else if (name == "dimIndex" && currentReg && !currentField) {
        currentRegDimIndex = xml.readElementText().trimmed();
      } else if (name == "name" && currentReg && !currentField) {
        currentReg->setData("Name", xml.readElementText());
      } else if (name == "addressOffset" && currentReg && !currentField) {
        QString off = xml.readElementText();
        currentReg->setData("Offset/LSB",
                            QString("0x%1").arg(parseSvdNum(off), 0, 16));
      } else if (name == "size" && currentReg && !currentField) {
        currentReg->setData("Size/Width", xml.readElementText());
      } else if (name == "access" && currentReg && !currentField) {
        currentRegAccess = xml.readElementText();
        currentReg->setData("SW Access",
                            svdPropsToUvm(currentRegAccess, currentRegMwv,
                                          currentRegReadAction));
      } else if (name == "modifiedWriteValues" && currentReg && !currentField) {
        currentRegMwv = xml.readElementText();
        currentReg->setData("SW Access",
                            svdPropsToUvm(currentRegAccess, currentRegMwv,
                                          currentRegReadAction));
      } else if (name == "readAction" && currentReg && !currentField) {
        currentRegReadAction = xml.readElementText();
        currentReg->setData("SW Access",
                            svdPropsToUvm(currentRegAccess, currentRegMwv,
                                          currentRegReadAction));
      } else if (name == "resetValue" && currentReg && !currentField) {
        QString rv = xml.readElementText();
        currentReg->setData("Reset Value",
                            QString("0x%1").arg(parseSvdNum(rv), 0, 16));
      } else if (name == "description" && currentReg && !currentField) {
        currentReg->setData("Description", xml.readElementText());
      } else if (name == "field") {
        // Field container
        currentFieldAccess = currentRegAccess;
        currentFieldMwv = currentRegMwv;
        currentFieldReadAction = currentRegReadAction;

        QVariantMap fldData;
        fldData["Type"] = "fld";
        fldData["Offset/LSB"] = "0";
        fldData["Size/Width"] = "1";
        fldData["Name"] = "FIELD";
        fldData["SW Access"] =
            currentReg ? currentReg->data("SW Access").toString() : "RW";
        fldData["HW Access"] = "RO";
        fldData["Reset Value"] = "0x0";
        fldData["Is Rand"] = "true";
        fldData["Volatile"] = "false";
        fldData["Has Reset"] = "true";
        fldData["Description"] = "";
        currentField = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld,
                                          fldData, currentReg);
        if (currentReg)
          currentReg->appendChild(currentField);
      } else if (name == "name" && currentField) {
        currentField->setData("Name", xml.readElementText());
      } else if (name == "bitOffset" && currentField) {
        currentField->setData(
            "Offset/LSB", QString::number(parseSvdNum(xml.readElementText())));
      } else if (name == "bitWidth" && currentField) {
        currentField->setData(
            "Size/Width", QString::number(parseSvdNum(xml.readElementText())));
      } else if (name == "bitRange" && currentField) {
        // Format: [msb:lsb]
        QString br = xml.readElementText();
        QRegularExpression re(R"(\[(\d+):(\d+)\])");
        auto match = re.match(br);
        if (match.hasMatch()) {
          uint64_t msb = match.captured(1).toULongLong();
          uint64_t lsb = match.captured(2).toULongLong();
          currentField->setData("Offset/LSB", QString::number(lsb));
          currentField->setData("Size/Width", QString::number(msb - lsb + 1));
        }
      } else if (name == "access" && currentField) {
        currentFieldAccess = xml.readElementText();
        currentField->setData("SW Access",
                              svdPropsToUvm(currentFieldAccess, currentFieldMwv,
                                            currentFieldReadAction));
      } else if (name == "modifiedWriteValues" && currentField) {
        currentFieldMwv = xml.readElementText();
        currentField->setData("SW Access",
                              svdPropsToUvm(currentFieldAccess, currentFieldMwv,
                                            currentFieldReadAction));
      } else if (name == "readAction" && currentField) {
        currentFieldReadAction = xml.readElementText();
        currentField->setData("SW Access",
                              svdPropsToUvm(currentFieldAccess, currentFieldMwv,
                                            currentFieldReadAction));
      } else if (name == "description" && currentField) {
        currentField->setData("Description", xml.readElementText());
      }
    } else if (token == QXmlStreamReader::EndElement) {
      QString name = xml.name().toString();
      if (name == "field") {
        currentField = nullptr;
      } else if (name == "register") {
        if (currentReg) {
          if (currentRegDim == 0)
            currentRegDim = 1;
          if (currentRegDimInc == 0) {
            uint32_t rsize = currentReg->data("Size/Width").toUInt();
            if (rsize == 0)
              rsize = globalWidth;
            currentRegDimInc = (rsize + 7) / 8;
          }
          QStringList indices =
              parseDimIndex(currentRegDimIndex, currentRegDim);
          QString rawName = currentReg->data("Name").toString();
          QString rawDesc = currentReg->data("Description").toString();
          uint64_t baseOffset =
              parseSvdNum(currentReg->data("Offset/LSB").toString());

          QString idx0 = indices.value(0, "0");
          QString name0 = rawName.contains("%s")
                              ? QString(rawName).replace("%s", idx0)
                              : (currentRegDim > 1 ? rawName + idx0 : rawName);
          QString desc0 = rawDesc.contains("%s")
                              ? QString(rawDesc).replace("%s", idx0)
                              : rawDesc;
          currentReg->setData("Name", name0);
          currentReg->setData("Description", desc0);

          for (uint32_t i = 1; i < currentRegDim; ++i) {
            QString idx = indices.value(i, QString::number(i));
            QString nameI = rawName.contains("%s")
                                ? QString(rawName).replace("%s", idx)
                                : rawName + idx;
            QString descI = rawDesc.contains("%s")
                                ? QString(rawDesc).replace("%s", idx)
                                : rawDesc;
            uint64_t offI = baseOffset + i * currentRegDimInc;

            RegMapTreeItem *newReg = cloneTreeItem(
                currentReg, currentBlock ? currentBlock : rootItem);
            newReg->setData("Name", nameI);
            newReg->setData("Description", descI);
            newReg->setData("Offset/LSB", QString("0x%1").arg(offI, 0, 16));
            if (currentBlock)
              currentBlock->appendChild(newReg);
            else
              rootItem->appendChild(newReg);
          }
        }
        currentReg = nullptr;
      } else if (name == "peripheral") {
        currentBlock = nullptr;
      }
    }
  }

  if (xml.hasError()) {
    delete rootItem;
    result.success = false;
    result.errorMessage = QString("XML Parse Error: %1 (line %2)")
                              .arg(xml.errorString())
                              .arg(xml.lineNumber());
    return result;
  }

  // Resolve derived peripherals
  bool progress = true;
  int maxPasses = pendingDerived.size();
  while (progress && maxPasses-- > 0) {
    progress = false;
    for (const auto &pair : pendingDerived) {
      RegMapTreeItem *derivedBlk = pair.first;
      const QString &targetName = pair.second;
      RegMapTreeItem *srcBlk = nullptr;
      for (RegMapTreeItem *blk : rootItem->getChildItems()) {
        if (blk && blk != derivedBlk &&
            blk->data("Name").toString() == targetName) {
          srcBlk = blk;
          break;
        }
      }
      if (srcBlk) {
        if (derivedBlk->data("Description").toString().isEmpty() &&
            !srcBlk->data("Description").toString().isEmpty()) {
          derivedBlk->setData("Description", srcBlk->data("Description"));
        }
        for (RegMapTreeItem *srcReg : srcBlk->getChildItems()) {
          if (!srcReg || srcReg->kindString() != "reg")
            continue;
          bool exists = false;
          for (RegMapTreeItem *existingReg : derivedBlk->getChildItems()) {
            if (existingReg && existingReg->data("Name").toString() ==
                                   srcReg->data("Name").toString()) {
              exists = true;
              break;
            }
          }
          if (!exists) {
            RegMapTreeItem *clonedReg = cloneTreeItem(srcReg, derivedBlk);
            derivedBlk->appendChild(clonedReg);
            progress = true;
          }
        }
      }
    }
  }

  model->setRootItem(rootItem);

  if (config) {
    protormap::Config cfg;
    cfg.set_project_name(deviceName.toStdString());
    cfg.set_reg_width(globalWidth);
    config->deserialize(cfg);
  }

  result.success = true;
  return result;
}

FormatResult CmsisSvdHandler::write(const QString &filepath,
                                    RegMapTreeModel *model,
                                    RegConfigWindow *config) {
  FormatResult result;
  if (!model || !model->getRootItem()) {
    result.success = false;
    result.errorMessage = "No model data to export.";
    return result;
  }

  QFile file(filepath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    result.success = false;
    result.errorMessage =
        QString("Cannot open output file: %1").arg(file.errorString());
    return result;
  }

  uint32_t regWidth = 32;
  QString projectName = "CMSIS_Device";
  if (config) {
    protormap::Config *cfg = config->serialize();
    if (cfg->reg_width() > 0)
      regWidth = cfg->reg_width();
    if (!cfg->project_name().empty())
      projectName = QString::fromStdString(cfg->project_name());
    delete cfg;
  }

  QXmlStreamWriter xml(&file);
  xml.setAutoFormatting(true);
  xml.setAutoFormattingIndent(2);
  xml.writeStartDocument();

  xml.writeStartElement("device");
  xml.writeAttribute("schemaVersion", "1.3");
  xml.writeAttribute("xmlns:xs", "https://www.w3.org/2001/XMLSchema-instance");

  xml.writeTextElement("name", projectName);
  xml.writeTextElement("version", "1.0");
  xml.writeTextElement("description", "Auto-generated CMSIS-SVD from rmap");
  xml.writeTextElement("addressUnitBits", "8");
  xml.writeTextElement("width", QString::number(regWidth));
  xml.writeTextElement("size", QString::number(regWidth));
  xml.writeTextElement("resetValue", "0x00000000");
  xml.writeTextElement("resetMask", "0xFFFFFFFF");

  xml.writeStartElement("peripherals");

  RegMapTreeItem *root = model->getRootItem();
  for (RegMapTreeItem *blk : root->getChildItems()) {
    if (!blk || blk->kindString() != "blk")
      continue;

    xml.writeStartElement("peripheral");
    xml.writeTextElement("name", blk->data("Name").toString());
    xml.writeTextElement("description", blk->data("Description").toString());
    xml.writeTextElement("baseAddress", blk->data("Offset/LSB").toString());

    xml.writeStartElement("addressBlock");
    xml.writeTextElement("offset", "0x0");
    xml.writeTextElement("size", "0x1000");
    xml.writeTextElement("usage", "registers");
    xml.writeEndElement(); // addressBlock

    xml.writeStartElement("registers");
    for (RegMapTreeItem *reg : blk->getChildItems()) {
      if (!reg || reg->kindString() != "reg")
        continue;

      xml.writeStartElement("register");
      xml.writeTextElement("name", reg->data("Name").toString());
      xml.writeTextElement("description", reg->data("Description").toString());
      xml.writeTextElement("addressOffset", reg->data("Offset/LSB").toString());
      xml.writeTextElement("size", QString::number(regWidth));
      xml.writeTextElement("access",
                           uvmAccessToSvd(reg->data("SW Access").toString()));
      {
        QString mwv =
            uvmToSvdModifiedWriteValues(reg->data("SW Access").toString());
        if (!mwv.isEmpty())
          xml.writeTextElement("modifiedWriteValues", mwv);
        QString ra = uvmToSvdReadAction(reg->data("SW Access").toString());
        if (!ra.isEmpty())
          xml.writeTextElement("readAction", ra);
      }
      xml.writeTextElement("resetValue", reg->data("Reset Value").toString());

      if (reg->childCount() > 0) {
        xml.writeStartElement("fields");
        for (RegMapTreeItem *fld : reg->getChildItems()) {
          if (!fld || fld->kindString() != "fld")
            continue;

          xml.writeStartElement("field");
          xml.writeTextElement("name", fld->data("Name").toString());
          xml.writeTextElement("description",
                               fld->data("Description").toString());
          xml.writeTextElement("bitOffset", fld->data("Offset/LSB").toString());
          xml.writeTextElement("bitWidth", fld->data("Size/Width").toString());
          xml.writeTextElement(
              "access", uvmAccessToSvd(fld->data("SW Access").toString()));
          {
            QString mwv =
                uvmToSvdModifiedWriteValues(fld->data("SW Access").toString());
            if (!mwv.isEmpty())
              xml.writeTextElement("modifiedWriteValues", mwv);
            QString ra = uvmToSvdReadAction(fld->data("SW Access").toString());
            if (!ra.isEmpty())
              xml.writeTextElement("readAction", ra);
          }
          xml.writeEndElement(); // field
        }
        xml.writeEndElement(); // fields
      }

      xml.writeEndElement(); // register
    }
    xml.writeEndElement(); // registers
    xml.writeEndElement(); // peripheral
  }

  xml.writeEndElement(); // peripherals
  xml.writeEndElement(); // device
  xml.writeEndDocument();

  file.close();
  result.success = true;
  return result;
}
