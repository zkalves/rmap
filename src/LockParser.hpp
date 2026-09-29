/* This file is part of rmap.
 *
 * rmap is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rmap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rmap.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef LOCK_PARSER_HPP
#define LOCK_PARSER_HPP

#include <QString>
#include <QStringList>
#include <QVector>

class RegMapTreeModel;

enum class LockScope {
  Write,      // Protects from software writes
  Read,       // Protects from software reads (returns 0)
  Both,       // Protects from both software reads and writes
  Independent // Distinct write and read conditions
};

struct ParsedLockResult {
  bool valid = false;
  QString errorMessage;
  QString expression; // Clean language-agnostic expression
  QStringList
      externalSignals; // Deduplicated list of external hardware input signals
  QStringList registerFieldRefs; // List of referenced REG.FIELD strings

  // Backward-compatible scope & expression fields:
  LockScope scope = LockScope::Write;
  bool hasWriteLock = false;
  QString writeExpr;
  QString svWriteExpr;
  QString vWriteExpr;
  QString vhdWriteExpr;

  bool hasReadLock = false;
  QString readExpr;
  QString svReadExpr;
  QString vReadExpr;
  QString vhdReadExpr;

  QString svExpr;
  QString vExpr;
  QString vhdExpr;
};

class LockParser {
public:
  /**
   * @brief Parse and validate a language-agnostic lock expression.
   * Supports flexible boolean syntax: C-style (&&, ||, !, ^, ==, !=) and
   * textual (AND, OR, NOT, XOR), parentheses, numbers (0, 1, 1'b0, 1'b1, '0',
   * '1', 0x...), external signals, and register.field references.
   *
   * @param rawExpr The lock expression
   * @param model Optional pointer to the active RegMapTreeModel to validate
   * register/field references
   * @param currentBlkName Optional name of current block context
   * @return ParsedLockResult containing validity, external signals, field refs,
   * and normalized expression
   */
  static ParsedLockResult parse(const QString &rawExpr,
                                const RegMapTreeModel *model = nullptr,
                                const QString &currentBlkName = QString());

  /**
   * @brief Quick extraction of external hardware signals from an expression
   * without full model lookup.
   */
  static QStringList extractExternalSignals(const QString &rawExpr);

  /**
   * @brief Check whether an expression is syntactically valid (balanced parens,
   * valid tokens/operators).
   */
  static bool isValidSyntax(const QString &rawExpr,
                            QString *errorMessage = nullptr);

  /**
   * @brief Format lock components into a canonical lock string.
   */
  static QString formatLockString(LockScope scope, const QString &writeExpr,
                                  const QString &readExpr = QString());

  /**
   * @brief Format lock text for UI badge display (e.g. "🔒 <expr>").
   */
  static QString formatBadgeText(const QString &rawExpr);

  // Language conversion utilities for templates and previews:
  static QString toSystemVerilog(const QString &rawExpr,
                                 const RegMapTreeModel *model = nullptr,
                                 const QString &currentBlkName = QString());

  static QString toVerilog(const QString &rawExpr,
                           const RegMapTreeModel *model = nullptr,
                           const QString &currentBlkName = QString());

  static QString toVhdl(const QString &rawExpr,
                        const RegMapTreeModel *model = nullptr,
                        const QString &currentBlkName = QString());
};

#endif // LOCK_PARSER_HPP
