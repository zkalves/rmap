/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "JsonHandler.hpp"
#include <QFile>
#include <QTextStream>
#include <nlohmann/json.hpp>
#include "../RegMapTreeModel.hpp"
#include "../RegMapTreeItem.hpp"
#include "../RegConfigWindow.hpp"

using json = nlohmann::json;
QString JsonHandler::formatName() const { return QStringLiteral("JSON Schema"); }
QStringList JsonHandler::supportedExtensions() const { return {QStringLiteral("json")}; }
QString JsonHandler::fileFilter() const { return QStringLiteral("JSON Register Map (*.json)"); }

FormatResult JsonHandler::read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config)
{
    FormatResult result;
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.success = false;
        result.errorMessage = QString("Cannot open JSON file: %1").arg(file.errorString());
        return result;
    }

    QString content = QTextStream(&file).readAll();
    file.close();

    json rootJson;
    try {
        rootJson = json::parse(content.toStdString());
    } catch (const std::exception &e) {
        result.success = false;
        result.errorMessage = QString("JSON parse error: %1").arg(e.what());
        return result;
    }

    uint32_t regWidth = 32;
    if (rootJson.contains("reg_width") && rootJson["reg_width"].is_number()) {
        regWidth = rootJson["reg_width"].get<uint32_t>();
    }

    QString projName = "chip_map";
    if (rootJson.contains("project_name") && rootJson["project_name"].is_string()) {
        projName = QString::fromStdString(rootJson["project_name"].get<std::string>());
    } else if (rootJson.contains("name") && rootJson["name"].is_string()) {
        projName = QString::fromStdString(rootJson["name"].get<std::string>());
    }

    QString projVersion = "1.0";
    if (rootJson.contains("project_version") && rootJson["project_version"].is_string()) {
        projVersion = QString::fromStdString(rootJson["project_version"].get<std::string>());
    }

    QVector<QString> cols = {"Type", "Offset/LSB", "Size/Width", "Name", "Access Policy", "HW Access", "Reset Value", "Is Rand", "Volatile", "Has Reset", "Description"};
    QVariantMap rootData;
    for (const QString &c : cols) rootData[c] = c;
    RegMapTreeItem *rootItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::root, rootData);

    if (rootJson.contains("blocks") && rootJson["blocks"].is_array()) {
        for (const auto &blkJson : rootJson["blocks"]) {
            QVariantMap blkData;
            blkData["Type"] = "blk";
            blkData["Offset/LSB"] = blkJson.contains("offset_hex") ? QString::fromStdString(blkJson["offset_hex"].get<std::string>()) : "0x0";
            blkData["Name"] = blkJson.contains("name") ? QString::fromStdString(blkJson["name"].get<std::string>()) : "block";
            blkData["Description"] = blkJson.contains("description") ? QString::fromStdString(blkJson["description"].get<std::string>()) : "";

            RegMapTreeItem *blkItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, blkData, rootItem);
            rootItem->appendChild(blkItem);

            if (blkJson.contains("registers") && blkJson["registers"].is_array()) {
                for (const auto &regJson : blkJson["registers"]) {
                    QVariantMap regData;
                    regData["Type"] = "reg";
                    regData["Offset/LSB"] = regJson.contains("offset_hex") ? QString::fromStdString(regJson["offset_hex"].get<std::string>()) :
                                            (regJson.contains("offset_lsb") ? QString("0x%1").arg(regJson["offset_lsb"].get<uint64_t>(), 0, 16) : "0x0");
                    regData["Size/Width"] = QString::number(regWidth);
                    regData["Name"] = regJson.contains("name") ? QString::fromStdString(regJson["name"].get<std::string>()) : "REG";
                    regData["Access Policy"] = regJson.contains("access") ? QString::fromStdString(regJson["access"].get<std::string>()) : "RW";
                    regData["HW Access"] = regJson.contains("hw_access") ? QString::fromStdString(regJson["hw_access"].get<std::string>()) : "RO";
                    regData["Reset Value"] = regJson.contains("reset_hex") ? QString::fromStdString(regJson["reset_hex"].get<std::string>()) : "0x0";
                    regData["Description"] = regJson.contains("description") ? QString::fromStdString(regJson["description"].get<std::string>()) : "";

                    RegMapTreeItem *regItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, regData, blkItem);
                    blkItem->appendChild(regItem);

                    if (regJson.contains("fields") && regJson["fields"].is_array()) {
                        for (const auto &fldJson : regJson["fields"]) {
                            QVariantMap fldData;
                            fldData["Type"] = "fld";
                            fldData["Offset/LSB"] = fldJson.contains("offset_lsb") ? QString::number(fldJson["offset_lsb"].get<uint64_t>()) : "0";
                            fldData["Size/Width"] = fldJson.contains("size_width") ? QString::number(fldJson["size_width"].get<uint64_t>()) : "1";
                            fldData["Name"] = fldJson.contains("name") ? QString::fromStdString(fldJson["name"].get<std::string>()) : "FIELD";
                            fldData["Access Policy"] = fldJson.contains("access") ? QString::fromStdString(fldJson["access"].get<std::string>()) : "RW";
                            fldData["HW Access"] = fldJson.contains("hw_access") ? QString::fromStdString(fldJson["hw_access"].get<std::string>()) : "RO";
                            fldData["Reset Value"] = fldJson.contains("reset_hex") ? QString::fromStdString(fldJson["reset_hex"].get<std::string>()) : "0x0";
                            fldData["Is Rand"] = (fldJson.contains("is_rand") && fldJson["is_rand"].get<bool>()) ? "true" : "false";
                            fldData["Volatile"] = (fldJson.contains("volatile") && fldJson["volatile"].get<bool>()) ? "true" : "false";
                            fldData["Has Reset"] = (fldJson.contains("has_reset") && fldJson["has_reset"].get<bool>()) ? "true" : "false";
                            fldData["Description"] = fldJson.contains("description") ? QString::fromStdString(fldJson["description"].get<std::string>()) : "";

                            RegMapTreeItem *fldItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, fldData, regItem);
                            regItem->appendChild(fldItem);
                        }
                    }
                }
            }
        }
    }

    if (rootJson.contains("memories") && rootJson["memories"].is_array()) {
        for (const auto &memJson : rootJson["memories"]) {
            QVariantMap memData;
            memData["Type"] = "mem";
            memData["Offset/LSB"] = memJson.contains("offset_hex") ? QString::fromStdString(memJson["offset_hex"].get<std::string>()) :
                                    (memJson.contains("offset_lsb") ? QString("0x%1").arg(memJson["offset_lsb"].get<uint64_t>(), 0, 16) : "0x0");
            memData["Size/Width"] = memJson.contains("size_width") ? QString::number(memJson["size_width"].get<uint64_t>()) : "4096";
            memData["Name"] = memJson.contains("name") ? QString::fromStdString(memJson["name"].get<std::string>()) : "MEMORY";
            memData["Access Policy"] = memJson.contains("access") ? QString::fromStdString(memJson["access"].get<std::string>()) : "RW";
            memData["HW Access"] = memJson.contains("hw_access") ? QString::fromStdString(memJson["hw_access"].get<std::string>()) : "RO";
            memData["Description"] = memJson.contains("description") ? QString::fromStdString(memJson["description"].get<std::string>()) : "";

            RegMapTreeItem *memItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::mem, memData, rootItem);
            rootItem->appendChild(memItem);
        }
    }

    if (config) {
        config->setRegisterWidth(regWidth);
        config->setProjectName(projName);
        config->setProjectVersion(projVersion);
    }

    if (model) {
        model->setRootItem(rootItem);
    }

    result.success = true;
    return result;
}

FormatResult JsonHandler::write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config)
{
    FormatResult result;
    if (!model || !model->getRootItem()) {
        result.success = false;
        result.errorMessage = "No register map model available to export.";
        return result;
    }

    uint32_t regWidth = 32;
    QString projName = "chip_map";
    QString projVersion = "1.0";
    if (config) {
        protormap::Config *cfg = config->serialize();
        if (cfg) {
            if (cfg->reg_width() > 0) regWidth = cfg->reg_width();
            if (!cfg->project_name().empty()) projName = QString::fromStdString(cfg->project_name());
            if (!cfg->project_version().empty()) projVersion = QString::fromStdString(cfg->project_version());
            delete cfg;
        }
    }

    json rootJson = model->extractJsonData(regWidth);
    rootJson["project_name"] = projName.toStdString();
    rootJson["project_version"] = projVersion.toStdString();

    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        result.success = false;
        result.errorMessage = QString("Cannot write to JSON file: %1").arg(file.errorString());
        return result;
    }

    QTextStream out(&file);
    out << QString::fromStdString(rootJson.dump(4)) << "\n";
    file.close();

    result.success = true;
    return result;
}
