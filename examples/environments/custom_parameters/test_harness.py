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

sys.path.insert(0, os.path.abspath("work/python"))
import reg_map as reg_map

def main():
    print("[Python Harness] Testing custom_parameters Register Driver...")
    mem = {}
    def sim_read(addr):
        return mem.get(addr, 0)
    def sim_write(addr, val):
        mem[addr] = val

    block = reg_map.PARAMETRIC_COREBlock(base_addr=0x0, read_fn=sim_read, write_fn=sim_write)
    reg = getattr(block, 'config_reg')
    assert reg is not None
    print(f'Tested register {reg.name} at offset 0x{reg.offset:X}')
    if 'CLK_RATIO' in reg.fields:
        block.write_field(reg, 'CLK_RATIO', 1)
        assert block.read_field(reg, 'CLK_RATIO') == 1
        print('Tested field CLK_RATIO read/write')
    print("[Python Harness] All custom_parameters driver tests PASSED!")

if __name__ == "__main__":
    main()
