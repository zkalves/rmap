#!/usr/bin/env python3

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

"""
test_coverage.py - Automated Unit Test Suite for Multi-Metric Code Coverage Engine

Verifies:
  1. Subsystem categorization
  2. Calculation of all 6 coverage metrics (Lines, Functions, Branches, Conditions/MCDC, Calls, Blocks)
  3. Edge cases (0 total items, division by zero prevention)
  4. Markdown, HTML, JSON, and console rendering
  5. README.md marker-based badge and table injection (idempotence)
  6. Threshold evaluation logic
"""

import json
import os
import shutil
import sys
import unittest
from pathlib import Path

# Add project root and script directory to path
PROJECT_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(PROJECT_ROOT / "script"))

from generate_coverage import (
    categorize_subsystem,
    compute_metrics,
    get_color_for_percent,
    render_console_summary,
    render_html_report,
    render_markdown_report,
    update_readme,
)


class TestCoverageEngine(unittest.TestCase):
    def setUp(self):
        self.work_dir = PROJECT_ROOT / "work" / "test_coverage_engine"
        if self.work_dir.exists():
            shutil.rmtree(self.work_dir)
        self.work_dir.mkdir(parents=True, exist_ok=True)

    def tearDown(self):
        if self.work_dir.exists():
            shutil.rmtree(self.work_dir)

    def test_subsystem_categorization(self):
        self.assertEqual(categorize_subsystem("src/format/JsonHandler.cpp"), "Format Parsers & Serializers")
        self.assertEqual(categorize_subsystem("src/format/SystemRdlHandler.cpp"), "Format Parsers & Serializers")
        self.assertEqual(categorize_subsystem("src/RegMapTreeModel.cpp"), "Core Architecture & Model")
        self.assertEqual(categorize_subsystem("src/RegMapTreeItem.cpp"), "Core Architecture & Model")
        self.assertEqual(categorize_subsystem("src/UndoCommands.hpp"), "Core Architecture & Model")
        self.assertEqual(categorize_subsystem("src/RegBitfieldBarWidget.cpp"), "GUI Widgets & Main Window")
        self.assertEqual(categorize_subsystem("src/BlockMemoryMapWidget.cpp"), "GUI Widgets & Main Window")
        self.assertEqual(categorize_subsystem("src/RegMapWindow.cpp"), "GUI Widgets & Main Window")
        self.assertEqual(categorize_subsystem("src/CodeGenerator.cpp"), "Code Generation Engine")
        self.assertEqual(categorize_subsystem("src/PreferencesWindow.cpp"), "Dialogs & Configuration")
        self.assertEqual(categorize_subsystem("src/RegConfigWindow.cpp"), "Dialogs & Configuration")
        self.assertEqual(categorize_subsystem("src/AboutWindow.cpp"), "Dialogs & Configuration")
        self.assertEqual(categorize_subsystem("src/ThemeManager.cpp"), "System Services & Utilities")
        self.assertEqual(categorize_subsystem("src/AppSettings.cpp"), "System Services & Utilities")
        self.assertEqual(categorize_subsystem("src/PathUtils.cpp"), "System Services & Utilities")

    def test_color_thresholds(self):
        self.assertEqual(get_color_for_percent(95.0), "brightgreen")
        self.assertEqual(get_color_for_percent(80.0), "brightgreen")
        self.assertEqual(get_color_for_percent(70.0), "green")
        self.assertEqual(get_color_for_percent(55.0), "yellow")
        self.assertEqual(get_color_for_percent(40.0), "orange")
        self.assertEqual(get_color_for_percent(20.0), "red")

    def test_compute_metrics_calculation(self):
        mock_data = {
            "src/Model.cpp": {
                "subsystem": "Core Architecture & Model",
                "lines": {1: 5, 2: 0, 3: 2, 4: 0},  # 2 of 4 covered (50.0%)
                "funcs": {"foo()": 3, "bar()": 0},   # 1 of 2 covered (50.0%)
                "branches": {(1, 0): 2, (1, 1): 0},  # 1 of 2 covered (50.0%)
                "conds": [{"count": 2, "covered": 1}], # 1 of 2 covered (50.0%)
                "calls": [{"returned": 1}, {"returned": 0}], # 1 of 2 covered (50.0%)
                "blocks_total": 4,
                "blocks_exec": 2, # 2 of 4 covered (50.0%)
            },
            "src/format/Parser.cpp": {
                "subsystem": "Format Parsers & Serializers",
                "lines": {10: 1, 11: 2, 12: 3, 13: 4}, # 4 of 4 covered (100.0%)
                "funcs": {"parse()": 10},               # 1 of 1 covered (100.0%)
                "branches": {(10, 0): 5, (10, 1): 5},   # 2 of 2 covered (100.0%)
                "conds": [{"count": 4, "covered": 4}],  # 4 of 4 covered (100.0%)
                "calls": [{"returned": 5}],             # 1 of 1 covered (100.0%)
                "blocks_total": 2,
                "blocks_exec": 2, # 2 of 2 covered (100.0%)
            },
        }

        res = compute_metrics(mock_data)
        s = res["summary"]

        # Lines: (2 + 4) / (4 + 4) = 6 / 8 = 75.0%
        self.assertEqual(s["lines"]["total"], 8)
        self.assertEqual(s["lines"]["covered"], 6)
        self.assertEqual(s["lines"]["percent"], 75.0)

        # Functions: (1 + 1) / (2 + 1) = 2 / 3 = 66.67%
        self.assertEqual(s["functions"]["total"], 3)
        self.assertEqual(s["functions"]["covered"], 2)
        self.assertEqual(s["functions"]["percent"], 66.67)

        # Branches: (1 + 2) / (2 + 2) = 3 / 4 = 75.0%
        self.assertEqual(s["branches"]["total"], 4)
        self.assertEqual(s["branches"]["covered"], 3)
        self.assertEqual(s["branches"]["percent"], 75.0)

        # Conditions: (1 + 4) / (2 + 4) = 5 / 6 = 83.33%
        self.assertEqual(s["conditions"]["total"], 6)
        self.assertEqual(s["conditions"]["covered"], 5)
        self.assertEqual(s["conditions"]["percent"], 83.33)

        # Calls: (1 + 1) / (2 + 1) = 2 / 3 = 66.67%
        self.assertEqual(s["calls"]["total"], 3)
        self.assertEqual(s["calls"]["covered"], 2)
        self.assertEqual(s["calls"]["percent"], 66.67)

        # Blocks: (2 + 2) / (4 + 2) = 4 / 6 = 66.67%
        self.assertEqual(s["blocks"]["total"], 6)
        self.assertEqual(s["blocks"]["covered"], 4)
        self.assertEqual(s["blocks"]["percent"], 66.67)

        # Subsystems verification
        self.assertEqual(len(res["subsystems"]), 2)
        subs = {sub["name"]: sub for sub in res["subsystems"]}
        self.assertIn("Core Architecture & Model", subs)
        self.assertIn("Format Parsers & Serializers", subs)
        self.assertEqual(subs["Core Architecture & Model"]["lines"]["percent"], 50.0)
        self.assertEqual(subs["Format Parsers & Serializers"]["lines"]["percent"], 100.0)

    def test_division_by_zero_prevention(self):
        empty_data = {
            "src/Empty.cpp": {
                "subsystem": "System Services & Utilities",
                "lines": {},
                "funcs": {},
                "branches": {},
                "conds": [],
                "calls": [],
                "blocks_total": 0,
                "blocks_exec": 0,
            }
        }
        res = compute_metrics(empty_data)
        s = res["summary"]
        self.assertEqual(s["lines"]["percent"], 0.0)
        self.assertEqual(s["functions"]["percent"], 0.0)
        self.assertEqual(s["branches"]["percent"], 0.0)
        self.assertEqual(s["conditions"]["percent"], 0.0)
        self.assertEqual(s["calls"]["percent"], 0.0)
        self.assertEqual(s["blocks"]["percent"], 0.0)

    def test_rendering_outputs(self):
        mock_data = {
            "src/Sample.cpp": {
                "subsystem": "Core Architecture & Model",
                "lines": {1: 1, 2: 0},
                "funcs": {"init()": 1},
                "branches": {(1, 0): 1},
                "conds": [{"count": 2, "covered": 1}],
                "calls": [{"returned": 1}],
                "blocks_total": 2,
                "blocks_exec": 1,
            }
        }
        res = compute_metrics(mock_data)

        console_txt = render_console_summary(res)
        self.assertIn("rmap CODE COVERAGE REPORT", console_txt)
        self.assertIn("Lines", console_txt)
        self.assertIn("Conditions (MC/DC)", console_txt)

        md_txt = render_markdown_report(res)
        self.assertIn("## Code Coverage Metrics", md_txt)
        self.assertIn("**Lines**", md_txt)
        self.assertIn("**Conditions (MC/DC)**", md_txt)
        self.assertIn("Architectural Subsystems Breakdown", md_txt)
        self.assertIn("Detailed Source Files Coverage", md_txt)

        html_txt = render_html_report(res)
        self.assertIn("<!DOCTYPE html>", html_txt)
        self.assertIn("rmap — Code Coverage Report", html_txt)
        self.assertIn("Line Coverage", html_txt)
        self.assertIn("Condition (MC/DC)", html_txt)
        self.assertIn("Architectural Subsystems", html_txt)

    def test_update_readme_idempotence(self):
        dummy_readme = self.work_dir / "README.md"
        dummy_readme.write_text(
            "# Test Project\n\n"
            "[![CI](https://example.com/ci.svg)](https://example.com)\n"
            "[![Qt 6](https://img.shields.io/badge/Qt-6-green.svg)](https://www.qt.io/)\n\n"
            "## Key Features\n\n- Feature 1\n\n"
            "## Quickstart & Installation\n\nBuild instructions\n",
            encoding="utf-8",
        )

        mock_data = {
            "src/Sample.cpp": {
                "subsystem": "Core Architecture & Model",
                "lines": {1: 1, 2: 1},
                "funcs": {"test()": 1},
                "branches": {(1, 0): 1},
                "conds": [{"count": 2, "covered": 2}],
                "calls": [{"returned": 1}],
                "blocks_total": 1,
                "blocks_exec": 1,
            }
        }
        metrics = compute_metrics(mock_data)

        # First update
        update_readme(dummy_readme, metrics)
        c1 = dummy_readme.read_text(encoding="utf-8")
        self.assertIn("<!-- COVERAGE_BADGES_START -->", c1)
        self.assertIn("<!-- COVERAGE_SECTION_START -->", c1)
        self.assertIn("## Code Coverage Metrics", c1)
        self.assertIn("[![Line Coverage]", c1)

        # Second update (must replace in-place without duplicate markers)
        update_readme(dummy_readme, metrics)
        c2 = dummy_readme.read_text(encoding="utf-8")
        self.assertEqual(c2.count("<!-- COVERAGE_BADGES_START -->"), 1)
        self.assertEqual(c2.count("<!-- COVERAGE_SECTION_START -->"), 1)
        self.assertEqual(c2.count("## Code Coverage Metrics"), 1)

    def test_throw_branch_filtering(self):
        mock_data = {
            "src/Sample.cpp": {
                "subsystem": "Core Architecture & Model",
                "lines": {1: 1},
                "funcs": {"test()": 1},
                "branches": {
                    (1, 0): {"count": 5, "throw": False},  # Decision branch, taken
                    (1, 1): {"count": 0, "throw": False},  # Decision branch, not taken
                    (1, 2): {"count": 0, "throw": True},   # Compiler unwind branch, not taken
                },
                "conds": [],
                "calls": [],
                "blocks_total": 1,
                "blocks_exec": 1,
            }
        }
        # By default, exclude_throw_branches=True
        m_filtered = compute_metrics(mock_data, exclude_throw_branches=True)
        s = m_filtered["summary"]
        self.assertEqual(s["branches"]["total"], 2)
        self.assertEqual(s["branches"]["covered"], 1)
        self.assertEqual(s["branches"]["percent"], 50.0)
        self.assertEqual(s["branches_raw"]["total"], 3)
        self.assertEqual(s["branches_raw"]["covered"], 1)

        # When include_throw_branches=True
        m_unfiltered = compute_metrics(mock_data, exclude_throw_branches=False)
        s_unf = m_unfiltered["summary"]
        self.assertEqual(s_unf["branches"]["total"], 3)
        self.assertEqual(s_unf["branches"]["covered"], 1)
        self.assertEqual(s_unf["branches"]["percent"], 33.33)


if __name__ == "__main__":
    unittest.main()
