#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

import sys
import os

# Add work/python to import path
sys.path.insert(0, os.path.abspath("work/python"))
from reg_map import SPI_TopBlock

def main():
    print("[Python Harness] Testing SPI Register Driver...")
    # Simulated register memory bank
    mem = {0x1000: 0, 0x1004: 0}

    def sim_read(addr):
        return mem.get(addr, 0)

    def sim_write(addr, val):
        mem[addr] = val

    block = SPI_TopBlock(base_addr=0x1000, read_fn=sim_read, write_fn=sim_write)
    assert block.ctrl.offset == 0x0000
    assert block.status.offset == 0x0004

    # Test field write and read
    block.write_field(block.ctrl, "EN", 1)
    assert block.read_field(block.ctrl, "EN") == 1
    assert mem[0x1000] == 1

    block.write_field(block.ctrl, "MODE", 2)
    assert block.read_field(block.ctrl, "MODE") == 2
    assert mem[0x1000] == 0x5

    print("[Python Harness] All SPI driver tests PASSED!")

if __name__ == "__main__":
    main()
