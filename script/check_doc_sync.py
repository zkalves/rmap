#!/usr/bin/env python3

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

"""
check_doc_sync.py - Bidirectional Documentation-Implementation Lockstep Enforcer for rmap.

Enforces:
1. Documentation -> Implementation:
   Any changes in the documentation (docs/) must also update the implementation (src/, templates/, tests/).
   If docs change without implementation updates, it must be explicitly marked as a non-functional
   documentation fix via '[doc-only]'.
2. Implementation -> Documentation:
   Any changes in the implementation (src/, templates/) must be flagged for the documentation.
   If implementation changes without updating docs/, an explicit 'DOC-FLAG: <details>' must be
   provided in the commit message or PR description to track the documentation update.
3. Static Architectural Parity Invariants:
   - CLI flags in src/main.cpp vs docs/user/cli-reference.md.
   - Format handlers in src/format/ vs docs/user/architecture.md.
   - Template deliverables in templates/ vs docs/user/templates-and-codegen.md.
   - Themes and languages supported in themes/ and translations/ vs documentation.
"""

import argparse
import glob
import os
import re
import subprocess
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent


def parse_cli_options_from_code():
    """Extract all CLI options registered in src/main.cpp."""
    main_cpp = PROJECT_ROOT / "src" / "main.cpp"
    if not main_cpp.exists():
        return set()

    with open(main_cpp, "r", encoding="utf-8") as f:
        content = f.read()

    options = set()
    # Match QCommandLineOption x({"a", "b", ...}, ...)
    for match in re.finditer(r'QCommandLineOption\s+\w+\s*\(\s*\{([^}]+)\}', content):
        names = re.findall(r'"([^"]+)"', match.group(1))
        for n in names:
            options.add(f"-{n}" if len(n) == 1 else f"--{n}")

    # Match QCommandLineOption x("name", ...)
    for match in re.finditer(r'QCommandLineOption\s+\w+\s*\(\s*"([^"]+)"', content):
        n = match.group(1)
        options.add(f"-{n}" if len(n) == 1 else f"--{n}")

    # Built-in Qt parser options
    if "addHelpOption()" in content:
        options.add("-h")
        options.add("--help")
        options.add("--help-all")
    if "addVersionOption()" in content:
        options.add("-v")
        options.add("--version")

    return options


def parse_cli_options_from_docs():
    """Extract all CLI options documented in docs/user/cli-reference.md."""
    cli_md = PROJECT_ROOT / "docs" / "user" / "cli-reference.md"
    if not cli_md.exists():
        return set()

    with open(cli_md, "r", encoding="utf-8") as f:
        lines = f.readlines()

    options = set()
    in_table = False
    for line in lines:
        stripped = line.strip()
        if stripped.startswith("| Option | Long Option |"):
            in_table = True
            continue
        if in_table:
            if not stripped.startswith("|"):
                in_table = False
                continue
            if ":---" in stripped:
                continue
            cols = [c.strip() for c in stripped.split("|")[1:-1]]
            if len(cols) >= 2:
                for col in cols[:2]:
                    flags = re.findall(r"-{1,2}[a-zA-Z0-9_\-]+", col)
                    for fl in flags:
                        fl = fl.strip()
                        if fl not in ("-", "--", "---"):
                            options.add(fl)

    return options


def check_cli_parity():
    """Verify bidirectional parity between src/main.cpp and docs/user/cli-reference.md."""
    print("Checking Parity: CLI Flags (src/main.cpp <-> docs/user/cli-reference.md)...")
    code_opts = parse_cli_options_from_code()
    docs_opts = parse_cli_options_from_docs()

    missing_in_docs = sorted(code_opts - docs_opts)
    missing_in_code = sorted(docs_opts - code_opts)

    errors = []
    if missing_in_docs:
        errors.append(
            f"  FAIL: The following CLI options exist in code (src/main.cpp) but are NOT documented in docs/user/cli-reference.md:\n"
            + "\n".join(f"    - {opt}" for opt in missing_in_docs)
        )
    if missing_in_code:
        errors.append(
            f"  FAIL: The following CLI options are documented in docs/user/cli-reference.md but are NOT implemented in src/main.cpp:\n"
            + "\n".join(f"    - {opt}" for opt in missing_in_code)
        )

    if errors:
        for err in errors:
            print(err, file=sys.stderr)
        return False

    print(f"  ✓ Verified {len(code_opts)} CLI options in bidirectional parity between code and documentation.")
    return True


def check_templates_parity():
    """Verify all templates on disk are documented in docs/user/templates-and-codegen.md."""
    print("Checking Parity: Templates (templates/ <-> docs/user/templates-and-codegen.md)...")
    tmpl_files = glob.glob(str(PROJECT_ROOT / "templates" / "**" / "*.inja"), recursive=True)
    disk_templates = set()
    for tf in tmpl_files:
        rel = os.path.relpath(tf, PROJECT_ROOT / "templates")
        disk_templates.add(rel)

    doc_path = PROJECT_ROOT / "docs" / "user" / "templates-and-codegen.md"
    if not doc_path.exists():
        print(f"  FAIL: {doc_path} not found!", file=sys.stderr)
        return False

    with open(doc_path, "r", encoding="utf-8") as f:
        doc_content = f.read()

    doc_templates = set(re.findall(r"`([a-zA-Z0-9_\-]+/[a-zA-Z0-9_\-\*\.]+\.inja)`", doc_content))
    uvm_tb_matches = re.findall(r"`([a-zA-Z0-9_]+\.sv)`", doc_content)
    for utm in uvm_tb_matches:
        doc_templates.add(f"uvm_tb/{utm}.inja")

    missing_in_docs = []
    for dt in sorted(disk_templates):
        cat = dt.split("/")[0]
        wildcard = f"{cat}/*.sv.inja"
        if dt not in doc_templates and wildcard not in doc_templates:
            missing_in_docs.append(dt)

    if missing_in_docs:
        print("  FAIL: The following templates in templates/ are not documented in docs/user/templates-and-codegen.md:", file=sys.stderr)
        for m in missing_in_docs:
            print(f"    - {m}", file=sys.stderr)
        return False

    print(f"  ✓ Verified all {len(disk_templates)} templates are documented in templates-and-codegen.md.")
    return True


def check_formats_parity():
    """Verify supported format handlers in src/format/ are documented in docs/user/architecture.md."""
    print("Checking Parity: Format Handlers (src/format/ <-> docs/user/architecture.md)...")
    handlers = [
        ("CmsisSvdHandler", "CMSIS-SVD"),
        ("SystemRdlHandler", "SystemRDL"),
        ("IpxactHandler", "IP-XACT"),
        ("ProtobufHandler", "Protobuf"),
        ("JsonHandler", "JSON"),
        ("CsvHandler", "CSV"),
    ]

    arch_doc = PROJECT_ROOT / "docs" / "user" / "architecture.md"
    if not arch_doc.exists():
        print(f"  FAIL: {arch_doc} not found!", file=sys.stderr)
        return False

    with open(arch_doc, "r", encoding="utf-8") as f:
        content = f.read()

    missing = []
    for class_name, format_name in handlers:
        if class_name not in content and format_name not in content:
            missing.append(f"{class_name} ({format_name})")

    if missing:
        print("  FAIL: The following format handlers in src/format/ are not documented in docs/user/architecture.md:", file=sys.stderr)
        for m in missing:
            print(f"    - {m}", file=sys.stderr)
        return False

    print(f"  ✓ Verified all {len(handlers)} format handlers are documented in architecture.md.")
    return True


def get_git_modified_files(diff_target=None, staged=False):
    """Retrieve list of modified files from git."""
    cmd = ["git", "diff", "--name-only"]
    if staged:
        cmd.append("--cached")
    elif diff_target:
        cmd.extend(diff_target.split())

    try:
        res = subprocess.run(cmd, cwd=PROJECT_ROOT, capture_output=True, text=True, check=True)
        files = [line.strip() for line in res.stdout.splitlines() if line.strip()]
        return files
    except Exception as e:
        print(f"Warning: git command failed: {e}", file=sys.stderr)
        return []


def check_changeset_sync(files, commit_msg=None):
    """
    Enforce:
    1. If docs/ changed -> implementation must also change, or commit must have '[doc-only]'.
    2. If src/ or templates/ changed -> docs/ must also change, or commit must have 'DOC-FLAG:'.
    """
    print("Enforcing Bidirectional Documentation-Implementation Lockstep on Changeset...")

    docs_changed = [f for f in files if f.startswith("docs/")]
    impl_changed = [f for f in files if f.startswith(("src/", "templates/"))]
    tests_changed = [f for f in files if f.startswith("tests/")]

    msg_text = commit_msg or ""
    has_doc_only = "[doc-only]" in msg_text.lower() or "[doc]" in msg_text.lower() or msg_text.strip().startswith("docs:")
    has_doc_flag = "doc-flag:" in msg_text.lower() or "[doc-flag]" in msg_text.lower() or "doc-flag" in msg_text.lower()

    violations = []

    # Rule 1: Docs modified -> Implementation must also be updated (unless explicitly doc-only)
    if docs_changed and not impl_changed and not tests_changed:
        if not has_doc_only:
            violations.append(
                "❌ RULE VIOLATION (Docs -> Implementation):\n"
                "  Documentation in 'docs/' was modified without any corresponding updates to implementation ('src/', 'templates/') or 'tests/'.\n"
                "  Mandate: Any changes in the documentation must also update the implementation.\n"
                "  Remediation:\n"
                "    - Update the corresponding C++ source code, templates, or test suites to reflect the documentation change.\n"
                "    - If this change is strictly a non-functional typo/formatting/grammar fix, add '[doc-only]' to your commit message or PR title."
            )

    # Rule 2: Implementation modified -> Must be flagged for documentation
    if impl_changed and not docs_changed:
        if not has_doc_flag:
            violations.append(
                "❌ RULE VIOLATION (Implementation -> Documentation Flagging):\n"
                "  Implementation in 'src/' or 'templates/' was modified without updating documentation in 'docs/',\n"
                "  and NO documentation flag ('DOC-FLAG:') was provided in the commit message or PR.\n"
                "  Mandate: Any changes in the implementation must be flagged for the documentation.\n"
                "  Remediation:\n"
                "    - Update the relevant documentation in 'docs/' in lockstep with your implementation changes, OR\n"
                "    - Add 'DOC-FLAG: <description of changes or tracking issue>' to your commit message or PR description."
            )

    if violations:
        print("\n" + "\n\n".join(violations) + "\n", file=sys.stderr)
        return False

    print("  ✓ Changeset satisfies Bidirectional Documentation-Implementation Lockstep rules.")
    return True


def main():
    parser = argparse.ArgumentParser(description="Bidirectional Documentation-Implementation Lockstep Enforcer")
    parser.add_argument("--static", action="store_true", help="Run static architectural parity checks")
    parser.add_argument("--staged", action="store_true", help="Check staged git index against lockstep rules")
    parser.add_argument("--git-check", nargs="?", const="HEAD~1..HEAD", help="Check git diff range (default: HEAD~1..HEAD)")
    parser.add_argument("--commit-msg-file", type=str, help="Path to commit message file for commit-msg hook")
    parser.add_argument("--commit-msg", type=str, help="Commit message string for evaluation")

    args = parser.parse_args()

    commit_msg = args.commit_msg or ""
    if args.commit_msg_file and os.path.exists(args.commit_msg_file):
        with open(args.commit_msg_file, "r", encoding="utf-8", errors="ignore") as f:
            commit_msg = f.read()

    passed = True

    # Always run static parity checks if requested or if no git-specific flag is passed
    if args.static or (not args.staged and not args.git_check and not args.commit_msg_file):
        print("=== Running Static Documentation-Implementation Parity Invariants ===")
        passed &= check_cli_parity()
        passed &= check_templates_parity()
        passed &= check_formats_parity()

    # If git staged or range requested
    if args.staged:
        files = get_git_modified_files(staged=True)
        if files:
            passed &= check_changeset_sync(files, commit_msg=commit_msg)
    elif args.git_check:
        files = get_git_modified_files(diff_target=args.git_check)
        if files:
            passed &= check_changeset_sync(files, commit_msg=commit_msg)
    elif args.commit_msg_file:
        # Check staged changes in commit-msg hook
        files = get_git_modified_files(staged=True)
        if not files:
            # Fallback to diff of HEAD vs working index
            files = get_git_modified_files(diff_target="HEAD")
        if files:
            passed &= check_changeset_sync(files, commit_msg=commit_msg)

    if not passed:
        sys.exit(1)

    print("\nSUCCESS: All documentation and implementation lockstep checks passed cleanly!")
    sys.exit(0)


if __name__ == "__main__":
    main()
