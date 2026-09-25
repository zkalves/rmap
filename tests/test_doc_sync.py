#!/usr/bin/env python3

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

"""
test_doc_sync.py - Unit Test Suite for Bidirectional Documentation-Implementation Lockstep Enforcer.

Verifies:
1. Static CLI options parity between src/main.cpp and docs/user/cli-reference.md.
2. Static template catalog parity between templates/ and docs/user/templates-and-codegen.md.
3. Static format handlers parity between src/format/ and docs/user/architecture.md.
4. Changeset lockstep enforcement logic:
   - Rejection of docs/ changes without implementation updates or [doc-only] marker.
   - Acceptance of docs/ changes with [doc-only] marker.
   - Acceptance of docs/ changes alongside implementation updates.
   - Rejection of implementation (src/, templates/) changes without docs/ updates or DOC-FLAG.
   - Acceptance of implementation changes with DOC-FLAG.
   - Acceptance of implementation changes with docs/ updates.
5. End-to-end execution of script/check_doc_sync.py via CLI.
"""

import os
import subprocess
import sys
import unittest
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(PROJECT_ROOT / "script"))

from check_doc_sync import (
    check_cli_parity,
    check_formats_parity,
    check_templates_parity,
    check_changeset_sync,
    parse_cli_options_from_code,
    parse_cli_options_from_docs,
)


class TestDocImplementationSync(unittest.TestCase):
    def test_static_cli_parity(self):
        """Verify that CLI flags in src/main.cpp and docs/user/cli-reference.md match 100%."""
        self.assertTrue(check_cli_parity(), "CLI flags parity check failed")

    def test_static_templates_parity(self):
        """Verify that all templates in templates/ are catalogued in docs/."""
        self.assertTrue(check_templates_parity(), "Templates parity check failed")

    def test_static_formats_parity(self):
        """Verify that all format handlers in src/format/ are catalogued in docs/."""
        self.assertTrue(check_formats_parity(), "Format handlers parity check failed")

    def test_docs_changed_without_impl_fails(self):
        """Docs changed without implementation updates or [doc-only] must fail."""
        files = ["docs/user/architecture.md"]
        self.assertFalse(check_changeset_sync(files, commit_msg="feat: update architecture"))

    def test_docs_changed_with_doc_only_passes(self):
        """Docs changed with [doc-only] marker must pass."""
        files = ["docs/user/architecture.md"]
        self.assertTrue(check_changeset_sync(files, commit_msg="docs: fix typos in architecture [doc-only]"))
        self.assertTrue(check_changeset_sync(files, commit_msg="docs: update getting started guide"))
        self.assertTrue(check_changeset_sync(files, commit_msg="doc: update getting started guide"))
        self.assertTrue(check_changeset_sync(files, commit_msg="docs(user): update architecture guide"))
        self.assertTrue(check_changeset_sync(files, commit_msg="doc(gui): clarify bitfield bar usage"))

    def test_docs_and_impl_changed_passes(self):
        """Docs and implementation changed together must pass."""
        files = ["docs/user/cli-reference.md", "src/main.cpp"]
        self.assertTrue(check_changeset_sync(files, commit_msg="feat(cli): add new option"))

    def test_impl_changed_without_docs_or_flag_fails(self):
        """Implementation changed without docs update or DOC-FLAG must fail."""
        files = ["src/main.cpp"]
        self.assertFalse(check_changeset_sync(files, commit_msg="fix(main): adjust startup behavior"))

        files_tmpl = ["templates/rtl/reg_map.sv.inja"]
        self.assertFalse(check_changeset_sync(files_tmpl, commit_msg="refactor(rtl): optimize mux tree"))

    def test_impl_changed_with_doc_flag_passes(self):
        """Implementation changed with DOC-FLAG must pass."""
        files = ["src/main.cpp"]
        msg = "fix(main): internal timing adjustment\n\nDOC-FLAG: Internal refactor, no user-facing CLI changes"
        self.assertTrue(check_changeset_sync(files, commit_msg=msg))

        msg2 = "feat(rtl): experimental strobe logic\n\n[DOC-FLAG] Tracking in issue #42 for next release doc update"
        self.assertTrue(check_changeset_sync(files, commit_msg=msg2))

    def test_non_doc_non_impl_files_pass_unrestricted(self):
        """Changes to tests, scripts, or project meta files do not trigger lockstep rejection."""
        files = ["tests/test_coverage.py", "script/render_docs", ".gitignore"]
        self.assertTrue(check_changeset_sync(files, commit_msg="test: add coverage checks"))

    def test_cli_execution_static(self):
        """Test check_doc_sync.py CLI execution with --static."""
        script_path = PROJECT_ROOT / "script" / "check_doc_sync.py"
        res = subprocess.run(
            [sys.executable, str(script_path), "--static"],
            cwd=str(PROJECT_ROOT),
            capture_output=True,
            text=True,
        )
        self.assertEqual(res.returncode, 0, f"check_doc_sync.py --static failed:\n{res.stderr}\n{res.stdout}")
        self.assertIn("SUCCESS", res.stdout)


if __name__ == "__main__":
    unittest.main()
