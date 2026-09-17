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
    print("[Python Harness] Testing hw_precedence Register Driver...")
    mem = {}
    def sim_read(addr):
        return mem.get(addr, 0)
    def sim_write(addr, val):
        mem[addr] = val

    block = reg_map.ARBITRATION_BLOCKBlock(base_addr=0x0, read_fn=sim_read, write_fn=sim_write)
    flags_reg = getattr(block, 'status_flags')
    assert flags_reg is not None
    assert flags_reg.offset == 0x0
    print(f'Tested register {flags_reg.name} at offset 0x{flags_reg.offset:X}')

    ctrl_reg = getattr(block, 'control')
    assert ctrl_reg is not None
    assert ctrl_reg.offset == 0x4
    print(f'Tested register {ctrl_reg.name} at offset 0x{ctrl_reg.offset:X}')

    block.write_field(ctrl_reg, 'ENABLE', 1)
    assert block.read_field(ctrl_reg, 'ENABLE') == 1
    print('Tested field ENABLE read/write')
    print("[Python Harness] All hw_precedence driver tests PASSED!")

if __name__ == "__main__":
    main()
