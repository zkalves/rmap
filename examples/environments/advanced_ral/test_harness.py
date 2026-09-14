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
    print("[Python Harness] Testing advanced_ral Register Driver...")
    mem = {}
    def sim_read(addr):
        return mem.get(addr, 0)
    def sim_write(addr, val):
        mem[addr] = val

    block = reg_map.RAL_ADVANCED_BLOCKBlock(base_addr=0x0, read_fn=sim_read, write_fn=sim_write)

    # 1. Indirect Addressing Registers
    cfg_index = getattr(block, 'cfg_index', None)
    assert cfg_index is not None, "cfg_index register missing"
    print(f"Tested indirect index register {cfg_index.name} at offset 0x{cfg_index.offset:X}")

    cfg_data = getattr(block, 'cfg_data', None)
    assert cfg_data is not None, "cfg_data register missing"
    print(f"Tested indirect data register {cfg_data.name} at offset 0x{cfg_data.offset:X}")

    # Set index to 0x0042
    block.write_field(cfg_index, 'INDEX', 0x0042)
    assert block.read_field(cfg_index, 'INDEX') == 0x0042
    print("Verified indirect index field access")

    # 2. Hardware FIFO Register
    fifo_port = getattr(block, 'fifo_port', None)
    assert fifo_port is not None, "fifo_port register missing"
    print(f"Tested FIFO port register {fifo_port.name} at offset 0x{fifo_port.offset:X}")

    # 3. Status with Callbacks Register
    status_cbs = getattr(block, 'status_cbs', None)
    assert status_cbs is not None, "status_cbs register missing"
    print(f"Tested status register {status_cbs.name} at offset 0x{status_cbs.offset:X}")

    # 4. Interrupt Flags with Test Suppression
    intr_flags = getattr(block, 'intr_flags', None)
    assert intr_flags is not None, "intr_flags register missing"
    print(f"Tested interrupt flags register {intr_flags.name} at offset 0x{intr_flags.offset:X}")

    print("[Python Harness] All advanced_ral driver tests PASSED!")

if __name__ == "__main__":
    main()
