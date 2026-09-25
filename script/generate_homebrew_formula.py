#!/usr/bin/env python3

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

"""
generate_homebrew_formula.py - Generate / Update Formula/rmap.rb
"""

import argparse
import hashlib
import os
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent
FORMULA_PATH = PROJECT_ROOT / "Formula" / "rmap.rb"
VERSION_PATH = PROJECT_ROOT / "VERSION"


def calculate_sha256(filepath: Path) -> str:
    h = hashlib.sha256()
    with open(filepath, "rb") as f:
        while chunk := f.read(65536):
            h.update(chunk)
    return h.hexdigest()


def generate_formula(version: str, sha256: str = "") -> str:
    sha_line = f'  sha256 "{sha256}"' if sha256 else '  # sha256 "..."'
    return f"""# typed: false
# frozen_string_literal: true

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

class Rmap < Formula
  desc "Hardware Register Map Designer & Model Generator"
  homepage "https://github.com/zkalves/rmap"
  url "https://github.com/zkalves/rmap/archive/refs/tags/v{version}.tar.gz"
{sha_line}
  license "MPL-2.0"
  head "https://github.com/zkalves/rmap.git", branch: "main"

  depends_on "cmake" => :build
  depends_on "protobuf"
  depends_on "qt@6"

  def install
    qt6 = Formula["qt@6"]

    args = std_cmake_args + %W[
      -DCMAKE_BUILD_TYPE=Release
      -DBUILD_TESTING=OFF
      -DCMAKE_PREFIX_PATH=#{{qt6.opt_prefix}}
      -DQT_DIR=#{{qt6.opt_lib}}/cmake/Qt6
    ]

    system "cmake", "-S", ".", "-B", "build", *args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    ENV["QT_QPA_PLATFORM"] = "offscreen"
    assert_match "rmap", shell_output("#{{bin}}/rmap --version")
  end
end
"""


def main():
    parser = argparse.ArgumentParser(description="Generate Homebrew Formula for rmap.")
    parser.add_argument("--version", default="", help="Release version (defaults to VERSION file)")
    parser.add_argument("--tarball", default="", help="Path to release tarball to compute SHA256")
    parser.add_argument("--sha256", default="", help="Explicit SHA256 hash")
    parser.add_argument("--check", action="store_true", help="Check formula syntax using ruby -c")
    args = parser.parse_args()

    version = args.version or VERSION_PATH.read_text(encoding="utf-8").strip()

    sha256 = args.sha256
    if args.tarball and not sha256:
        tarball_path = Path(args.tarball)
        if tarball_path.exists():
            sha256 = calculate_sha256(tarball_path)

    formula_content = generate_formula(version, sha256)
    FORMULA_PATH.parent.mkdir(parents=True, exist_ok=True)
    FORMULA_PATH.write_text(formula_content, encoding="utf-8")
    print(f"Generated {FORMULA_PATH} for version v{version}")

    if args.check:
        import subprocess
        res = subprocess.run(["ruby", "-c", str(FORMULA_PATH)], capture_output=True, text=True)
        if res.returncode != 0:
            print(f"Ruby syntax error in formula:\n{res.stderr}", file=sys.stderr)
            sys.exit(1)
        print("Ruby syntax check passed cleanly.")


if __name__ == "__main__":
    main()
