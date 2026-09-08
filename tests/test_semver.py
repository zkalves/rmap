#!/usr/bin/env python3

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

"""
test_semver.py - Automated Test Suite for Semantic Versioning & Conventional Commits

Verifies:
  1. parse_commit_bump in bump_version.py:
     - feat! and fix! breaking changes triggering MAJOR bump
     - scoped breaking changes feat(scope)! and fix(scope)! triggering MAJOR bump
     - other breaking tags (doc!, docs!, refactor!, perf!, etc.) triggering MAJOR bump
     - BREAKING CHANGE: footers triggering MAJOR bump
     - feat / feature triggering MINOR bump
     - doc / docs / fix / bugfix / chore / test / tests / perf / refactor triggering PATCH bump
  2. bump_version mathematical SemVer transitions (major, minor, patch)
  3. commit-msg git hook validation for feat!, fix!, doc:, and aliases
"""

import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(PROJECT_ROOT / "script"))

from bump_version import bump_version, parse_commit_bump, parse_semver


class TestSemanticVersioning(unittest.TestCase):
    def test_parse_semver_valid(self):
        self.assertEqual(parse_semver("0.2.0"), (0, 2, 0))
        self.assertEqual(parse_semver("1.0.0"), (1, 0, 0))
        self.assertEqual(parse_semver("2.14.3-beta.1"), (2, 14, 3))
        self.assertEqual(parse_semver("3.0.0+build123"), (3, 0, 0))

    def test_bump_version_transitions(self):
        self.assertEqual(bump_version("0.2.0", "patch"), "0.2.1")
        self.assertEqual(bump_version("0.2.0", "minor"), "0.3.0")
        self.assertEqual(bump_version("0.2.0", "major"), "1.0.0")
        self.assertEqual(bump_version("1.5.9", "patch"), "1.5.10")
        self.assertEqual(bump_version("1.5.9", "minor"), "1.6.0")
        self.assertEqual(bump_version("1.5.9", "major"), "2.0.0")

    def test_breaking_changes_major_bump(self):
        breaking_cases = [
            "feat!: remove deprecated export method",
            "fix!: change return type of parse()",
            "feat(gui)!: redesign register layout and remove legacy tabs",
            "fix(core)!: change serialization schema",
            "doc!: complete overhaul of CLI documentation breaking links",
            "docs!: restructure docs portal",
            "refactor!: rewrite memory map widget architecture",
            "perf!: alter public buffer allocation API",
            "chore!: drop support for C++14",
            "test!: revamp test runner CLI",
            "[breaking] drop deprecated format",
            "[feat!] rewrite register delegate",
            "[fix!] critical breaking architectural patch",
            "[doc!] breaking documentation change",
            "refactor(core): internal cleanup\n\nBREAKING CHANGE: new API required",
            "fix(parser): align registers\n\nBREAKING-CHANGE: offsets must be 4-byte aligned",
        ]
        for msg in breaking_cases:
            with self.subTest(msg=msg):
                self.assertEqual(
                    parse_commit_bump(msg),
                    "major",
                    f"Commit '{msg}' should trigger MAJOR bump",
                )

    def test_feature_minor_bump(self):
        feature_cases = [
            "feat: add continuous bitfield bar widget",
            "feat(gui): support dark mode theme switching",
            "feature: add 64-bit register support",
            "feature(export): add rust PAC crate generation",
            "[feat] add new linter rule",
            "[feature] support JUnit XML report output",
        ]
        for msg in feature_cases:
            with self.subTest(msg=msg):
                self.assertEqual(
                    parse_commit_bump(msg),
                    "minor",
                    f"Commit '{msg}' should trigger MINOR bump",
                )

    def test_doc_and_other_tags_patch_bump(self):
        patch_cases = [
            "doc: update getting started guide",
            "docs: document new coverage metrics command",
            "doc(cli): explain --lint --strict options",
            "docs(api): document RegMapTreeModel methods",
            "fix: correct off-by-one address error",
            "fix(parser): resolve SystemRDL lexer syntax issue",
            "bugfix: fix null pointer dereference on file close",
            "bugfix(theme): fix high contrast palette contrast ratio",
            "perf: optimize register table search filtering",
            "refactor: extract path relativization helper",
            "test: add automated unit tests for coverage script",
            "tests: add tests for delegates",
            "chore: bump inja dependency to v3.3.0",
            "build: add ENABLE_COVERAGE cmake option",
            "ci: add coverage analysis job",
            "style: reformat according to clang-format",
            "revert: revert previous faulty commit",
            "[doc] update documentation",
            "[docs] fix typos in README",
            "[fix] fix window geometry save",
            "[bugfix] correct bit width calculation",
            "[refactor] split delegates into separate files",
            "[test] add unit tests",
            "[chore] update copyright notice",
        ]
        for msg in patch_cases:
            with self.subTest(msg=msg):
                self.assertEqual(
                    parse_commit_bump(msg),
                    "patch",
                    f"Commit '{msg}' should trigger PATCH bump",
                )

    def test_commit_msg_hook_script(self):
        hook_path = PROJECT_ROOT / "script" / "git-hooks" / "commit-msg"
        self.assertTrue(hook_path.exists())

        valid_messages = [
            "feat!: break compatibility in parser",
            "fix!: breaking fix for register offset calculation",
            "feat(core)!: breaking interface redesign",
            "fix(gui)!: breaking window layout change",
            "doc: update cli user manual",
            "doc(dev): document coverage metrics script",
            "docs: update installation instructions",
            "docs(api): document C++ delegates",
            "feature: add new peripheral template",
            "bugfix: handle empty json files cleanly",
            "tests: add coverage unit tests",
            "[doc] update manual",
            "[doc!] breaking docs update",
            "[breaking] major breaking change",
        ]

        invalid_messages = [
            "invalid commit message without prefix",
            "random: arbitrary tag not in conventional commits",
            "just fixing a typo",
        ]

        with tempfile.TemporaryDirectory() as tmpdir:
            msg_file = Path(tmpdir) / "COMMIT_EDITMSG"

            for msg in valid_messages:
                with self.subTest(msg=msg):
                    msg_file.write_text(f"{msg}\n", encoding="utf-8")
                    proc = subprocess.run(
                        [str(hook_path), str(msg_file)],
                        capture_output=True,
                        text=True,
                    )
                    self.assertEqual(
                        proc.returncode,
                        0,
                        f"Hook should accept '{msg}'. Stderr: {proc.stderr}",
                    )

            for msg in invalid_messages:
                with self.subTest(msg=msg):
                    msg_file.write_text(f"{msg}\n", encoding="utf-8")
                    proc = subprocess.run(
                        [str(hook_path), str(msg_file)],
                        capture_output=True,
                        text=True,
                    )
                    self.assertNotEqual(
                        proc.returncode,
                        0,
                        f"Hook should reject invalid commit '{msg}'",
                    )


if __name__ == "__main__":
    unittest.main()
