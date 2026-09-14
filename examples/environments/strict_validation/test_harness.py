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
    print("[Python Harness] Testing strict_validation Register Driver...")
    mem = {}
    def sim_read(addr):
        return mem.get(addr, 0)
    def sim_write(addr, val):
        mem[addr] = val

    block = reg_map.STRICT_BLOCKBlock(base_addr=0x0, read_fn=sim_read, write_fn=sim_write)
    reg = getattr(block, 'control')
    assert reg is not None
    print(f'Tested register {reg.name} at offset 0x{reg.offset:X}')
    if 'START' in reg.fields:
        block.write_field(reg, 'START', 1)
        assert block.read_field(reg, 'START') == 1
        print('Tested field START read/write')
    print("[Python Harness] All strict_validation driver tests PASSED!")

if __name__ == "__main__":
    main()
