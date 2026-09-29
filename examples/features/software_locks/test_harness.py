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
    print("[Python Harness] Testing software_locks Register Driver...")
    mem = {}
    def sim_read(addr):
        return mem.get(addr, 0)
    def sim_write(addr, val):
        mem[addr] = val

    block = reg_map.SECURITY_COREBlock(base_addr=0x0, read_fn=sim_read, write_fn=sim_write)

    # 1. SEC_CTRL
    ctrl = getattr(block, 'sec_ctrl')
    assert ctrl is not None
    assert ctrl.offset == 0x0
    block.write_field(ctrl, 'LOCK_BIT', 1)
    assert block.read_field(ctrl, 'LOCK_BIT') == 1

    # 2. SEC_KEY
    key = getattr(block, 'sec_key')
    assert key is not None
    assert key.offset == 0x4
    block.write_field(key, 'KEY_PAYLOAD', 0x12345678)
    assert block.read_field(key, 'KEY_PAYLOAD') == 0x12345678

    # 3. CRYPTO_CFG
    crypto = getattr(block, 'crypto_cfg')
    assert crypto is not None
    assert crypto.offset == 0x8
    block.write_field(crypto, 'ALGO_SELECT', 0x0002)
    assert block.read_field(crypto, 'ALGO_SELECT') == 0x0002

    # 4. MULTI_FIELD_REG
    multi = getattr(block, 'multi_field_reg')
    assert multi is not None
    assert multi.offset == 0xC
    block.write_field(multi, 'PUBLIC_STATUS', 0x55)
    block.write_field(multi, 'KEY_SLICE', 0xAA)
    block.write_field(multi, 'PASSCODE', 0x33)
    block.write_field(multi, 'SECURE_CTRL', 0xCC)

    assert block.read_field(multi, 'PUBLIC_STATUS') == 0x55
    assert block.read_field(multi, 'KEY_SLICE') == 0xAA
    assert block.read_field(multi, 'PASSCODE') == 0x33
    assert block.read_field(multi, 'SECURE_CTRL') == 0xCC

    print("[Python Harness] All software_locks driver tests PASSED!")

if __name__ == "__main__":
    main()
