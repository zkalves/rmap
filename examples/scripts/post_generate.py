#!/usr/bin/env python3

# This file is part of rmap.
#
# rmap is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# rmap is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with rmap.  If not, see <https://www.gnu.org/licenses/>.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

"""
Post-generation hook script for rmap examples.
Demonstrates automated inspection of generated register map artifacts.
"""

import sys
import json
import os

def main():
    print("[post_generate.py] Executing post-generation hook...")

    # Inspect command line arguments or stdin for JSON metadata if passed
    json_path = None
    for arg in sys.argv[1:]:
        if arg.endswith(".json") and os.path.isfile(arg):
            json_path = arg
            break

    if json_path:
        with open(json_path, "r", encoding="utf-8") as f:
            data = json.load(f)
        proj_name = data.get("project_name", "Unknown")
        proj_ver = data.get("project_version", "Unknown")
        print(f"[post_generate.py] Verified project metadata: {proj_name} v{proj_ver}")

    print("[post_generate.py] Post-generation hook completed successfully.")
    sys.exit(0)

if __name__ == "__main__":
    main()
