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
    print("[Python Harness] Testing narrow_bus_8bit Register Driver...")
    mem = {}
    def sim_read(addr):
        return mem.get(addr, 0)
    def sim_write(addr, val):
        mem[addr] = val

    block = reg_map.NARROW_UART_8BITBlock(base_addr=0x0, read_fn=sim_read, write_fn=sim_write)
    baud = getattr(block, 'baud_div')
    assert baud is not None
    assert baud.offset == 0x0
    print(f'Tested register {baud.name} at offset 0x{baud.offset:X}')

    status = getattr(block, 'status')
    assert status is not None
    assert status.offset == 0x1
    print(f'Tested register {status.name} at offset 0x{status.offset:X}')

    data = getattr(block, 'data')
    assert data is not None
    assert data.offset == 0x2
    print(f'Tested register {data.name} at offset 0x{data.offset:X}')

    config = getattr(block, 'config')
    assert config is not None
    assert config.offset == 0x3
    print(f'Tested register {config.name} at offset 0x{config.offset:X}')

    block.write_field(config, 'PARITY_EN', 1)
    assert block.read_field(config, 'PARITY_EN') == 1
    print('Tested field PARITY_EN read/write')
    print("[Python Harness] All narrow_bus_8bit driver tests PASSED!")

if __name__ == "__main__":
    main()
