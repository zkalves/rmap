/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "LockParser.hpp"
#include "RegMapTreeItem.hpp"
#include "RegMapTreeModel.hpp"
#include <QRegularExpression>
#include <QSet>
#include <QStringList>

namespace {

enum class TokenType {
  Identifier,
  Number,
  OpAnd,
  OpOr,
  OpXor,
  OpNot,
  OpEq,
  OpNeq,
  LParen,
  RParen,
  EndOfInput,
  Error
};

struct Token {
  TokenType type = TokenType::EndOfInput;
  QString text;
  int pos = 0;
};

class Lexer {
public:
  explicit Lexer(const QString &input)
      : m_src(input), m_pos(0), m_len(input.length()) {}

  Token nextToken() {
    skipWhitespace();
    if (m_pos >= m_len) {
      return {TokenType::EndOfInput, "", m_pos};
    }

    int startPos = m_pos;
    QChar ch = m_src[m_pos];

    if (ch == '(') {
      m_pos++;
      return {TokenType::LParen, "(", startPos};
    }
    if (ch == ')') {
      m_pos++;
      return {TokenType::RParen, ")", startPos};
    }
    if (ch == '&') {
      m_pos++;
      if (m_pos < m_len && m_src[m_pos] == '&')
        m_pos++;
      return {TokenType::OpAnd, "&&", startPos};
    }
    if (ch == '|') {
      m_pos++;
      if (m_pos < m_len && m_src[m_pos] == '|')
        m_pos++;
      return {TokenType::OpOr, "||", startPos};
    }
    if (ch == '^') {
      m_pos++;
      return {TokenType::OpXor, "^", startPos};
    }
    if (ch == '!') {
      m_pos++;
      if (m_pos < m_len && m_src[m_pos] == '=') {
        m_pos++;
        return {TokenType::OpNeq, "!=", startPos};
      }
      return {TokenType::OpNot, "!", startPos};
    }
    if (ch == '~') {
      m_pos++;
      return {TokenType::OpNot, "!", startPos};
    }
    if (ch == '=') {
      m_pos++;
      if (m_pos < m_len && m_src[m_pos] == '=')
        m_pos++;
      return {TokenType::OpEq, "==", startPos};
    }
    if (ch == '/') {
      if (m_pos + 1 < m_len && m_src[m_pos + 1] == '=') {
        m_pos += 2;
        return {TokenType::OpNeq, "!=", startPos};
      }
    }

    // Numbers: 0, 1, 1'b0, 1'b1, '0', '1', 0x...
    if (ch.isDigit() || ch == '\'') {
      while (m_pos < m_len && (m_src[m_pos].isLetterOrNumber() ||
                               m_src[m_pos] == '\'' || m_src[m_pos] == '_')) {
        m_pos++;
      }
      QString numStr = m_src.mid(startPos, m_pos - startPos);
      return {TokenType::Number, numStr, startPos};
    }

    // Identifiers or keyword operators (AND, OR, NOT, XOR)
    if (ch.isLetter() || ch == '_') {
      while (m_pos < m_len && (m_src[m_pos].isLetterOrNumber() ||
                               m_src[m_pos] == '_' || m_src[m_pos] == '.')) {
        m_pos++;
      }
      QString idStr = m_src.mid(startPos, m_pos - startPos);

      // Check for keyword operators (case-insensitive)
      if (idStr.compare("AND", Qt::CaseInsensitive) == 0) {
        return {TokenType::OpAnd, "&&", startPos};
      }
      if (idStr.compare("OR", Qt::CaseInsensitive) == 0) {
        return {TokenType::OpOr, "||", startPos};
      }
      if (idStr.compare("NOT", Qt::CaseInsensitive) == 0) {
        return {TokenType::OpNot, "!", startPos};
      }
      if (idStr.compare("XOR", Qt::CaseInsensitive) == 0) {
        return {TokenType::OpXor, "^", startPos};
      }

      return {TokenType::Identifier, idStr, startPos};
    }

    // Unrecognized character
    m_pos++;
    return {TokenType::Error, QString(ch), startPos};
  }

private:
  void skipWhitespace() {
    while (m_pos < m_len && m_src[m_pos].isSpace()) {
      m_pos++;
    }
  }

  QString m_src;
  int m_pos = 0;
  int m_len = 0;
};

bool findFieldInModel(const RegMapTreeModel *model,
                      const QString &currentBlkName, const QStringList &parts,
                      uint32_t &outLsb, uint32_t &outWidth, QString &outRegName,
                      QString &outFldName, QString *outError) {
  if (!model)
    return false; // GCOV_EXCL_LINE - Defensive null model check

  QString targetBlk;
  QString targetReg;
  QString targetFld;

  if (parts.size() == 2) {
    targetBlk = currentBlkName;
    targetReg = parts[0];
    targetFld = parts[1];
  } else if (parts.size() >= 3) {
    targetBlk = parts[0];
    targetReg = parts[1];
    targetFld = parts[2];
  // GCOV_EXCL_START - Defensive parts size guard
  } else {
    if (outError)
      *outError = QString("Invalid register.field reference: '%1'")
                      .arg(parts.join('.'));
    return false;
  }
  // GCOV_EXCL_STOP

  std::function<bool(RegMapTreeItem *)> searchTree =
      [&](RegMapTreeItem *item) -> bool {
    if (!item)
      return false; // GCOV_EXCL_LINE - Defensive null item check

    if (item->kind() == RegMapTreeItem::e_rmmKind::reg) {
      QString rName = item->data("Name").toString();
      if (rName.compare(targetReg, Qt::CaseInsensitive) == 0) {
        if (!targetBlk.isEmpty()) {
          RegMapTreeItem *blk = item->parentItem();
          if (blk && blk->kind() == RegMapTreeItem::e_rmmKind::blk) {
            QString bName = blk->data("Name").toString();
            if (bName.compare(targetBlk, Qt::CaseInsensitive) != 0) {
              return false;
            }
          }
        }

        for (int i = 0; i < item->childCount(); ++i) {
          RegMapTreeItem *child = item->child(i);
          if (child && child->kind() == RegMapTreeItem::e_rmmKind::fld) {
            QString fName = child->data("Name").toString();
            if (fName.compare(targetFld, Qt::CaseInsensitive) == 0) {
              bool okLsb = false;
              bool okWidth = false;
              outLsb = child->data("Offset/LSB").toUInt(&okLsb);
              outWidth = child->data("Size/Width").toUInt(&okWidth);
              if (!okWidth || outWidth == 0)
                outWidth = 1;
              outRegName = rName;
              outFldName = fName;
              return true;
            }
          }
        }
      }
    }

    for (int i = 0; i < item->childCount(); ++i) {
      if (searchTree(item->child(i)))
        return true;
    }
    return false;
  };

  if (!searchTree(model->rootItem())) {
    if (outError) {
      *outError =
          QString("Reference '%1' could not be resolved in register map.")
              .arg(parts.join('.'));
    }
    return false;
  }

  return true;
}

ParsedLockResult parseSingle(const QString &cleanExpr,
                             const RegMapTreeModel *model,
                             const QString &currentBlkName) {
  ParsedLockResult res;
  QString input = cleanExpr.trimmed();

  // GCOV_EXCL_START - Defensive empty input guard
  if (input.isEmpty()) {
    res.valid = true;
    return res;
  }
  // GCOV_EXCL_STOP

  Lexer lexer(input);
  QVector<Token> tokens;
  int parenDepth = 0;

  while (true) {
    Token tok = lexer.nextToken();
    if (tok.type == TokenType::EndOfInput) {
      break;
    }
    if (tok.type == TokenType::Error) {
      res.valid = false;
      res.errorMessage = QString("Unexpected character '%1' at position %2.")
                             .arg(tok.text)
                             .arg(tok.pos);
      return res;
    }
    if (tok.type == TokenType::LParen) {
      parenDepth++;
    } else if (tok.type == TokenType::RParen) {
      parenDepth--;
      if (parenDepth < 0) {
        res.valid = false;
        res.errorMessage =
            QString("Unmatched closing parenthesis at position %1.")
                .arg(tok.pos);
        return res;
      }
    }
    tokens.append(tok);
  }

  if (parenDepth != 0) {
    res.valid = false;
    res.errorMessage =
        "Mismatched opening parenthesis (unclosed parentheses in expression).";
    return res;
  }

  // GCOV_EXCL_START - Defensive empty tokens guard
  if (tokens.isEmpty()) {
    res.valid = true;
    return res;
  }
  // GCOV_EXCL_STOP

  // Basic syntax validation (consecutive operators, dangling operators)
  for (int i = 0; i < tokens.size(); ++i) {
    TokenType t = tokens[i].type;
    bool isBinaryOp = (t == TokenType::OpAnd || t == TokenType::OpOr ||
                       t == TokenType::OpXor || t == TokenType::OpEq ||
                       t == TokenType::OpNeq);
    if (isBinaryOp) {
      if (i == 0 || i == tokens.size() - 1) {
        res.valid = false;
        res.errorMessage = QString("Binary operator '%1' cannot be at the "
                                   "start or end of expression.")
                               .arg(tokens[i].text);
        return res;
      }
      TokenType prev = tokens[i - 1].type;
      TokenType next = tokens[i + 1].type;
      if (prev == TokenType::OpAnd || prev == TokenType::OpOr ||
          prev == TokenType::OpXor || prev == TokenType::OpNot ||
          prev == TokenType::LParen) {
        res.valid = false;
        res.errorMessage = QString("Unexpected operator sequence around '%1'.")
                               .arg(tokens[i].text);
        return res;
      }
      if (next == TokenType::OpAnd || next == TokenType::OpOr ||
          next == TokenType::OpXor || next == TokenType::RParen) {
        res.valid = false;
        res.errorMessage = QString("Unexpected operator sequence after '%1'.")
                               .arg(tokens[i].text);
        return res;
      }
    }
  }

  QSet<QString> uniqueExtSignals;
  QSet<QString> uniqueFldRefs;
  QString agnosticOutput;
  QString svOutput;
  QString vOutput;
  QString vhdOutput;

  static const QRegularExpression validIdRegex("^[a-zA-Z_][a-zA-Z0-9_]*$");

  for (int i = 0; i < tokens.size(); ++i) {
    const Token &t = tokens[i];
    if (i > 0 && t.type != TokenType::RParen &&
        tokens[i - 1].type != TokenType::LParen &&
        tokens[i - 1].type != TokenType::OpNot) {
      agnosticOutput += " ";
      svOutput += " ";
      vOutput += " ";
      vhdOutput += " ";
    }

    switch (t.type) {
    case TokenType::OpAnd:
      agnosticOutput += "&&";
      svOutput += "&&";
      vOutput += "&&";
      vhdOutput += "and";
      break;
    case TokenType::OpOr:
      agnosticOutput += "||";
      svOutput += "||";
      vOutput += "||";
      vhdOutput += "or";
      break;
    case TokenType::OpXor:
      agnosticOutput += "^";
      svOutput += "^";
      vOutput += "^";
      vhdOutput += "xor";
      break;
    case TokenType::OpNot:
      agnosticOutput += "!";
      svOutput += "!";
      vOutput += "!";
      vhdOutput += "not ";
      break;
    case TokenType::OpEq:
      agnosticOutput += "==";
      svOutput += "==";
      vOutput += "==";
      vhdOutput += "=";
      break;
    case TokenType::OpNeq:
      agnosticOutput += "!=";
      svOutput += "!=";
      vOutput += "!=";
      vhdOutput += "/=";
      break;
    case TokenType::LParen:
      agnosticOutput += "(";
      svOutput += "(";
      vOutput += "(";
      vhdOutput += "(";
      break;
    case TokenType::RParen:
      agnosticOutput += ")";
      svOutput += ")";
      vOutput += ")";
      vhdOutput += ")";
      break;
    case TokenType::Number: {
      QString num = t.text;
      agnosticOutput += num;
      if (num == "1" || num == "1'b1" || num == "'1'") {
        svOutput += "1'b1";
        vOutput += "1'b1";
        vhdOutput += "'1'";
      } else if (num == "0" || num == "1'b0" || num == "'0'") {
        svOutput += "1'b0";
        vOutput += "1'b0";
        vhdOutput += "'0'";
      } else {
        svOutput += num;
        vOutput += num;
        vhdOutput += num;
      }
      break;
    }
    case TokenType::Identifier: {
      QString id = t.text;
      if (id.contains('.')) {
        // Register / field reference (e.g. CTRL.LOCK or BLK.CTRL.LOCK)
        QStringList parts = id.split('.');
        uniqueFldRefs.insert(id);
        uint32_t lsb = 0;
        uint32_t width = 1;
        QString rName;
        QString fName;
        QString err;

        if (model && !findFieldInModel(model, currentBlkName, parts, lsb, width,
                                       rName, fName, &err)) {
          res.valid = false;
          res.errorMessage = err;
          return res;
        }

        if (rName.isEmpty()) {
          rName = parts.size() >= 2 ? parts[parts.size() - 2] : "reg";
        }

        QString svFldRef;
        QString vhdFldRef;
        if (width == 1) {
          svFldRef = QString("reg_%1_q[%2]").arg(rName.toLower()).arg(lsb);
          vhdFldRef =
              QString("reg_%1_q(%2) = '1'").arg(rName.toLower()).arg(lsb);
        } else {
          svFldRef = QString("reg_%1_q[%2 +: %3]")
                         .arg(rName.toLower())
                         .arg(lsb)
                         .arg(width);
          vhdFldRef = QString("reg_%1_q(%2 downto %3)")
                          .arg(rName.toLower())
                          .arg(lsb + width - 1)
                          .arg(lsb);
        }

        agnosticOutput += id;
        svOutput += svFldRef;
        vOutput += svFldRef;
        vhdOutput += vhdFldRef;
      } else {
        // External input signal
        // GCOV_EXCL_START - Defensive regex validation
        if (!validIdRegex.match(id).hasMatch()) {
          res.valid = false;
          res.errorMessage =
              QString("Invalid signal name '%1' in lock expression.").arg(id);
          return res;
        }
        // GCOV_EXCL_STOP
        uniqueExtSignals.insert(id);
        agnosticOutput += id;
        svOutput += id;
        vOutput += id;
        vhdOutput += QString("(%1 = '1')").arg(id);
      }
      break;
    }
    // GCOV_EXCL_START - Defensive switch default
    default:
      break;
    // GCOV_EXCL_STOP
    }
  }

  res.valid = true;
  res.expression = agnosticOutput;
  res.externalSignals = uniqueExtSignals.values();
  std::sort(res.externalSignals.begin(), res.externalSignals.end());
  res.registerFieldRefs = uniqueFldRefs.values();
  std::sort(res.registerFieldRefs.begin(), res.registerFieldRefs.end());
  res.svExpr = svOutput;
  res.vExpr = vOutput;
  res.vhdExpr = vhdOutput;

  return res;
}

} // namespace

ParsedLockResult LockParser::parse(const QString &rawExpr,
                                   const RegMapTreeModel *model,
                                   const QString &currentBlkName) {
  ParsedLockResult res;
  QString trimmed = rawExpr.trimmed();

  if (trimmed.isEmpty()) {
    res.valid = true;
    res.scope = LockScope::Write;
    return res;
  }

  // Handle legacy scope prefixes if present for backward compatibility:
  bool hasWr = false;
  bool hasRd = false;
  QString wrExpr;
  QString rdExpr;
  LockScope scope = LockScope::Write;

  if (trimmed.contains(';')) {
    QStringList parts = trimmed.split(';', Qt::SkipEmptyParts);
    for (const QString &part : parts) {
      QString seg = part.trimmed();
      if (seg.startsWith("[w]", Qt::CaseInsensitive)) {
        hasWr = true;
        wrExpr = seg.mid(3).trimmed();
      } else if (seg.startsWith("[write]", Qt::CaseInsensitive)) {
        hasWr = true;
        wrExpr = seg.mid(7).trimmed();
      } else if (seg.startsWith("[r]", Qt::CaseInsensitive)) {
        hasRd = true;
        rdExpr = seg.mid(3).trimmed();
      } else if (seg.startsWith("[read]", Qt::CaseInsensitive)) {
        hasRd = true;
        rdExpr = seg.mid(6).trimmed();
      } else if (seg.startsWith("[rw]", Qt::CaseInsensitive)) {
        hasWr = true;
        hasRd = true;
        wrExpr = seg.mid(4).trimmed();
        rdExpr = wrExpr;
      } else if (seg.startsWith("[both]", Qt::CaseInsensitive)) {
        hasWr = true;
        hasRd = true;
        wrExpr = seg.mid(6).trimmed();
        rdExpr = wrExpr;
      } else {
        if (!hasWr) {
          hasWr = true;
          wrExpr = seg;
        } else {
          hasRd = true;
          rdExpr = seg;
        }
      }
    }
    if (hasWr && hasRd && wrExpr != rdExpr) {
      scope = LockScope::Independent;
    } else if (hasWr && hasRd) {
      scope = LockScope::Both;
    } else if (hasRd) {
      scope = LockScope::Read;
    } else {
      scope = LockScope::Write;
    }
  } else if (trimmed.startsWith("[w]", Qt::CaseInsensitive)) {
    hasWr = true;
    scope = LockScope::Write;
    wrExpr = trimmed.mid(3).trimmed();
  } else if (trimmed.startsWith("[write]", Qt::CaseInsensitive)) {
    hasWr = true;
    scope = LockScope::Write;
    wrExpr = trimmed.mid(7).trimmed();
  } else if (trimmed.startsWith("[r]", Qt::CaseInsensitive)) {
    hasRd = true;
    scope = LockScope::Read;
    rdExpr = trimmed.mid(3).trimmed();
  } else if (trimmed.startsWith("[read]", Qt::CaseInsensitive)) {
    hasRd = true;
    scope = LockScope::Read;
    rdExpr = trimmed.mid(6).trimmed();
  } else if (trimmed.startsWith("[rw]", Qt::CaseInsensitive)) {
    hasWr = true;
    hasRd = true;
    scope = LockScope::Both;
    wrExpr = trimmed.mid(4).trimmed();
    rdExpr = wrExpr;
  } else if (trimmed.startsWith("[both]", Qt::CaseInsensitive)) {
    hasWr = true;
    hasRd = true;
    scope = LockScope::Both;
    wrExpr = trimmed.mid(6).trimmed();
    rdExpr = wrExpr;
  } else {
    // Pure language-agnostic expression (default)
    hasWr = true;
    scope = LockScope::Write;
    wrExpr = trimmed;
  }

  QSet<QString> allSignals;
  QSet<QString> allRefs;

  res.scope = scope;
  res.hasWriteLock = hasWr;
  res.hasReadLock = hasRd;

  if (hasWr && !wrExpr.isEmpty()) {
    ParsedLockResult rWr = parseSingle(wrExpr, model, currentBlkName);
    if (!rWr.valid) {
      return rWr;
    }
    res.writeExpr = rWr.expression;
    res.svWriteExpr = rWr.svExpr;
    res.vWriteExpr = rWr.vExpr;
    res.vhdWriteExpr = rWr.vhdExpr;
    allSignals.unite(
        QSet<QString>(rWr.externalSignals.begin(), rWr.externalSignals.end()));
    allRefs.unite(QSet<QString>(rWr.registerFieldRefs.begin(),
                                rWr.registerFieldRefs.end()));
  }

  if (hasRd && !rdExpr.isEmpty()) {
    ParsedLockResult rRd = parseSingle(rdExpr, model, currentBlkName);
    if (!rRd.valid) {
      return rRd;
    }
    res.readExpr = rRd.expression;
    res.svReadExpr = rRd.svExpr;
    res.vReadExpr = rRd.vExpr;
    res.vhdReadExpr = rRd.vhdExpr;
    allSignals.unite(
        QSet<QString>(rRd.externalSignals.begin(), rRd.externalSignals.end()));
    allRefs.unite(QSet<QString>(rRd.registerFieldRefs.begin(),
                                rRd.registerFieldRefs.end()));
  }

  res.valid = true;
  res.expression = res.writeExpr.isEmpty() ? res.readExpr : res.writeExpr;
  res.externalSignals = allSignals.values();
  std::sort(res.externalSignals.begin(), res.externalSignals.end());
  res.registerFieldRefs = allRefs.values();
  std::sort(res.registerFieldRefs.begin(), res.registerFieldRefs.end());

  // Backward compatibility fields
  res.svExpr = res.svWriteExpr.isEmpty() ? res.svReadExpr : res.svWriteExpr;
  res.vExpr = res.vWriteExpr.isEmpty() ? res.vReadExpr : res.vWriteExpr;
  res.vhdExpr = res.vhdWriteExpr.isEmpty() ? res.vhdReadExpr : res.vhdWriteExpr;

  return res;
}

QStringList LockParser::extractExternalSignals(const QString &rawExpr) {
  ParsedLockResult res = parse(rawExpr, nullptr);
  return res.externalSignals;
}

bool LockParser::isValidSyntax(const QString &rawExpr, QString *errorMessage) {
  ParsedLockResult res = parse(rawExpr, nullptr);
  if (errorMessage && !res.valid) {
    *errorMessage = res.errorMessage;
  }
  return res.valid;
}

QString LockParser::formatLockString(LockScope scope, const QString &writeExpr,
                                     const QString &readExpr) {
  QString wr = writeExpr.trimmed();
  QString rd = readExpr.trimmed();

  switch (scope) {
  case LockScope::Write:
    return QString("[w] %1").arg(wr);
  case LockScope::Read:
    return QString("[r] %1").arg(rd);
  case LockScope::Both:
    return QString("[rw] %1").arg(wr);
  case LockScope::Independent:
    if (!wr.isEmpty() && !rd.isEmpty())
      return QString("[w] %1; [r] %2").arg(wr, rd);
    if (!wr.isEmpty())
      return QString("[w] %1").arg(wr);
    return QString("[r] %1").arg(rd);
  }
  return wr;
}

QString LockParser::formatBadgeText(const QString &rawExpr) {
  QString trimmed = rawExpr.trimmed();
  if (trimmed.isEmpty())
    return QString();

  if (trimmed.startsWith("🔒"))
    return trimmed;

  // Clean any old legacy scope tags if present
  QString display = trimmed;
  if (display.startsWith("[w] ", Qt::CaseInsensitive))
    display = display.mid(4).trimmed();
  else if (display.startsWith("[r] ", Qt::CaseInsensitive))
    display = display.mid(4).trimmed();
  else if (display.startsWith("[rw] ", Qt::CaseInsensitive))
    display = display.mid(5).trimmed();

  return QString("🔒 %1").arg(display);
}

QString LockParser::toSystemVerilog(const QString &rawExpr,
                                    const RegMapTreeModel *model,
                                    const QString &currentBlkName) {
  ParsedLockResult res = parse(rawExpr, model, currentBlkName);
  return res.svExpr;
}

QString LockParser::toVerilog(const QString &rawExpr,
                              const RegMapTreeModel *model,
                              const QString &currentBlkName) {
  ParsedLockResult res = parse(rawExpr, model, currentBlkName);
  return res.vExpr;
}

QString LockParser::toVhdl(const QString &rawExpr, const RegMapTreeModel *model,
                           const QString &currentBlkName) {
  ParsedLockResult res = parse(rawExpr, model, currentBlkName);
  return res.vhdExpr;
}
