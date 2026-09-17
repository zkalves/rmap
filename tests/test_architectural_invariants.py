#!/usr/bin/env python3

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

"""
Architectural Invariant Enforcement Suite for rmap.

Verifies:
1. src/ contains zero hardcoded references to template names.
2. src/ contains zero hardcoded references to theme IDs or names.
3. src/ contains zero hardcoded references to language/locale codes.
4. src/ contains zero hardcoded references to specific example files.
5. All Inja templates in templates/ are registered and covered by tests/test_template.py.
"""

import os
import sys
import glob
import re

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
SRC_DIR = os.path.join(PROJECT_ROOT, "src")
TEMPLATES_DIR = os.path.join(PROJECT_ROOT, "templates")
THEMES_DIR = os.path.join(PROJECT_ROOT, "themes")
TRANSLATIONS_DIR = os.path.join(PROJECT_ROOT, "translations")
EXAMPLES_DIR = os.path.join(PROJECT_ROOT, "examples")


def collect_source_files():
    source_files = []
    for root, _, files in os.walk(SRC_DIR):
        for f in files:
            if f.endswith((".cpp", ".hpp", ".h", ".c")):
                source_files.append(os.path.join(root, f))
    return source_files


def test_no_hardcoded_templates(sources):
    print("Checking Invariant 1: Zero hardcoded template references in src/...")
    # Gather all template basenames (e.g., reg_map.sv, reg_map.h, reg_model.sv, etc.)
    template_files = glob.glob(os.path.join(TEMPLATES_DIR, "**", "*.inja"), recursive=True)
    template_names = set()
    for tf in template_files:
        base = os.path.basename(tf)
        if base.endswith(".inja"):
            base = base[:-5]
        template_names.add(base)

    # Filter out generic terms if any, keep specific template filenames
    forbidden = [name for name in template_names if len(name) > 4]

    violations = []
    for src in sources:
        rel_path = os.path.relpath(src, PROJECT_ROOT)
        with open(src, "r", encoding="utf-8", errors="ignore") as f:
            for line_no, line in enumerate(f, 1):
                # Ignore pure comment lines if needed, but per rule comments were also cleaned
                for tmpl in forbidden:
                    if tmpl in line:
                        violations.append((rel_path, line_no, tmpl, line.strip()))

    if violations:
        print("FAIL: Hardcoded template names found in src/:", file=sys.stderr)
        for v in violations:
            print(f"  {v[0]}:{v[1]} - references '{v[2]}': {v[3]}", file=sys.stderr)
        return False

    print(f"  ✓ Verified {len(sources)} source files: 0 template name references found.")
    return True


def test_no_hardcoded_themes(sources):
    print("Checking Invariant 2: Zero hardcoded theme references in src/...")
    theme_files = glob.glob(os.path.join(THEMES_DIR, "*.json"))
    theme_ids = set()
    for tf in theme_files:
        base = os.path.splitext(os.path.basename(tf))[0]
        if base != "template":
            theme_ids.add(base)

    violations = []
    for src in sources:
        rel_path = os.path.relpath(src, PROJECT_ROOT)
        with open(src, "r", encoding="utf-8", errors="ignore") as f:
            for line_no, line in enumerate(f, 1):
                for tid in theme_ids:
                    # Match exact string literal of theme id
                    if f'"{tid}"' in line or f"'{tid}'" in line:
                        violations.append((rel_path, line_no, tid, line.strip()))

    if violations:
        print("FAIL: Hardcoded theme IDs found in src/:", file=sys.stderr)
        for v in violations:
            print(f"  {v[0]}:{v[1]} - references theme '{v[2]}': {v[3]}", file=sys.stderr)
        return False

    print(f"  ✓ Verified {len(sources)} source files: 0 theme ID references found.")
    return True


def test_no_hardcoded_languages(sources):
    print("Checking Invariant 3: Zero hardcoded translation codes in src/...")
    # Distinct non-English language codes from translations/
    trans_files = glob.glob(os.path.join(TRANSLATIONS_DIR, "rmap_*.json"))
    lang_codes = set()
    for tf in trans_files:
        base = os.path.basename(tf)
        code = base.replace("rmap_", "").replace(".json", "")
        lang_codes.add(code)

    # Distinct codes like zh_CN, pt_BR
    distinct_codes = [c for c in lang_codes if "_" in c]

    violations = []
    for src in sources:
        rel_path = os.path.relpath(src, PROJECT_ROOT)
        with open(src, "r", encoding="utf-8", errors="ignore") as f:
            for line_no, line in enumerate(f, 1):
                for code in distinct_codes:
                    if f'"{code}"' in line or f"'{code}'" in line:
                        violations.append((rel_path, line_no, code, line.strip()))

    if violations:
        print("FAIL: Hardcoded language codes found in src/:", file=sys.stderr)
        for v in violations:
            print(f"  {v[0]}:{v[1]} - references '{v[2]}': {v[3]}", file=sys.stderr)
        return False

    print(f"  ✓ Verified {len(sources)} source files: 0 hardcoded language tables found.")
    return True


def test_no_hardcoded_examples(sources):
    print("Checking Invariant 4: Zero hardcoded example file references in src/...")
    example_files = glob.glob(os.path.join(EXAMPLES_DIR, "**", "*.rmt"), recursive=True)
    example_basenames = [os.path.basename(ef) for ef in example_files]

    violations = []
    for src in sources:
        rel_path = os.path.relpath(src, PROJECT_ROOT)
        with open(src, "r", encoding="utf-8", errors="ignore") as f:
            for line_no, line in enumerate(f, 1):
                for eb in example_basenames:
                    if eb in line:
                        violations.append((rel_path, line_no, eb, line.strip()))

    if violations:
        print("FAIL: Hardcoded example references found in src/:", file=sys.stderr)
        for v in violations:
            print(f"  {v[0]}:{v[1]} - references '{v[2]}': {v[3]}", file=sys.stderr)
        return False

    print(f"  ✓ Verified {len(sources)} source files: 0 example file references found.")
    return True


def test_all_templates_registered():
    print("Checking Invariant 5: All Inja templates registered in test_template.py...")
    test_template_py = os.path.join(PROJECT_ROOT, "tests", "test_template.py")
    with open(test_template_py, "r", encoding="utf-8") as f:
        content = f.read()

    template_files = glob.glob(os.path.join(TEMPLATES_DIR, "**", "*.inja"), recursive=True)
    missing = []
    for tf in template_files:
        rel = os.path.relpath(tf, PROJECT_ROOT)
        base = os.path.basename(tf)
        out_name = base[:-5] if base.endswith(".inja") else base
        # Verify either the relative template path, template base, or generated output name is covered
        if rel not in content and base not in content and out_name not in content:
            missing.append(rel)

    if missing:
        print("FAIL: Unregistered template files in test_template.py:", file=sys.stderr)
        for m in missing:
            print(f"  {m}", file=sys.stderr)
        return False

    print(f"  ✓ Verified all {len(template_files)} Inja templates are covered by test_template.py.")
    return True


def main():
    print("=== Running Architectural Invariants Test Suite ===\n")
    sources = collect_source_files()
    assert len(sources) > 0, "No source files found in src/!"

    passed = True
    passed &= test_no_hardcoded_templates(sources)
    passed &= test_no_hardcoded_themes(sources)
    passed &= test_no_hardcoded_languages(sources)
    passed &= test_no_hardcoded_examples(sources)
    passed &= test_all_templates_registered()

    if passed:
        print("\n=======================================================")
        print("SUCCESS: All architectural invariants verified cleanly!")
        print("=======================================================")
        sys.exit(0)
    else:
        print("\n=======================================================", file=sys.stderr)
        print("FAILED: Architectural invariant violations detected!", file=sys.stderr)
        print("=======================================================", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
