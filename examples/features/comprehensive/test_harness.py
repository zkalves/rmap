#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

import sys
import os

sys.path.insert(0, os.path.abspath("work/python"))
import reg_map as reg_map

def main():
    print("[Python Harness] Testing comprehensive Register Driver...")
    mem = {}
    def sim_read(addr):
        return mem.get(addr, 0)
    def sim_write(addr, val):
        mem[addr] = val

    block = reg_map.CORE_SUBSYSTEMBlock(base_addr=0x0, read_fn=sim_read, write_fn=sim_write)
    reg = getattr(block, 'control')
    assert reg is not None
    print(f'Tested register {reg.name} at offset 0x{reg.offset:X}')
    if 'ENABLE' in reg.fields:
        block.write_field(reg, 'ENABLE', 1)
        assert block.read_field(reg, 'ENABLE') == 1
        print('Tested field ENABLE read/write')

    once_reg = getattr(block, 'once_and_toggle')
    assert once_reg is not None
    assert once_reg.offset == 0x10
    print(f'Tested register {once_reg.name} at offset 0x{once_reg.offset:X}')

    side_reg = getattr(block, 'write_read_side_effects')
    assert side_reg is not None
    assert side_reg.offset == 0x14
    print(f'Tested register {side_reg.name} at offset 0x{side_reg.offset:X}')

    comb_reg = getattr(block, 'combined_bit_side_effects')
    assert comb_reg is not None
    assert comb_reg.offset == 0x18
    print(f'Tested register {comb_reg.name} at offset 0x{comb_reg.offset:X}')

    only_reg = getattr(block, 'write_only_side_effects')
    assert only_reg is not None
    assert only_reg.offset == 0x1C
    print(f'Tested register {only_reg.name} at offset 0x{only_reg.offset:X}')

    print("[Python Harness] All comprehensive driver tests PASSED!")

if __name__ == "__main__":
    main()
