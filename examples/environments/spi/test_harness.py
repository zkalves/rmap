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
