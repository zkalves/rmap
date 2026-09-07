/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include "AboutWindow.hpp"
#include "AppSettings.hpp"
#include <QApplication>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QMoveEvent>

AboutWindow::AboutWindow(QWidget *parent) :
    QDialog(parent, Qt::Window)
{
    setupUi(this);

    setWindowTitle(tr("About rmap"));
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    resize(620, 500);
    setMinimumSize(520, 400);

    initContent();
    restoreWindowStateFromSettings();
}

QString AboutWindow::applicationVersion() const
{
    QString ver = QApplication::applicationVersion();
    return ver.isEmpty() ? QStringLiteral("v0.2.0") : ver;
}

QString AboutWindow::applicationName() const
{
    QString name = QApplication::applicationName();
    return name.isEmpty() ? QStringLiteral("rmap") : name;
}

void AboutWindow::initContent()
{
    QString ver = applicationVersion();
    this->labelVersion->setText(tr("Version %1").arg(ver));

    // Tab 1: About
    QString aboutHtml = QStringLiteral(
        "<p><b>rmap</b> is a high-performance, open-source hardware register map design, "
        "architectural validation, and multi-target code generation environment built with "
        "<b>Modern C++ (C++17)</b> and <b>Qt 6</b>.</p>"
        "<p>Designed specifically for <b>ASIC</b>, <b>FPGA</b>, <b>Verification</b>, and "
        "<b>Embedded Firmware</b> engineers, rmap unifies hardware address space specification "
        "across the entire chip design lifecycle.</p>"
        "<h4>Key Highlights:</h4>"
        "<ul>"
        "<li><b>Hierarchical Modeling:</b> Intuitive structure covering Memory Maps, Register Blocks, Registers, Bitfields, and Memories.</li>"
        "<li><b>Real-Time ASIC Linter:</b> Automatic DRC checks detecting keyword collisions, capacity overflow, contradictory access policies, and missing reset definitions.</li>"
        "<li><b>Interactive Visualizers:</b> Dynamic 32/64-bit Bitfield Slice Visualizer and vertical Block Memory Map widget with address gap detection.</li>"
        "<li><b>Accessibility:</b> High-contrast barrier-free CVD (Okabe-Ito / Wong) color-blind palette with textual tags.</li>"
        "<li><b>Undo/Redo Stack:</b> Full command pattern history for all editing, insertion, deletion, and duplication operations.</li>"
        "</ul>"
    );
    this->textBrowserAbout->setHtml(aboutHtml);

    // Tab 2: Features
    QString featuresHtml = QStringLiteral(
        "<h4>Supported Formats (Import, Export & Conversion):</h4>"
        "<ul>"
        "<li><b>Accellera SystemRDL:</b> SystemRDL 1.0 and 2.0 specifications (<code>.rdl</code>).</li>"
        "<li><b>ARM CMSIS-SVD:</b> Cortex-M peripheral description XML (<code>.svd</code>).</li>"
        "<li><b>IEEE 1685 IP-XACT:</b> 2009, 2014, and 2022 schema XML (<code>.xml</code>, <code>.ipxact</code>).</li>"
        "<li><b>Standard JSON Schema:</b> Standardized JSON register map exchange format (<code>.json</code>).</li>"
        "<li><b>Spreadsheet Tables:</b> RFC 4180 CSV and TSV tabular data (<code>.csv</code>, <code>.tsv</code>).</li>"
        "<li><b>Native Protocol Buffers:</b> Google Protobuf text format (<code>.rmt</code>) and binary wire format (<code>.rmb</code>).</li>"
        "</ul>"
        "<h4>Code Generation Outputs (15 Inja Templates):</h4>"
        "<ul>"
        "<li><b>Hardware RTL:</b> Synthesizable generic bus-agnostic SystemVerilog register files and self-checking testbenches.</li>"
        "<li><b>Verification (UVM):</b> Complete IEEE 1800.2 UVM register models (<code>uvm_reg_block</code>, <code>uvm_reg</code>, <code>uvm_reg_field</code>) and testbench environments.</li>"
        "<li><b>Firmware & Embedded:</b> C/C++ headers with bitmasks, struct layouts, and register offset definitions.</li>"
        "<li><b>Rust:</b> Type-safe zero-cost Rust Peripheral Access Crates (PAC).</li>"
        "<li><b>Python:</b> Object-oriented Python bring-up drivers and Cocotb / PyUVM testbenches.</li>"
        "<li><b>Documentation:</b> Searchable interactive single-page HTML documentation and GitHub-flavored Markdown.</li>"
        "</ul>"
    );
    this->textBrowserFeatures->setHtml(featuresHtml);

    // Tab 3: Libraries & Credits
    QString librariesHtml = QStringLiteral(
        "<h4>Core Framework & Third-Party Dependencies:</h4>"
        "<ul>"
        "<li><b>Qt Framework:</b> Qt 6 (QtCore, QtWidgets, QtGui, QtTest) — GUI, model/view architecture, and platform abstraction.<br/>"
        "<i>Compiled with Qt %1, Running on Qt %2</i></li>"
        "<li><b>Pantor Inja:</b> Modern, expressive template engine for C++ (v3.3.0).</li>"
        "<li><b>nlohmann/json:</b> JSON for Modern C++ serialization library.</li>"
        "<li><b>Google Protocol Buffers:</b> Scalable data interchange schema (Protobuf v3).</li>"
        "<li><b>Modern C++17:</b> High-efficiency core data structures and concurrency.</li>"
        "</ul>"
    ).arg(QT_VERSION_STR, qVersion());
    this->textBrowserLibraries->setHtml(librariesHtml);

    // Tab 4: License
    QString licenseHtml = QStringLiteral(
        "<h4>Mozilla Public License Version 2.0</h4>"
        "<p><b>Copyright &copy; 2026 Ezequiel Alves. All rights reserved.</b></p>"
        "<p>This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. "
        "If a copy of the MPL was not distributed with this file, You can obtain one at "
        "<a href=\"https://mozilla.org/MPL/2.0/\">https://mozilla.org/MPL/2.0/</a>.</p>"
        "<p>rmap is free and open-source software. You are welcome to use, study, modify, and "
        "redistribute it under the conditions of the Mozilla Public License 2.0.</p>"
    );
    this->textBrowserLicense->setHtml(licenseHtml);
}

void AboutWindow::saveWindowStateToSettings()
{
    AppSettings::instance().setWindowSize("AboutWindow", size());
    AppSettings::instance().setWindowPos("AboutWindow", pos());
}

void AboutWindow::restoreWindowStateFromSettings()
{
    QSize savedSize = AppSettings::instance().windowSize("AboutWindow");
    if (savedSize.isValid()) {
        resize(savedSize);
    }
    QPoint savedPos = AppSettings::instance().windowPos("AboutWindow");
    if (!savedPos.isNull()) {
        move(savedPos);
    }
}

void AboutWindow::accept()
{
    saveWindowStateToSettings();
    QDialog::accept();
}

void AboutWindow::closeEvent(QCloseEvent *event)
{
    saveWindowStateToSettings();
    QDialog::closeEvent(event);
}

void AboutWindow::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    saveWindowStateToSettings();
}

void AboutWindow::moveEvent(QMoveEvent *event)
{
    QDialog::moveEvent(event);
    saveWindowStateToSettings();
}
