/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef REGMAPDELEGATE_HPP
#define REGMAPDELEGATE_HPP

#include <QStyledItemDelegate>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QLineEdit>
#include <QComboBox>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include "ThemeManager.hpp"

class RegMapDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit RegMapDelegate(QObject *parent = nullptr);
    explicit RegMapDelegate(const QRegularExpression &regex, QObject *parent = nullptr);
    ~RegMapDelegate() override = default;

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

protected:
    QRegularExpression m_regex;
};

class RegHexDecBinDelegate : public RegMapDelegate
{
    Q_OBJECT

public:
    explicit RegHexDecBinDelegate(QObject *parent = nullptr);
};

class RegIntDelegate : public RegMapDelegate
{
    Q_OBJECT

public:
    explicit RegIntDelegate(QObject *parent = nullptr);
};

class RegStrDelegate : public RegMapDelegate
{
    Q_OBJECT

public:
    explicit RegStrDelegate(QObject *parent = nullptr);
};

AccessColors getAccessPolicyColors(const QString &access, bool colorBlind);
AccessColors getAccessPolicyColors(const QString &access, ColorBlindMode mode);

// Delegate for UVM / SW Access Policies (RW, RO, WO, W1C, etc.)
class RegAccessPolicyDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit RegAccessPolicyDelegate(QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

// Delegate for Hardware Access Policies (RO, RW, WO, NA, W1C, etc.)
class RegHwAccessDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit RegHwAccessDelegate(QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

// Delegate for Boolean properties (Is Rand, Volatile, Has Reset)
class RegBoolDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit RegBoolDelegate(QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

#endif // REGMAPDELEGATE_HPP
