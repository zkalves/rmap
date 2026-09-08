#!/usr/bin/env python3

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

"""
bump_version.py - Semantic Versioning Automation Script for rmap

Usage:
  python3 script/bump_version.py --current
  python3 script/bump_version.py --patch [--tag]
  python3 script/bump_version.py --minor [--tag]
  python3 script/bump_version.py --major [--tag]
  python3 script/bump_version.py --auto  [--tag]
"""

import sys
import os
import re
import json
import argparse
import subprocess
from pathlib import Path
from typing import Optional

PROJECT_ROOT = Path(__file__).resolve().parent.parent
VERSION_FILE = PROJECT_ROOT / "VERSION"
MANIFEST_FILE = PROJECT_ROOT / ".release-please-manifest.json"


def get_current_version() -> str:
    if not VERSION_FILE.exists():
        return "0.2.0"
    return VERSION_FILE.read_text(encoding="utf-8").strip()


def parse_semver(ver_str: str):
    m = re.match(r"^(\d+)\.(\d+)\.(\d+)(?:-([0-9A-Za-z.-]+))?(?:\+([0-9A-Za-z.-]+))?$", ver_str)
    if not m:
        raise ValueError(f"Invalid SemVer string: {ver_str}")
    return int(m.group(1)), int(m.group(2)), int(m.group(3))


def bump_version(ver_str: str, bump_type: str) -> str:
    major, minor, patch = parse_semver(ver_str)
    if bump_type == "major":
        return f"{major + 1}.0.0"
    elif bump_type == "minor":
        return f"{major}.{minor + 1}.0"
    elif bump_type == "patch":
        return f"{major}.{minor}.{patch + 1}"
    else:
        raise ValueError(f"Unknown bump type: {bump_type}")


def parse_commit_bump(msg: str) -> Optional[str]:
    """Parse a single commit message to determine if it requires a major, minor, or patch bump."""
    msg = msg.strip()
    if not msg:
        return None
    first_line = msg.splitlines()[0].strip()

    # 1. Breaking change checks -> MAJOR
    # - Conventional Commit breaking markers: feat!: or feat(scope)!:, fix!:, doc!:, etc.
    # - BREAKING CHANGE: or BREAKING-CHANGE: in subject or body
    # - Bracketed style: [breaking], [feat!], [fix!], etc.
    if "BREAKING CHANGE:" in msg or "BREAKING-CHANGE:" in msg:
        return "major"
    if re.search(r"^(feat|feature|fix|bugfix|doc|docs|style|refactor|perf|test|tests|build|ci|chore|revert)(\([^)]+\))?!:", first_line, re.IGNORECASE):
        return "major"
    if re.search(r"^\[(breaking|feat!|fix!|feature!|bugfix!|doc!|docs!)\]", first_line, re.IGNORECASE):
        return "major"

    # 2. Feature checks -> MINOR
    if re.search(r"^(feat|feature)(\([^)]+\))?:", first_line, re.IGNORECASE):
        return "minor"
    if re.search(r"^\[(feat|feature)\]", first_line, re.IGNORECASE):
        return "minor"

    # 3. Patch checks (fix, doc, docs, style, refactor, perf, test, build, ci, chore, revert) -> PATCH
    if re.search(r"^(fix|bugfix|doc|docs|style|refactor|perf|test|tests|build|ci|chore|revert)(\([^)]+\))?:", first_line, re.IGNORECASE):
        return "patch"
    if re.search(r"^\[(fix|bugfix|doc|docs|style|refactor|perf|test|tests|build|ci|chore|revert)\]", first_line, re.IGNORECASE):
        return "patch"

    return None


def detect_auto_bump() -> str:
    """Analyze git commits since last release tag to determine appropriate SemVer bump."""
    # Find latest tag
    try:
        tag_proc = subprocess.run(
            ["git", "describe", "--tags", "--match", "v[0-9]*", "--abbrev=0"],
            cwd=PROJECT_ROOT,
            capture_output=True,
            text=True,
            check=False,
        )
        if tag_proc.returncode == 0 and tag_proc.stdout.strip():
            latest_tag = tag_proc.stdout.strip()
            rev_range = f"{latest_tag}..HEAD"
        else:
            rev_range = "HEAD~20..HEAD"
    except Exception:
        rev_range = "HEAD"

    try:
        log_proc = subprocess.run(
            ["git", "log", rev_range, "--pretty=format:%B---COMMIT_SEP---"],
            cwd=PROJECT_ROOT,
            capture_output=True,
            text=True,
            check=False,
        )
        messages = log_proc.stdout.split("---COMMIT_SEP---")
    except Exception:
        messages = []

    has_breaking = False
    has_feat = False
    has_patch = False

    for msg in messages:
        bump = parse_commit_bump(msg)
        if bump == "major":
            has_breaking = True
        elif bump == "minor":
            has_feat = True
        elif bump == "patch":
            has_patch = True

    if has_breaking:
        return "major"
    elif has_feat:
        return "minor"
    elif has_patch:
        return "patch"
    else:
        return "patch"


def update_version_files(new_ver: str):
    VERSION_FILE.write_text(f"{new_ver}\n", encoding="utf-8")
    print(f"Updated {VERSION_FILE} -> {new_ver}")

    if MANIFEST_FILE.exists():
        try:
            data = json.loads(MANIFEST_FILE.read_text(encoding="utf-8"))
            data["."] = new_ver
            MANIFEST_FILE.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
            print(f"Updated {MANIFEST_FILE} -> {new_ver}")
        except Exception as e:
            print(f"Warning: Failed to update {MANIFEST_FILE}: {e}")


def git_commit_and_tag(new_ver: str):
    tag_name = f"v{new_ver}"
    print(f"Creating Git commit and tag {tag_name}...")
    files_to_add = ["VERSION"]
    if MANIFEST_FILE.exists():
        files_to_add.append(".release-please-manifest.json")

    subprocess.run(["git", "add"] + files_to_add, cwd=PROJECT_ROOT, check=True)
    subprocess.run(
        ["git", "commit", "-m", f"chore(release): {tag_name}"],
        cwd=PROJECT_ROOT,
        check=True,
    )
    subprocess.run(
        ["git", "tag", "-a", tag_name, "-m", f"Release {tag_name}"],
        cwd=PROJECT_ROOT,
        check=True,
    )
    print(f"Successfully committed and tagged {tag_name}!")


def main():
    parser = argparse.ArgumentParser(description="Automated semantic versioning tool for rmap.")
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--current", "-c", action="store_true", help="Display current version")
    group.add_argument("--patch", action="store_true", help="Bump patch version (e.g. 0.2.0 -> 0.2.1)")
    group.add_argument("--minor", action="store_true", help="Bump minor version (e.g. 0.2.0 -> 0.3.0)")
    group.add_argument("--major", action="store_true", help="Bump major version (e.g. 0.2.0 -> 1.0.0)")
    group.add_argument("--auto", action="store_true", help="Automatically determine bump from commits")

    parser.add_argument("--tag", "-t", action="store_true", help="Commit and create git tag vX.Y.Z")
    parser.add_argument("--dry-run", action="store_true", help="Show proposed changes without writing")

    args = parser.parse_args()

    curr_ver = get_current_version()

    if args.current:
        print(f"Current version in VERSION: {curr_ver}")
        try:
            desc = subprocess.run(
                ["git", "describe", "--tags", "--always", "--dirty"],
                cwd=PROJECT_ROOT,
                capture_output=True,
                text=True,
                check=False,
            ).stdout.strip()
            print(f"Git describe: {desc}")
        except Exception:
            pass
        return 0

    bump_type = None
    if args.patch:
        bump_type = "patch"
    elif args.minor:
        bump_type = "minor"
    elif args.major:
        bump_type = "major"
    elif args.auto:
        bump_type = detect_auto_bump()
        print(f"Auto-detected bump type: {bump_type}")

    new_ver = bump_version(curr_ver, bump_type)
    print(f"Bumping version: {curr_ver} -> {new_ver} ({bump_type})")

    if args.dry_run:
        print("Dry run enabled. No files modified.")
        return 0

    update_version_files(new_ver)

    if args.tag:
        git_commit_and_tag(new_ver)

    return 0


if __name__ == "__main__":
    sys.exit(main())
