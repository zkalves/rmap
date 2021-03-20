#include "CmsisSvdHandler.hpp"
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include "../RegMapTreeModel.hpp"
#include "../RegMapTreeItem.hpp"
#include "../RegConfigWindow.hpp"

namespace {

QString svdAccessToUvm(const QString &acc) {
    QString a = acc.toLower().trimmed();
    if (a == "read-write" || a == "rw") return "RW";
    if (a == "read-only" || a == "ro" || a == "r") return "RO";
    if (a == "write-only" || a == "wo" || a == "writeonce") return "WO";
    if (a == "read-writeonce" || a == "w1c") return "W1C";
    if (a == "w0c") return "W0C";
    if (a == "rc") return "RC";
    if (a == "rs") return "RS";
    if (a == "w1s") return "W1S";
    if (a == "w0s") return "W0S";
    return "RW";
}

QString uvmAccessToSvd(const QString &acc) {
    QString a = acc.toUpper().trimmed();
    if (a == "RO") return "read-only";
    if (a == "WO") return "write-only";
    if (a == "W1C" || a == "W0C") return "read-writeOnce";
    return "read-write";
}

uint64_t parseSvdNum(const QString &str) {
    QString s = str.trimmed().toLower();
    if (s.startsWith("0x")) return s.mid(2).toULongLong(nullptr, 16);
    if (s.startsWith("#")) return s.mid(1).toULongLong(nullptr, 16);
    if (s.startsWith("0b")) return s.mid(2).toULongLong(nullptr, 2);
    return s.toULongLong(nullptr, 10);
}

} // anonymous namespace

FormatResult CmsisSvdHandler::read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config)
{
    FormatResult result;
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.success = false;
        result.errorMessage = QString("Cannot open CMSIS-SVD file: %1").arg(file.errorString());
        return result;
    }

    QXmlStreamReader xml(&file);

    QVector<QString> cols = {"Type", "Offset/LSB", "Size/Width", "Name", "Access Policy", "HW Access", "Reset Value", "Is Rand", "Volatile", "Has Reset", "Description"};
    QVariantMap rootData;
    for (const QString &c : cols) rootData[c] = c;
    RegMapTreeItem *rootItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::root, rootData);

    RegMapTreeItem *currentBlock = nullptr;
    RegMapTreeItem *currentReg = nullptr;
    RegMapTreeItem *currentField = nullptr;

    QString deviceName = "MCU_Device";
    uint32_t globalWidth = 32;

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartElement) {
            QString name = xml.name().toString();

            if (name == "device") {
                // Device root
            } else if (name == "name" && !currentBlock && !currentReg && !currentField) {
                deviceName = xml.readElementText();
            } else if (name == "size" && !currentBlock && !currentReg && !currentField) {
                uint32_t w = xml.readElementText().toUInt();
                if (w > 0) globalWidth = w;
            } else if (name == "peripheral") {
                // Block container
                QVariantMap blkData;
                blkData["Type"] = "blk";
                blkData["Offset/LSB"] = "0x0";
                blkData["Name"] = "PERIPHERAL";
                blkData["Description"] = "";
                currentBlock = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, blkData, rootItem);
                rootItem->appendChild(currentBlock);
            } else if (name == "name" && currentBlock && !currentReg && !currentField) {
                currentBlock->setData("Name", xml.readElementText());
            } else if (name == "baseAddress" && currentBlock && !currentReg && !currentField) {
                QString addr = xml.readElementText();
                currentBlock->setData("Offset/LSB", QString("0x%1").arg(parseSvdNum(addr), 0, 16));
            } else if (name == "description" && currentBlock && !currentReg && !currentField) {
                currentBlock->setData("Description", xml.readElementText());
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
            } else if (name == "name" && currentReg && !currentField) {
                currentReg->setData("Name", xml.readElementText());
            } else if (name == "addressOffset" && currentReg && !currentField) {
                QString off = xml.readElementText();
                currentReg->setData("Offset/LSB", QString("0x%1").arg(parseSvdNum(off), 0, 16));
            } else if (name == "size" && currentReg && !currentField) {
                currentReg->setData("Size/Width", xml.readElementText());
            } else if (name == "access" && currentReg && !currentField) {
                currentReg->setData("Access Policy", svdAccessToUvm(xml.readElementText()));
            } else if (name == "resetValue" && currentReg && !currentField) {
                QString rv = xml.readElementText();
                currentReg->setData("Reset Value", QString("0x%1").arg(parseSvdNum(rv), 0, 16));
            } else if (name == "description" && currentReg && !currentField) {
                currentReg->setData("Description", xml.readElementText());
            } else if (name == "field") {
                // Field container
                QVariantMap fldData;
                fldData["Type"] = "fld";
                fldData["Offset/LSB"] = "0";
                fldData["Size/Width"] = "1";
                fldData["Name"] = "FIELD";
                fldData["Access Policy"] = currentReg ? currentReg->data("Access Policy").toString() : "RW";
                fldData["HW Access"] = "RO";
                fldData["Reset Value"] = "0x0";
                fldData["Is Rand"] = "true";
                fldData["Volatile"] = "false";
                fldData["Has Reset"] = "true";
                fldData["Description"] = "";
                currentField = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, fldData, currentReg);
                if (currentReg) currentReg->appendChild(currentField);
            } else if (name == "name" && currentField) {
                currentField->setData("Name", xml.readElementText());
            } else if (name == "bitOffset" && currentField) {
                currentField->setData("Offset/LSB", QString::number(parseSvdNum(xml.readElementText())));
            } else if (name == "bitWidth" && currentField) {
                currentField->setData("Size/Width", QString::number(parseSvdNum(xml.readElementText())));
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
                currentField->setData("Access Policy", svdAccessToUvm(xml.readElementText()));
            } else if (name == "description" && currentField) {
                currentField->setData("Description", xml.readElementText());
            }
        } else if (token == QXmlStreamReader::EndElement) {
            QString name = xml.name().toString();
            if (name == "field") currentField = nullptr;
            else if (name == "register") currentReg = nullptr;
            else if (name == "peripheral") currentBlock = nullptr;
        }
    }

    if (xml.hasError()) {
        delete rootItem;
        result.success = false;
        result.errorMessage = QString("XML Parse Error: %1 (line %2)").arg(xml.errorString()).arg(xml.lineNumber());
        return result;
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

FormatResult CmsisSvdHandler::write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config)
{
    FormatResult result;
    if (!model || !model->getRootItem()) {
        result.success = false;
        result.errorMessage = "No model data to export.";
        return result;
    }

    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        result.success = false;
        result.errorMessage = QString("Cannot open output file: %1").arg(file.errorString());
        return result;
    }

    uint32_t regWidth = 32;
    QString projectName = "CMSIS_Device";
    if (config) {
        protormap::Config *cfg = config->serialize();
        if (cfg->reg_width() > 0) regWidth = cfg->reg_width();
        if (!cfg->project_name().empty()) projectName = QString::fromStdString(cfg->project_name());
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
        if (!blk || blk->kindString() != "blk") continue;

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
            if (!reg || reg->kindString() != "reg") continue;

            xml.writeStartElement("register");
            xml.writeTextElement("name", reg->data("Name").toString());
            xml.writeTextElement("description", reg->data("Description").toString());
            xml.writeTextElement("addressOffset", reg->data("Offset/LSB").toString());
            xml.writeTextElement("size", QString::number(regWidth));
            xml.writeTextElement("access", uvmAccessToSvd(reg->data("Access Policy").toString()));
            xml.writeTextElement("resetValue", reg->data("Reset Value").toString());

            if (reg->childCount() > 0) {
                xml.writeStartElement("fields");
                for (RegMapTreeItem *fld : reg->getChildItems()) {
                    if (!fld || fld->kindString() != "fld") continue;

                    xml.writeStartElement("field");
                    xml.writeTextElement("name", fld->data("Name").toString());
                    xml.writeTextElement("description", fld->data("Description").toString());
                    xml.writeTextElement("bitOffset", fld->data("Offset/LSB").toString());
                    xml.writeTextElement("bitWidth", fld->data("Size/Width").toString());
                    xml.writeTextElement("access", uvmAccessToSvd(fld->data("Access Policy").toString()));
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
