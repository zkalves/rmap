/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "SystemRdlHandler.hpp"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include "../RegMapTreeModel.hpp"
#include "../RegMapTreeItem.hpp"
#include "../RegConfigWindow.hpp"

namespace {

enum class TokenType {
    Ident,
    String,
    Number,
    LBrace,     // {
    RBrace,     // }
    LBracket,   // [
    RBracket,   // ]
    Colon,      // :
    Semicolon,  // ;
    Equals,     // =
    At,         // @
    Arrow,      // ->
    Dot,        // .
    Eof
};

struct Token {
    TokenType type = TokenType::Eof;
    QString text;
    int line = 1;
};

class RdlLexer {
public:
    explicit RdlLexer(const QString &source) : m_src(source), m_pos(0), m_len(source.length()), m_line(1) {}

    Token nextToken() {
        skipWhitespaceAndComments();
        if (m_pos >= m_len) {
            return {TokenType::Eof, "", m_line};
        }

        const auto ch = m_src[m_pos];
        int startLine = m_line;

        if (ch == '{') { m_pos++; return {TokenType::LBrace, "{", startLine}; }
        if (ch == '}') { m_pos++; return {TokenType::RBrace, "}", startLine}; }
        if (ch == '[') { m_pos++; return {TokenType::LBracket, "[", startLine}; }
        if (ch == ']') { m_pos++; return {TokenType::RBracket, "]", startLine}; }
        if (ch == ':') { m_pos++; return {TokenType::Colon, ":", startLine}; }
        if (ch == ';') { m_pos++; return {TokenType::Semicolon, ";", startLine}; }
        if (ch == '=') { m_pos++; return {TokenType::Equals, "=", startLine}; }
        if (ch == '@') { m_pos++; return {TokenType::At, "@", startLine}; }
        if (ch == '.') { m_pos++; return {TokenType::Dot, ".", startLine}; }

        if (ch == '-' && m_pos + 1 < m_len && m_src[m_pos + 1] == '>') {
            m_pos += 2;
            return {TokenType::Arrow, "->", startLine};
        }

        // String literal "..."
        if (ch == '"') {
            m_pos++; // skip leading quote
            QString str;
            while (m_pos < m_len && m_src[m_pos] != '"') {
                if (m_src[m_pos] == '\\' && m_pos + 1 < m_len) {
                    m_pos++;
                    const auto esc = m_src[m_pos];
                    if (esc == 'n') str += '\n';
                    else if (esc == 't') str += '\t';
                    else if (esc == '"') str += '"';
                    else if (esc == '\\') str += '\\';
                    else str += esc;
                } else {
                    if (m_src[m_pos] == '\n') m_line++;
                    str += m_src[m_pos];
                }
                m_pos++;
            }
            if (m_pos < m_len && m_src[m_pos] == '"') {
                m_pos++; // skip trailing quote
            }
            return {TokenType::String, str, startLine};
        }

        // Number: 0x..., 8'h..., 123
        if (ch.isDigit() || (ch == '\'' && m_pos > 0)) {
            QString num;
            while (m_pos < m_len) {
                const auto c = m_src[m_pos];
                if (c.isLetterOrNumber() || c == '\'' || c == '_') {
                    num += c;
                    m_pos++;
                } else {
                    break;
                }
            }
            return {TokenType::Number, num, startLine};
        }

        // Identifier or keyword
        if (ch.isLetter() || ch == '_') {
            QString ident;
            while (m_pos < m_len && (m_src[m_pos].isLetterOrNumber() || m_src[m_pos] == '_')) {
                ident += m_src[m_pos++];
            }
            return {TokenType::Ident, ident, startLine};
        }

        // Single character fallback
        m_pos++;
        return {TokenType::Ident, QString(ch), startLine};
    }

private:
    void skipWhitespaceAndComments() {
        while (m_pos < m_len) {
            const auto ch = m_src[m_pos];
            if (ch == '\n') {
                m_line++;
                m_pos++;
            } else if (ch.isSpace()) {
                m_pos++;
            } else if (ch == '/' && m_pos + 1 < m_len && m_src[m_pos + 1] == '/') {
                // Line comment
                m_pos += 2;
                while (m_pos < m_len && m_src[m_pos] != '\n') {
                    m_pos++;
                }
            } else if (ch == '/' && m_pos + 1 < m_len && m_src[m_pos + 1] == '*') {
                // Block comment
                m_pos += 2;
                while (m_pos + 1 < m_len && !(m_src[m_pos] == '*' && m_src[m_pos + 1] == '/')) {
                    if (m_src[m_pos] == '\n') m_line++;
                    m_pos++;
                }
                if (m_pos + 1 < m_len) m_pos += 2;
            } else if (ch == '<' && m_pos + 1 < m_len && m_src[m_pos + 1] == '%') {
                // Embedded ASP/Perl comment <% ... %>
                m_pos += 2;
                while (m_pos + 1 < m_len && !(m_src[m_pos] == '%' && m_src[m_pos + 1] == '>')) {
                    if (m_src[m_pos] == '\n') m_line++;
                    m_pos++;
                }
                if (m_pos + 1 < m_len) m_pos += 2;
            } else {
                break;
            }
        }
    }

    QString m_src;
    int m_pos = 0;
    int m_len = 0;
    int m_line = 1;
};

uint64_t parseRdlNumber(const QString &str) {
    QString s = str.trimmed().toLower();
    if (s.contains('\'')) {
        int idx = s.indexOf('\'');
        QString base = s.mid(idx + 1);
        if (base.startsWith('h')) return base.mid(1).toULongLong(nullptr, 16);
        if (base.startsWith('b')) return base.mid(1).toULongLong(nullptr, 2);
        if (base.startsWith('d')) return base.mid(1).toULongLong(nullptr, 10);
    }
    if (s.startsWith("0x")) return s.mid(2).toULongLong(nullptr, 16);
    if (s.startsWith("0b")) return s.mid(2).toULongLong(nullptr, 2);
    return s.toULongLong(nullptr, 10);
}

QString formatRdlNumberHex(uint64_t val) {
    return QString("0x%1").arg(val, 0, 16);
}

QString rdlSwToAccess(const QString &sw) {
    QString s = sw.toLower().trimmed();
    if (s == "rw") return "RW";
    if (s == "r" || s == "ro") return "RO";
    if (s == "w" || s == "wo") return "WO";
    if (s == "w1c") return "W1C";
    if (s == "w0c") return "W0C";
    if (s == "rc") return "RC";
    if (s == "rs") return "RS";
    if (s == "w1s") return "W1S";
    if (s == "w0s") return "W0S";
    return "RW";
}

QString accessToRdlSw(const QString &access) {
    QString a = access.toUpper().trimmed();
    if (a == "RO") return "r";
    if (a == "WO") return "w";
    return a.toLower();
}

} // anonymous namespace

FormatResult SystemRdlHandler::read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config)
{
    FormatResult result;
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.success = false;
        result.errorMessage = QString("Cannot open SystemRDL file: %1").arg(file.errorString());
        return result;
    }

    QString source = QTextStream(&file).readAll();
    file.close();

    RdlLexer lexer(source);
    std::vector<Token> tokens;
    while (true) {
        Token tok = lexer.nextToken();
        tokens.push_back(tok);
        if (tok.type == TokenType::Eof) break;
    }

    // Root container
    QVector<QString> cols = {"Type", "Offset/LSB", "Size/Width", "Name", "Access Policy", "HW Access", "Reset Value", "Is Rand", "Volatile", "Has Reset", "Description"};
    QVariantMap rootData;
    for (const QString &c : cols) rootData[c] = c;
    RegMapTreeItem *rootItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::root, rootData);

    uint32_t defaultRegWidth = 32;
    QString defaultSw = "RW";
    QString projectName = "RDL_Project";

    // Top block item
    QVariantMap topBlockData;
    topBlockData["Type"] = "blk";
    topBlockData["Offset/LSB"] = "0x0";
    topBlockData["Name"] = "TOP_BLK";
    topBlockData["Description"] = "";
    RegMapTreeItem *currentBlock = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, topBlockData, rootItem);
    rootItem->appendChild(currentBlock);

    size_t idx = 0;
    auto peek = [&](size_t offset = 0) -> Token {
        return tokens[std::min(idx + offset, tokens.size() - 1)];
    };
    auto consume = [&]() -> Token {
        return tokens[std::min(idx++, tokens.size() - 1)];
    };

    uint64_t nextRegOffset = 0;

    while (idx < tokens.size() && peek().type != TokenType::Eof) {
        Token tok = peek();

        if (tok.type == TokenType::Ident && tok.text == "addrmap") {
            consume(); // addrmap
            if (peek().type == TokenType::Ident) {
                projectName = consume().text;
                currentBlock->setData("Name", projectName);
            }
            if (peek().type == TokenType::LBrace) consume();
            continue;
        }

        if (tok.type == TokenType::Ident && tok.text == "regfile") {
            consume(); // regfile
            QString blkName = "BLOCK";
            if (peek().type == TokenType::Ident) {
                blkName = consume().text;
            }
            if (peek().type == TokenType::LBrace) consume();

            QVariantMap blkData;
            blkData["Type"] = "blk";
            blkData["Offset/LSB"] = formatRdlNumberHex(nextRegOffset);
            blkData["Name"] = blkName;
            blkData["Description"] = "";
            RegMapTreeItem *newBlk = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::blk, blkData, rootItem);
            rootItem->appendChild(newBlk);
            currentBlock = newBlk;
            nextRegOffset = 0;
            continue;
        }

        if (tok.type == TokenType::Ident && tok.text == "default") {
            consume(); // default
            if (peek().type == TokenType::Ident && peek().text == "regwidth") {
                consume();
                if (peek().type == TokenType::Equals) consume();
                if (peek().type == TokenType::Number) {
                    defaultRegWidth = parseRdlNumber(consume().text);
                }
            } else if (peek().type == TokenType::Ident && peek().text == "sw") {
                consume();
                if (peek().type == TokenType::Equals) consume();
                if (peek().type == TokenType::Ident) {
                    defaultSw = rdlSwToAccess(consume().text);
                }
            }
            if (peek().type == TokenType::Semicolon) consume();
            continue;
        }

        // Register declaration: reg [typeName] { ... } [instName] [ @ offset ];
        if (tok.type == TokenType::Ident && tok.text == "reg") {
            consume(); // reg
            QString regTypeName;
            if (peek().type == TokenType::Ident && peek(1).type == TokenType::LBrace) {
                regTypeName = consume().text;
            }

            QString regName = !regTypeName.isEmpty() ? regTypeName : "REG";
            QString regDesc;
            uint64_t regOffset = nextRegOffset;
            bool hasExplicitOffset = false;

            struct ParsedField {
                QString name;
                uint32_t lsb = 0;
                uint32_t width = 1;
                QString access = QStringLiteral("RW");
                QString hwAccess = QStringLiteral("RO");
                uint64_t resetVal = 0;
                bool hasReset = false;
                QString desc;
            };
            std::vector<ParsedField> fields;

            if (peek().type == TokenType::LBrace) {
                consume(); // {
                uint32_t currentBit = 0;

                while (idx < tokens.size() && peek().type != TokenType::RBrace && peek().type != TokenType::Eof) {
                    Token inner = peek();
                    if (inner.type == TokenType::Ident && inner.text == "name") {
                        consume();
                        if (peek().type == TokenType::Equals) consume();
                        if (peek().type == TokenType::String) regName = consume().text;
                        if (peek().type == TokenType::Semicolon) consume();
                    } else if (inner.type == TokenType::Ident && inner.text == "desc") {
                        consume();
                        if (peek().type == TokenType::Equals) consume();
                        if (peek().type == TokenType::String) regDesc = consume().text;
                        if (peek().type == TokenType::Semicolon) consume();
                    } else if (inner.type == TokenType::Ident && inner.text == "field") {
                        consume(); // field
                        QString fldTypeName;
                        if (peek().type == TokenType::Ident && peek(1).type == TokenType::LBrace) {
                            fldTypeName = consume().text;
                        }

                        ParsedField fld;
                        fld.name = !fldTypeName.isEmpty() ? fldTypeName : "FIELD";
                        fld.access = defaultSw;
                        fld.hwAccess = "RO";

                        if (peek().type == TokenType::LBrace) {
                            consume(); // {
                            while (idx < tokens.size() && peek().type != TokenType::RBrace && peek().type != TokenType::Eof) {
                                Token fprop = peek();
                                if (fprop.type == TokenType::Ident && fprop.text == "desc") {
                                    consume();
                                    if (peek().type == TokenType::Equals) consume();
                                    if (peek().type == TokenType::String) fld.desc = consume().text;
                                    if (peek().type == TokenType::Semicolon) consume();
                                } else if (fprop.type == TokenType::Ident && fprop.text == "sw") {
                                    consume();
                                    if (peek().type == TokenType::Equals) consume();
                                    if (peek().type == TokenType::Ident) fld.access = rdlSwToAccess(consume().text);
                                    if (peek().type == TokenType::Semicolon) consume();
                                } else if (fprop.type == TokenType::Ident && fprop.text == "hw") {
                                    consume();
                                    if (peek().type == TokenType::Equals) consume();
                                    if (peek().type == TokenType::Ident) {
                                        QString h = consume().text.toLower();
                                        if (h == "r" || h == "ro") fld.hwAccess = "RO";
                                        else if (h == "w" || h == "wo") fld.hwAccess = "WO";
                                        else if (h == "rw") fld.hwAccess = "RW";
                                        else if (h == "na") fld.hwAccess = "NA";
                                        else fld.hwAccess = h.toUpper();
                                    }
                                    if (peek().type == TokenType::Semicolon) consume();
                                } else if (fprop.type == TokenType::Ident && (fprop.text == "rclr" || fprop.text == "rc")) {
                                    consume();
                                    fld.access = "RC";
                                    if (peek().type == TokenType::Semicolon) consume();
                                } else if (fprop.type == TokenType::Ident && (fprop.text == "w1clr" || fprop.text == "w1c")) {
                                    consume();
                                    fld.access = "W1C";
                                    if (peek().type == TokenType::Semicolon) consume();
                                } else if (fprop.type == TokenType::Ident && (fprop.text == "w1set" || fprop.text == "w1s")) {
                                    consume();
                                    fld.access = "W1S";
                                    if (peek().type == TokenType::Semicolon) consume();
                                } else {
                                    consume();
                                }
                            }
                            if (peek().type == TokenType::RBrace) consume(); // }
                        }

                        // Field instance name: e.g. PRESCALER[1:0] = 0;
                        if (peek().type == TokenType::Ident) {
                            fld.name = consume().text;
                        }

                        // Bit range [msb:lsb] or [width]
                        if (peek().type == TokenType::LBracket) {
                            consume(); // [
                            if (peek().type == TokenType::Number) {
                                uint32_t val1 = static_cast<uint32_t>(parseRdlNumber(consume().text));
                                if (peek().type == TokenType::Colon) {
                                    consume(); // :
                                    if (peek().type == TokenType::Number) {
                                        uint32_t val2 = static_cast<uint32_t>(parseRdlNumber(consume().text));
                                        fld.lsb = (std::min)(val1, val2);
                                        fld.width = ((std::max)(val1, val2) - fld.lsb) + 1;
                                    }
                                } else {
                                    fld.lsb = currentBit;
                                    fld.width = val1;
                                }
                            }
                            if (peek().type == TokenType::RBracket) consume(); // ]
                        } else {
                            fld.lsb = currentBit;
                            fld.width = 1;
                        }
                        currentBit = fld.lsb + fld.width;

                        // Reset value = 0;
                        if (peek().type == TokenType::Equals) {
                            consume(); // =
                            if (peek().type == TokenType::Number) {
                                fld.resetVal = parseRdlNumber(consume().text);
                                fld.hasReset = true;
                            }
                        }

                        if (peek().type == TokenType::Semicolon) consume();
                        fields.push_back(fld);
                    } else if (inner.type == TokenType::Ident && (peek(1).type == TokenType::Arrow || peek(1).type == TokenType::Dot)) {
                        // Reset assignment: full->reset = 1'b0;
                        QString targetFld = consume().text;
                        consume(); // -> or .
                        QString prop = consume().text;
                        if (prop == "reset" && peek().type == TokenType::Equals) {
                            consume();
                            if (peek().type == TokenType::Number) {
                                uint64_t rst = parseRdlNumber(consume().text);
                                for (size_t fi = 0; fi < fields.size(); ++fi) {
                                    if (fields[fi].name == targetFld) {
                                        fields[fi].resetVal = rst;
                                        fields[fi].hasReset = true;
                                    }
                                }
                            }
                        }
                        if (peek().type == TokenType::Semicolon) consume();
                    } else {
                        consume();
                    }
                }
                if (peek().type == TokenType::RBrace) consume(); // }
            }

            // Register instance name
            if (peek().type == TokenType::Ident) {
                regName = consume().text;
            }

            // Register offset @ 0x0
            if (peek().type == TokenType::At) {
                consume(); // @
                if (peek().type == TokenType::Number) {
                    regOffset = parseRdlNumber(consume().text);
                    hasExplicitOffset = true;
                }
            }
            if (peek().type == TokenType::Semicolon) consume();

            if (!hasExplicitOffset) {
                nextRegOffset = regOffset + (defaultRegWidth / 8);
            } else {
                nextRegOffset = regOffset + (defaultRegWidth / 8);
            }

            // Create Register node
            QVariantMap regData;
            regData["Type"] = "reg";
            regData["Offset/LSB"] = formatRdlNumberHex(regOffset);
            regData["Size/Width"] = QString::number(defaultRegWidth);
            regData["Name"] = regName;
            regData["Access Policy"] = defaultSw;
            regData["Reset Value"] = "0x0";
            regData["Description"] = regDesc;
            RegMapTreeItem *regItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::reg, regData, currentBlock);
            currentBlock->appendChild(regItem);

            // Create Field nodes
            for (const auto &f : fields) {
                QVariantMap fldData;
                fldData["Type"] = "fld";
                fldData["Offset/LSB"] = QString::number(f.lsb);
                fldData["Size/Width"] = QString::number(f.width);
                fldData["Name"] = f.name;
                fldData["Access Policy"] = f.access;
                fldData["HW Access"] = f.hwAccess;
                fldData["Reset Value"] = formatRdlNumberHex(f.resetVal);
                fldData["Is Rand"] = "true";
                fldData["Volatile"] = "false";
                fldData["Has Reset"] = f.hasReset ? "true" : "false";
                fldData["Description"] = f.desc;
                RegMapTreeItem *fldItem = new RegMapTreeItem(RegMapTreeItem::e_rmmKind::fld, fldData, regItem);
                regItem->appendChild(fldItem);
            }
            continue;
        }

        // Top-level properties or closing braces
        if (tok.type == TokenType::Ident && tok.text == "name") {
            consume();
            if (peek().type == TokenType::Equals) consume();
            if (peek().type == TokenType::String) projectName = consume().text;
            if (peek().type == TokenType::Semicolon) consume();
            continue;
        }

        consume();
    }

    if (config) {
        config->setRegisterWidth(defaultRegWidth);
        config->setProjectName(projectName);
    }

    if (model) {
        model->setRootItem(rootItem);
    }

    result.success = true;
    return result;
}

FormatResult SystemRdlHandler::write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config)
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
        result.errorMessage = QString("Cannot write to SystemRDL file: %1").arg(file.errorString());
        return result;
    }

    uint32_t regWidth = 32;
    QString projName = "chip_map";
    if (config) {
        protormap::Config *cfg = config->serialize();
        if (cfg) {
            if (cfg->reg_width() > 0) regWidth = cfg->reg_width();
            if (!cfg->project_name().empty()) projName = QString::fromStdString(cfg->project_name());
            delete cfg;
        }
    }

    QTextStream out(&file);
    out << "// Generated by rmap\n";
    out << "addrmap " << projName << " {\n";
    out << "    name = \"" << projName << "\";\n";
    out << "    default regwidth = " << regWidth << ";\n";
    out << "    default sw = rw;\n";
    out << "    default hw = r;\n\n";

    RegMapTreeItem *root = model->getRootItem();
    for (RegMapTreeItem *child : root->getChildItems()) {
        if (!child) continue;

        if (child->kind() == RegMapTreeItem::e_rmmKind::blk) {
            QString blkName = child->data("Name").toString().trimmed();
            if (blkName.isEmpty()) blkName = "block";

            out << "    // Block: " << blkName << "\n";
            for (RegMapTreeItem *reg : child->getChildItems()) {
                if (!reg || reg->kind() != RegMapTreeItem::e_rmmKind::reg) continue;

                QString regName = reg->data("Name").toString().trimmed();
                QString regOffset = reg->data("Offset/LSB").toString().trimmed();
                QString regDesc = reg->data("Description").toString().trimmed();

                out << "    reg {\n";
                if (!regDesc.isEmpty()) {
                    out << "        desc = \"" << regDesc << "\";\n";
                }

                for (RegMapTreeItem *fld : reg->getChildItems()) {
                    if (!fld || fld->kind() != RegMapTreeItem::e_rmmKind::fld) continue;

                    QString fldName = fld->data("Name").toString().trimmed();
                    uint32_t lsb = fld->data("Offset/LSB").toString().toUInt();
                    uint32_t width = fld->data("Size/Width").toString().toUInt();
                    if (width == 0) width = 1;
                    uint32_t msb = lsb + width - 1;
                    QString access = fld->data("Access Policy").toString().trimmed();
                    QString hwAccess = fld->data("HW Access").toString().trimmed().toLower();
                    if (hwAccess.isEmpty() || hwAccess == "ro") hwAccess = "r";
                    else if (hwAccess == "wo") hwAccess = "w";
                    QString resetVal = fld->data("Reset Value").toString().trimmed();
                    QString fldDesc = fld->data("Description").toString().trimmed();

                    out << "        field {\n";
                    if (!fldDesc.isEmpty()) {
                        out << "            desc = \"" << fldDesc << "\";\n";
                    }
                    out << "            sw = " << accessToRdlSw(access) << ";\n";
                    out << "            hw = " << hwAccess << ";\n";
                    out << "        } " << fldName << "[" << msb << ":" << lsb << "]";

                    if (fld->data("Has Reset").toString().toLower() == "true" || !resetVal.isEmpty()) {
                        out << " = " << resetVal;
                    }
                    out << ";\n";
                }

                out << "    } " << regName << " @ " << regOffset << ";\n\n";
            }
        }
    }

    out << "};\n";
    file.close();

    result.success = true;
    return result;
}
