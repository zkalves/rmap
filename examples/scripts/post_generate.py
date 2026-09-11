#!/usr/bin/env python3

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
