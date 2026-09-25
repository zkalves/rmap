#!/usr/bin/env python3

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

"""
test_packaging.py - Automated Test Suite for Packaging & Distribution Artifacts

Verifies:
  1. CPack packaging configuration (cmake/Packaging.cmake)
  2. FreeDesktop integration (res/rmap.desktop and hicolor application icons)
  3. AppImage builder script integrity and syntax (script/build_appimage.sh)
  4. CI packaging workflow jobs in .github/workflows/ci.yml (deb, rpm, appimage)
  5. Top-level Makefile package targets (package-deb, package-rpm, package-appimage)
"""

import os
import re
import subprocess
import unittest
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent


class TestPackaging(unittest.TestCase):
    def test_cpack_configuration(self):
        pkg_cmake = PROJECT_ROOT / "cmake" / "Packaging.cmake"
        self.assertTrue(pkg_cmake.exists(), "cmake/Packaging.cmake must exist")

        content = pkg_cmake.read_text(encoding="utf-8")
        self.assertIn('set(CPACK_PACKAGE_NAME "rmap")', content)
        self.assertIn('set(CPACK_PACKAGE_VENDOR "Ezequiel Alves")', content)
        self.assertIn('set(CPACK_PACKAGE_DESCRIPTION_SUMMARY', content)
        self.assertIn('set(CPACK_GENERATOR "DEB;RPM;TGZ")', content)
        self.assertIn('set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)', content)
        self.assertIn('set(CPACK_RPM_PACKAGE_AUTOREQPROV ON)', content)
        self.assertIn('set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")', content)
        self.assertIn('include(CPack)', content)

    def test_cmakelists_packaging_integration(self):
        cmakelists = PROJECT_ROOT / "CMakeLists.txt"
        content = cmakelists.read_text(encoding="utf-8")
        self.assertIn("include(cmake/Packaging.cmake)", content)
        self.assertIn("res/rmap.desktop", content)
        self.assertIn("${CMAKE_INSTALL_DATADIR}/applications", content)
        self.assertIn("res/images/app_icon.png", content)

    def test_desktop_entry(self):
        desktop_file = PROJECT_ROOT / "res" / "rmap.desktop"
        self.assertTrue(desktop_file.exists(), "res/rmap.desktop must exist")

        content = desktop_file.read_text(encoding="utf-8")
        self.assertIn("[Desktop Entry]", content)
        self.assertIn("Type=Application", content)
        self.assertIn("Name=rmap", content)
        self.assertIn("Exec=rmap", content)
        self.assertIn("Icon=rmap", content)
        self.assertIn("Categories=", content)

    def test_icon_assets_exist(self):
        main_icon = PROJECT_ROOT / "res" / "images" / "app_icon.png"
        self.assertTrue(main_icon.exists(), "Main application icon res/images/app_icon.png must exist")

        sizes = [16, 32, 48, 64, 128, 256, 512]
        for size in sizes:
            sized_icon = PROJECT_ROOT / "res" / "images" / f"app_icon_{size}.png"
            self.assertTrue(
                sized_icon.exists(),
                f"Application icon res/images/app_icon_{size}.png must exist",
            )

    def test_build_appimage_script_syntax_and_perms(self):
        appimage_script = PROJECT_ROOT / "script" / "build_appimage.sh"
        self.assertTrue(appimage_script.exists(), "script/build_appimage.sh must exist")
        self.assertTrue(
            os.access(appimage_script, os.X_OK),
            "script/build_appimage.sh must have executable permissions",
        )

        res = subprocess.run(
            ["bash", "-n", str(appimage_script)],
            capture_output=True,
            text=True,
        )
        self.assertEqual(res.returncode, 0, f"bash -n failed on script/build_appimage.sh: {res.stderr}")

        content = appimage_script.read_text(encoding="utf-8")
        self.assertIn("APPDIR=", content)
        self.assertIn("AppRun", content)
        self.assertIn("appimagetool", content)
        self.assertIn("usr/plugins", content)

    def test_makefile_packaging_targets(self):
        makefile = PROJECT_ROOT / "Makefile"
        content = makefile.read_text(encoding="utf-8")
        self.assertIn("package-deb:", content)
        self.assertIn("package-rpm:", content)
        self.assertIn("package-appimage:", content)
        self.assertIn("package-docker", content)
        self.assertIn("package-homebrew:", content)
        self.assertIn("package-module:", content)
        self.assertTrue(re.search(r"packages?\s+package\s*:", content) or "package:" in content)

    def test_dockerfile_and_dockerignore(self):
        dockerfile = PROJECT_ROOT / "Dockerfile"
        dockerignore = PROJECT_ROOT / ".dockerignore"
        self.assertTrue(dockerfile.exists(), "Dockerfile must exist")
        self.assertTrue(dockerignore.exists(), ".dockerignore must exist")

        content = dockerfile.read_text(encoding="utf-8")
        self.assertIn("FROM ubuntu:24.04 AS builder", content)
        self.assertIn("FROM ubuntu:24.04 AS runtime", content)
        self.assertIn("ENTRYPOINT", content)
        self.assertIn("QT_QPA_PLATFORM=offscreen", content)

    def test_homebrew_formula(self):
        formula = PROJECT_ROOT / "Formula" / "rmap.rb"
        self.assertTrue(formula.exists(), "Formula/rmap.rb must exist")

        content = formula.read_text(encoding="utf-8")
        self.assertIn("class Rmap < Formula", content)
        self.assertIn('depends_on "cmake"', content)
        self.assertIn('depends_on "qt@6"', content)
        self.assertIn('depends_on "protobuf"', content)

        res = subprocess.run(["ruby", "-c", str(formula)], capture_output=True, text=True)
        self.assertEqual(res.returncode, 0, f"ruby -c failed on Formula/rmap.rb: {res.stderr}")

        gen_script = PROJECT_ROOT / "script" / "generate_homebrew_formula.py"
        self.assertTrue(gen_script.exists(), "script/generate_homebrew_formula.py must exist")
        self.assertTrue(os.access(gen_script, os.X_OK), "generate_homebrew_formula.py must be executable")

    def test_environment_modules_packaging(self):
        lua_mod = PROJECT_ROOT / "packaging" / "modules" / "rmap.lua"
        tcl_mod = PROJECT_ROOT / "packaging" / "modules" / "rmap.tcl"
        self.assertTrue(lua_mod.exists(), "packaging/modules/rmap.lua must exist")
        self.assertTrue(tcl_mod.exists(), "packaging/modules/rmap.tcl must exist")

        lua_content = lua_mod.read_text(encoding="utf-8")
        self.assertIn("whatis([[Name: rmap]])", lua_content)
        self.assertIn("prepend_path(\"PATH\"", lua_content)

        tcl_content = tcl_mod.read_text(encoding="utf-8")
        self.assertIn("#%Module1.0", tcl_content)
        self.assertIn("prepend-path PATH", tcl_content)

        mod_script = PROJECT_ROOT / "script" / "build_module_package.sh"
        self.assertTrue(mod_script.exists(), "script/build_module_package.sh must exist")
        self.assertTrue(os.access(mod_script, os.X_OK), "build_module_package.sh must be executable")

        res = subprocess.run(["bash", "-n", str(mod_script)], capture_output=True, text=True)
        self.assertEqual(res.returncode, 0, f"bash -n failed on script/build_module_package.sh: {res.stderr}")

    def test_ci_packaging_workflow(self):
        ci_yaml = PROJECT_ROOT / ".github" / "workflows" / "ci.yml"
        self.assertTrue(ci_yaml.exists(), ".github/workflows/ci.yml must exist")

        content = ci_yaml.read_text(encoding="utf-8")
        self.assertIn("package-deb:", content)
        self.assertIn("package-rpm:", content)
        self.assertIn("package-appimage:", content)
        self.assertIn("package-docker:", content)
        self.assertIn("package-homebrew:", content)
        self.assertIn("package-module:", content)
        self.assertIn("rmap-deb", content)
        self.assertIn("rmap-rpm", content)
        self.assertIn("rmap-appimage", content)
        self.assertIn("rmap-homebrew-formula", content)
        self.assertIn("rmap-module-package", content)
        self.assertIn("image: almalinux:9", content)
        self.assertIn("runs-on: ubuntu-22.04", content)


if __name__ == "__main__":
    unittest.main()

