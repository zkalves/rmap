#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

import sys
import os

sys.path.insert(0, os.path.abspath("work/python"))
import reg_map

def main():
    print("[Python Harness] Testing soc_large_scale Register Driver across all 4 blocks...")
    mem = {}
    def sim_read(addr):
        return mem.get(addr, 0)
    def sim_write(addr, val):
        mem[addr] = val

    # 1. CPU_SUBSYSTEM
    cpu = reg_map.CPU_SUBSYSTEMBlock(base_addr=0x0, read_fn=sim_read, write_fn=sim_write)
    assert cpu.core_ctrl.offset == 0x00
    cpu.write_field(cpu.core_ctrl, "BOOT_EN", 1)
    assert cpu.read_field(cpu.core_ctrl, "BOOT_EN") == 1
    print("✓ CPU_SUBSYSTEM tested successfully")

    # 2. DMA_CONTROLLER
    dma = reg_map.DMA_CONTROLLERBlock(base_addr=0x4000, read_fn=sim_read, write_fn=sim_write)
    assert dma.dma_global_ctrl.offset == 0x00
    dma.write_field(dma.dma_global_ctrl, "DMA_EN", 1)
    assert dma.read_field(dma.dma_global_ctrl, "DMA_EN") == 1
    print("✓ DMA_CONTROLLER tested successfully")

    # 3. SECURITY_ENGINE
    sec = reg_map.SECURITY_ENGINEBlock(base_addr=0x8000, read_fn=sim_read, write_fn=sim_write)
    assert sec.sec_ctrl.offset == 0x00
    sec.write_field(sec.sec_ctrl, "AES_EN", 1)
    assert sec.read_field(sec.sec_ctrl, "AES_EN") == 1
    print("✓ SECURITY_ENGINE tested successfully")

    # 4. PERIPHERAL_BRIDGE
    bridge = reg_map.PERIPHERAL_BRIDGEBlock(base_addr=0xC000, read_fn=sim_read, write_fn=sim_write)
    assert bridge.bridge_config.offset == 0x00
    bridge.write_field(bridge.bridge_config, "BRIDGE_EN", 1)
    assert bridge.read_field(bridge.bridge_config, "BRIDGE_EN") == 1
    print("✓ PERIPHERAL_BRIDGE tested successfully")

    print("[Python Harness] All soc_large_scale driver tests PASSED!")

if __name__ == "__main__":
    main()
