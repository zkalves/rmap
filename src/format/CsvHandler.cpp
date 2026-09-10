/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "CsvHandler.hpp"
#include <QFile>
#include <QTextStream>
#include <QMap>
#include "../RegMapTreeModel.hpp"
#include "../RegMapTreeItem.hpp"
#include "../RegConfigWindow.hpp"

namespace {

QString escapeCsv(const QString &field, char16_t delimiter) {
    if (field.contains(QChar(delimiter)) || field.contains('"') || field.contains('\n') || field.contains('\r')) {
        QString escaped = field;
        escaped.replace("\"", "\"\"");
        return QString("\"%1\"").arg(escaped);
    }
    return field;
}

std::vector<QStringList> parseCsvLines(const QString &content, char16_t delimiter) {
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
                if (!currentRecord.isEmpty() && (currentRecord.size() > 1 || !currentRecord[0].isEmpty())) {
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
        if (!currentRecord.isEmpty() && (currentRecord.size() > 1 || !currentRecord[0].isEmpty())) {
            records.push_back(currentRecord);
        }
    }

    return records;
}

} // anonymous namespace

FormatResult CsvHandler::read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config)
{
    FormatResult result;
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        result.success = false;
        result.errorMessage = QString("Cannot open CSV file: %1").arg(file.errorString());
        return result;
    }

    QString content = QTextStream(&file).readAll();
    file.close();

    char16_t delimiter = filepath.endsWith(".tsv", Qt::CaseInsensitive) ? u'\t' : u',';
    auto rows = parseCsvLines(content, delimiter);
    if (rows.empty()) {
        result.success = false;
        result.errorMessage = "CSV file is empty.";
        return result;
    }

    // Column definitions
    QVector<QString> cols = {"Type", "Offset/LSB", "Size/Width", "Name", "Access Policy", "HW Access", "Reset Value", "Is Rand", "Volatile", "Has Reset", "Description"};
    QVariantMap rootData;
    for (const QString &c : cols) rootData[c] = c;
    RegMapTreeItem *rootItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::root, rootData);

    QHash<QString, RegMapTreeItem*> blockMap;
    QHash<QString, RegMapTreeItem*> regMap;

    // Headers check (skip if first row contains 'Type' or 'Block')
    size_t startRow = 0;
    if (!rows.empty() && (rows[0][0].toLower() == "type" || rows[0][0].toLower() == "block")) {
        startRow = 1;
    }

    for (size_t r = startRow; r < rows.size(); ++r) {
        const QStringList &row = rows[r];
        if (row.size() < 4) continue;

        QString type = row[0].toLower().trimmed();
        QString blkName = row.size() > 1 ? row[1].trimmed() : "TOP_BLK";
        QString regName = row.size() > 2 ? row[2].trimmed() : "";
        QString fldName = row.size() > 3 ? row[3].trimmed() : "";
        QString offsetLsb = row.size() > 4 ? row[4].trimmed() : "0";
        QString width = row.size() > 5 ? row[5].trimmed() : "1";
        QString access = row.size() > 6 ? row[6].trimmed() : "RW";
        QString reset = row.size() > 7 ? row[7].trimmed() : "0x0";
        QString isRand = row.size() > 8 ? row[8].trimmed() : "true";
        QString isVol = row.size() > 9 ? row[9].trimmed() : "false";
        QString hasReset = row.size() > 10 ? row[10].trimmed() : "true";
        QString desc = row.size() > 11 ? row[11].trimmed() : "";

        if (blkName.isEmpty()) blkName = "TOP_BLK";

        // Ensure Block exists
        RegMapTreeItem *blkItem = nullptr;
        if (!blockMap.contains(blkName)) {
            QVariantMap blkData;
            blkData["Type"] = "blk";
            blkData["Offset/LSB"] = "0x0";
            blkData["Name"] = blkName;
            blkData["Description"] = "";
            blkItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, blkData, rootItem);
            rootItem->appendChild(blkItem);
            blockMap[blkName] = blkItem;
        } else {
            blkItem = blockMap[blkName];
        }

        if (type == "blk") continue;

        // Ensure Register exists
        QString regKey = blkName + "::" + regName;
        RegMapTreeItem *regItem = nullptr;
        if (!regName.isEmpty()) {
            if (!regMap.contains(regKey)) {
                QVariantMap regData;
                regData["Type"] = "reg";
                regData["Offset/LSB"] = (type == "reg") ? offsetLsb : "0x0";
                regData["Size/Width"] = "32";
                regData["Name"] = regName;
                regData["Access Policy"] = access;
                regData["HW Access"] = "RO";
                regData["Reset Value"] = "0x0";
                regData["Description"] = (type == "reg") ? desc : "";
                regItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, regData, blkItem);
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
            fldData["Access Policy"] = access;
            fldData["HW Access"] = (access == "RO") ? "WO" : "RO";
            fldData["Reset Value"] = reset;
            fldData["Is Rand"] = isRand;
            fldData["Volatile"] = isVol;
            fldData["Has Reset"] = hasReset;
            fldData["Description"] = desc;
            RegMapTreeItem *fldItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, fldData, regItem);
            regItem->appendChild(fldItem);
        }
    }

    if (model) {
        model->setRootItem(rootItem);
    }

    result.success = true;
    return result;
}

FormatResult CsvHandler::write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config)
{
    FormatResult result;
    if (!model || !model->getRootItem()) {
        result.success = false;
        result.errorMessage = "No register map model available to export.";
        return result;
    }

    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        result.success = false;
        result.errorMessage = QString("Cannot write to CSV file: %1").arg(file.errorString());
        return result;
    }

    char16_t delimiter = filepath.endsWith(".tsv", Qt::CaseInsensitive) ? u'\t' : u',';
    QTextStream out(&file);

    // Header row
    QStringList headers = {"Type", "Block", "Register", "Field", "Offset/LSB", "Width", "Access", "Reset", "IsRand", "Volatile", "HasReset", "Description"};
    for (int i = 0; i < headers.size(); ++i) {
        out << escapeCsv(headers[i], delimiter) << (i + 1 < headers.size() ? QChar(delimiter) : QChar('\n'));
    }

    RegMapTreeItem *root = model->getRootItem();
    for (RegMapTreeItem *blk : root->getChildItems()) {
        if (!blk || blk->kind() != RegMapTreeItem::e_rmmKind::blk) continue;

        QString blkName = blk->data("Name").toString().trimmed();

        for (RegMapTreeItem *reg : blk->getChildItems()) {
            if (!reg || reg->kind() != RegMapTreeItem::e_rmmKind::reg) continue;

            QString regName = reg->data("Name").toString().trimmed();
            QString regOffset = reg->data("Offset/LSB").toString().trimmed();
            QString regDesc = reg->data("Description").toString().trimmed();

            QStringList regRow = {"reg", blkName, regName, "", regOffset, "32", "RW", "0x0", "false", "false", "false", regDesc};
            for (int i = 0; i < regRow.size(); ++i) {
                out << escapeCsv(regRow[i], delimiter) << (i + 1 < regRow.size() ? QChar(delimiter) : QChar('\n'));
            }

            auto fields = reg->getChildItems();
            for (RegMapTreeItem *fld : fields) {
                if (!fld || fld->kind() != RegMapTreeItem::e_rmmKind::fld) continue;

                QString fldName = fld->data("Name").toString().trimmed();
                QString lsb = fld->data("Offset/LSB").toString().trimmed();
                QString width = fld->data("Size/Width").toString().trimmed();
                QString access = fld->data("Access Policy").toString().trimmed();
                QString reset = fld->data("Reset Value").toString().trimmed();
                QString isRand = fld->data("Is Rand").toString().trimmed();
                QString isVol = fld->data("Volatile").toString().trimmed();
                QString hasReset = fld->data("Has Reset").toString().trimmed();
                QString fldDesc = fld->data("Description").toString().trimmed();

                QStringList row = {"fld", blkName, regName, fldName, lsb, width, access, reset, isRand, isVol, hasReset, fldDesc};
                for (int i = 0; i < row.size(); ++i) {
                    out << escapeCsv(row[i], delimiter) << (i + 1 < row.size() ? QChar(delimiter) : QChar('\n'));
                }
            }
        }
    }

    file.close();
    result.success = true;
    return result;
}
