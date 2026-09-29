/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QHeaderView>
#include <QImage>
#include <QItemSelectionModel>
#include <QTreeView>
#include <cstring>
#include <iostream>

#include "AppSettings.hpp"
#include "PreferencesWindow.hpp"
#include "RegBitfieldBarWidget.hpp"
#include "RegConfigWindow.hpp"
#include "RegMapWindow.hpp"

static bool imagesEqual(const QImage &img1, const QImage &img2) {
  if (img1.isNull() || img2.isNull() || img1.size() != img2.size()) {
    return false;
  }
  QImage c1 = img1.convertToFormat(QImage::Format_ARGB32);
  QImage c2 = img2.convertToFormat(QImage::Format_ARGB32);
  if (c1.sizeInBytes() != c2.sizeInBytes()) {
    return false;
  }
  return std::memcmp(c1.constBits(), c2.constBits(), c1.sizeInBytes()) == 0;
}

static bool saveScreenshot(const QPixmap &pixmap, const QString &path) {
  QImage newImg = pixmap.toImage();
  if (QFile::exists(path)) {
    QImage existingImg(path);
    if (imagesEqual(existingImg, newImg)) {
      std::cout << "  - Unchanged (kept): " << path.toStdString() << " ("
                << newImg.width() << "x" << newImg.height() << ")" << std::endl;
      return true;
    }
  }
  if (newImg.save(path, "PNG")) {
    std::cout << "  ✓ Captured: " << path.toStdString() << " ("
              << newImg.width() << "x" << newImg.height() << ")" << std::endl;
    return true;
  }
  std::cerr << "  ✗ Failed to save: " << path.toStdString() << std::endl;
  return false;
}

int main(int argc, char *argv[]) {
  qputenv("QT_QPA_PLATFORM", "offscreen");
  qputenv("RMAP_CONFIG_FILE", "work/rmap_screenshots.conf");
  QApplication app(argc, argv);

  // Apply clean Solarized-Dark theme and standard color profile
  AppSettings::instance().setColorScheme("solarized8");
  AppSettings::instance().setColorBlindMode(false);

  QString outDir = "docs/images";
  if (argc > 1) {
    outDir = QString::fromUtf8(argv[1]);
  }
  QDir().mkpath(outDir);

  std::cout << "[INFO] Generating documentation GUI screenshots into: "
            << outDir.toStdString() << std::endl;

  // 1. Dual-pane layout and bitfield visualizer with spi.rmt
  {
    RegMapWindow win("examples/peripherals/spi/spi.rmt");
    win.resize(1200, 750);
    win.show();

    auto *tree = win.findChild<QTreeView *>("treeView");
    if (tree && tree->model()) {
      tree->expandAll();
      // Select row 0 of block 0 (CTRL register)
      auto blockIndex = tree->model()->index(0, 0);
      if (blockIndex.isValid()) {
        auto ctrlIndex = tree->model()->index(0, 0, blockIndex);
        if (ctrlIndex.isValid()) {
          tree->selectionModel()->select(ctrlIndex,
                                         QItemSelectionModel::ClearAndSelect |
                                             QItemSelectionModel::Rows);
          tree->setCurrentIndex(ctrlIndex);
        }
      }
    }
    app.processEvents();

    // 1a. Full dual-pane window
    QPixmap fullWin = win.grab();
    QString path1 = outDir + "/gui_dual_pane_overview.png";
    saveScreenshot(fullWin, path1);

    // 1b. Close-up of the bitfield bar visualizer
    if (win.bitfieldWidget()) {
      QPixmap barPix = win.bitfieldWidget()->grab();
      QString path1b = outDir + "/gui_bitfield_bar_visualizer.png";
      saveScreenshot(barPix, path1b);
    }
  }

  // 2. Address space memory map diagram with address_gap_example.rmt
  {
    RegMapWindow win("examples/features/address_gap_example/address_gap_example.rmt");
    win.resize(1200, 750);
    win.show();

    auto *tree = win.findChild<QTreeView *>("treeView");
    if (tree && tree->model()) {
      tree->expandAll();
      // Select Sparse_Device block (row 0)
      auto blockIndex = tree->model()->index(0, 0);
      if (blockIndex.isValid()) {
        tree->selectionModel()->select(blockIndex,
                                       QItemSelectionModel::ClearAndSelect |
                                           QItemSelectionModel::Rows);
        tree->setCurrentIndex(blockIndex);
      }
    }
    app.processEvents();

    QPixmap mapWin = win.grab();
    QString path2 = outDir + "/gui_address_space_memory_map.png";
    saveScreenshot(mapWin, path2);
  }

  // 3. Project Configuration Dialog
  {
    RegMapWindow win("examples/peripherals/spi/spi.rmt");
    win.show();
    app.processEvents();

    auto *cfg = win.configWindow();
    if (cfg) {
      cfg->resize(960, 640);
      cfg->show();
      app.processEvents();

      QPixmap cfgPix = cfg->grab();
      QString path3 = outDir + "/gui_project_configuration.png";
      saveScreenshot(cfgPix, path3);
      cfg->close();
    }
  }

  // 4. Preferences & Color-Blind Settings Dialog
  {
    RegMapWindow win;
    win.show();
    app.processEvents();

    auto *pref = win.preferencesWindow();
    if (pref) {
      pref->adjustSize();
      pref->show();
      app.processEvents();

      QPixmap prefPix = pref->grab();
      QString path4 = outDir + "/gui_preferences_window.png";
      saveScreenshot(prefPix, path4);
      pref->close();
    }
  }

  std::cout
      << "[INFO] All GUI documentation screenshots generated successfully."
      << std::endl;
  return 0;
}
