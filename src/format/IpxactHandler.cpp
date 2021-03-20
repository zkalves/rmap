#include "IpxactHandler.hpp"
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include "../RegMapTreeModel.hpp"
#include "../RegMapTreeItem.hpp"
#include "../RegConfigWindow.hpp"

namespace {

QString ipxactAccessToUvm(const QString &acc) {
    QString a = acc.toLower().trimmed();
    if (a == "read-write" || a == "rw") return "RW";
    if (a == "read-only" || a == "ro" || a == "r") return "RO";
    if (a == "write-only" || a == "wo" || a == "w") return "WO";
    if (a == "read-writeonce" || a == "w1c") return "W1C";
    if (a == "w0c") return "W0C";
    if (a == "rc") return "RC";
    if (a == "rs") return "RS";
    if (a == "w1s") return "W1S";
    if (a == "w0s") return "W0S";
    return "RW";
}

QString uvmAccessToIpxact(const QString &acc) {
    QString a = acc.toUpper().trimmed();
    if (a == "RO") return "read-only";
    if (a == "WO") return "write-only";
    return "read-write";
}

uint64_t parseXmlNum(const QString &str) {
    QString s = str.trimmed().toLower();
    if (s.startsWith("0x")) return s.mid(2).toULongLong(nullptr, 16);
    if (s.startsWith("'h")) return s.mid(2).toULongLong(nullptr, 16);
    if (s.startsWith("0b")) return s.mid(2).toULongLong(nullptr, 2);
    return s.toULongLong(nullptr, 10);
}

} // anonymous namespace

FormatResult IpxactHandler::read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config)
{
    FormatResult result;
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.success = false;
        result.errorMessage = QString("Cannot open IP-XACT file: %1").arg(file.errorString());
        return result;
    }

    QXmlStreamReader xml(&file);

    QVector<QString> cols = {"Type", "Offset/LSB", "Size/Width", "Name", "Access Policy", "HW Access", "Reset Value", "Is Rand", "Volatile", "Has Reset", "Description"};
    QVariantMap rootData;
    for (const QString &c : cols) rootData[c] = c;
    RegMapTreeItem *rootItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::root, rootData);

    RegMapTreeItem *currentBlock = nullptr;
    RegMapTreeItem *currentReg = nullptr;

    QString projectName = "IPXACT_Component";
    uint32_t globalWidth = 32;

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartElement) {
            QString name = xml.name().toString();

            if (name == "component") {
                // Outer component
            } else if (name == "name" && !currentBlock && !currentReg) {
                projectName = xml.readElementText();
            } else if (name == "addressBlock") {
                // Block container
                QVariantMap blkData;
                blkData["Type"] = "blk";
                blkData["Offset/LSB"] = "0x0";
                blkData["Name"] = "ADDR_BLOCK";
                blkData["Description"] = "";
                currentBlock = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, blkData, rootItem);
                rootItem->appendChild(currentBlock);
            } else if (name == "name" && currentBlock && !currentReg) {
                currentBlock->setData("Name", xml.readElementText());
            } else if (name == "baseAddress" && currentBlock && !currentReg) {
                QString addr = xml.readElementText();
                currentBlock->setData("Offset/LSB", QString("0x%1").arg(parseXmlNum(addr), 0, 16));
            } else if (name == "register") {
                // Register container
                QVariantMap regData;
                regData["Type"] = "reg";
                regData["Offset/LSB"] = "0x0";
                regData["Size/Width"] = QString::number(globalWidth);
                regData["Name"] = "REG";
                regData["Access Policy"] = "RW";
                regData["HW Access"] = "RO";
                regData["Reset Value"] = "0x0";
                regData["Description"] = "";
                currentReg = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, regData, currentBlock ? currentBlock : rootItem);
                if (currentBlock) currentBlock->appendChild(currentReg);
                else rootItem->appendChild(currentReg);
            } else if (name == "name" && currentReg) {
                currentReg->setData("Name", xml.readElementText());
            } else if (name == "addressOffset" && currentReg) {
                QString offset = xml.readElementText();
                currentReg->setData("Offset/LSB", QString("0x%1").arg(parseXmlNum(offset), 0, 16));
            } else if (name == "size" && currentReg) {
                QString sizeStr = xml.readElementText();
                uint32_t sz = parseXmlNum(sizeStr);
                if (sz > 0) {
                    globalWidth = sz;
                    currentReg->setData("Size/Width", QString::number(sz));
                }
            } else if (name == "description" && currentReg) {
                currentReg->setData("Description", xml.readElementText());
            } else if (name == "field") {
                // Field container inside register
                QString fldName = "FIELD";
                QString fldDesc;
                uint32_t bitOffset = 0;
                uint32_t bitWidth = 1;
                QString access = "RW";
                uint64_t resetVal = 0;
                bool hasReset = false;

                while (!xml.atEnd()) {
                    xml.readNext();
                    if (xml.isEndElement() && xml.name().toString() == "field") {
                        break;
                    }
                    if (xml.isStartElement()) {
                        QString sub = xml.name().toString();
                        if (sub == "name") fldName = xml.readElementText();
                        else if (sub == "description") fldDesc = xml.readElementText();
                        else if (sub == "bitOffset") bitOffset = parseXmlNum(xml.readElementText());
                        else if (sub == "bitWidth") bitWidth = parseXmlNum(xml.readElementText());
                        else if (sub == "access") access = ipxactAccessToUvm(xml.readElementText());
                        else if (sub == "value") {
                            resetVal = parseXmlNum(xml.readElementText());
                            hasReset = true;
                        }
                    }
                }

                if (currentReg) {
                    QVariantMap fldData;
                    fldData["Type"] = "fld";
                    fldData["Offset/LSB"] = QString::number(bitOffset);
                    fldData["Size/Width"] = QString::number(bitWidth);
                    fldData["Name"] = fldName;
                    fldData["Access Policy"] = access;
                    fldData["HW Access"] = (access == "RO") ? "WO" : "RO";
                    fldData["Reset Value"] = QString("0x%1").arg(resetVal, 0, 16);
                    fldData["Is Rand"] = "true";
                    fldData["Volatile"] = "false";
                    fldData["Has Reset"] = hasReset ? "true" : "false";
                    fldData["Description"] = fldDesc;
                    RegMapTreeItem *fldItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, fldData, currentReg);
                    currentReg->appendChild(fldItem);
                }
            }
        } else if (token == QXmlStreamReader::EndElement) {
            QString name = xml.name().toString();
            if (name == "register") {
                currentReg = nullptr;
            } else if (name == "addressBlock") {
                currentBlock = nullptr;
            }
        }
    }

    if (xml.hasError()) {
        delete rootItem;
        result.success = false;
        result.errorMessage = QString("XML Parse error: %1 (line %2)").arg(xml.errorString()).arg(xml.lineNumber());
        return result;
    }

    file.close();

    // If no blocks were created, ensure at least one default block wrapper exists
    if (rootItem->childCount() == 0) {
        QVariantMap blkData;
        blkData["Type"] = "blk";
        blkData["Offset/LSB"] = "0x0";
        blkData["Name"] = projectName;
        rootItem->appendChild(new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, blkData, rootItem));
    }

    if (config) {
        config->setProjectName(projectName);
        config->setRegisterWidth(globalWidth);
    }

    if (model) {
        model->setRootItem(rootItem);
    }

    result.success = true;
    return result;
}

FormatResult IpxactHandler::write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config)
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
        result.errorMessage = QString("Cannot write to IP-XACT file: %1").arg(file.errorString());
        return result;
    }

    uint32_t regWidth = 32;
    QString projName = "chip_map";
    QString projVer = "1.0";
    if (config) {
        protormap::Config *cfg = config->serialize();
        if (cfg) {
            if (cfg->reg_width() > 0) regWidth = cfg->reg_width();
            if (!cfg->project_name().empty()) projName = QString::fromStdString(cfg->project_name());
            if (!cfg->project_version().empty()) projVer = QString::fromStdString(cfg->project_version());
            delete cfg;
        }
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();

    xml.writeStartElement("ipxact:component");
    xml.writeAttribute("xmlns:ipxact", "https://www.accellera.org/XMLSchema/IPXACT/1685-2014");
    xml.writeAttribute("xmlns:xsi", "https://www.w3.org/2001/XMLSchema-instance");

    xml.writeTextElement("ipxact:vendor", "rmap");
    xml.writeTextElement("ipxact:library", "components");
    xml.writeTextElement("ipxact:name", projName);
    xml.writeTextElement("ipxact:version", projVer);

    xml.writeStartElement("ipxact:memoryMaps");
    xml.writeStartElement("ipxact:memoryMap");
    xml.writeTextElement("ipxact:name", "default_memory_map");

    RegMapTreeItem *root = model->getRootItem();
    for (RegMapTreeItem *blk : root->getChildItems()) {
        if (!blk || blk->kind() != RegMapTreeItem::e_rmmKind::blk) continue;

        xml.writeStartElement("ipxact:addressBlock");
        xml.writeTextElement("ipxact:name", blk->data("Name").toString().trimmed());
        xml.writeTextElement("ipxact:baseAddress", blk->data("Offset/LSB").toString().trimmed());
        xml.writeTextElement("ipxact:range", "0x1000");
        xml.writeTextElement("ipxact:width", QString::number(regWidth));

        for (RegMapTreeItem *reg : blk->getChildItems()) {
            if (!reg || reg->kind() != RegMapTreeItem::e_rmmKind::reg) continue;

            xml.writeStartElement("ipxact:register");
            xml.writeTextElement("ipxact:name", reg->data("Name").toString().trimmed());
            if (!reg->data("Description").toString().trimmed().isEmpty()) {
                xml.writeTextElement("ipxact:description", reg->data("Description").toString().trimmed());
            }
            xml.writeTextElement("ipxact:addressOffset", reg->data("Offset/LSB").toString().trimmed());
            xml.writeTextElement("ipxact:size", QString::number(regWidth));

            for (RegMapTreeItem *fld : reg->getChildItems()) {
                if (!fld || fld->kind() != RegMapTreeItem::e_rmmKind::fld) continue;

                xml.writeStartElement("ipxact:field");
                xml.writeTextElement("ipxact:name", fld->data("Name").toString().trimmed());
                if (!fld->data("Description").toString().trimmed().isEmpty()) {
                    xml.writeTextElement("ipxact:description", fld->data("Description").toString().trimmed());
                }
                xml.writeTextElement("ipxact:bitOffset", fld->data("Offset/LSB").toString().trimmed());
                xml.writeTextElement("ipxact:bitWidth", fld->data("Size/Width").toString().trimmed());
                xml.writeTextElement("ipxact:access", uvmAccessToIpxact(fld->data("Access Policy").toString().trimmed()));

                if (fld->data("Has Reset").toString().toLower() == "true") {
                    xml.writeStartElement("ipxact:resets");
                    xml.writeStartElement("ipxact:reset");
                    xml.writeTextElement("ipxact:value", fld->data("Reset Value").toString().trimmed());
                    xml.writeEndElement(); // ipxact:reset
                    xml.writeEndElement(); // ipxact:resets
                }

                xml.writeEndElement(); // ipxact:field
            }

            xml.writeEndElement(); // ipxact:register
        }

        xml.writeEndElement(); // ipxact:addressBlock
    }

    xml.writeEndElement(); // ipxact:memoryMap
    xml.writeEndElement(); // ipxact:memoryMaps
    xml.writeEndElement(); // ipxact:component

    xml.writeEndDocument();
    file.close();

    result.success = true;
    return result;
}
