/* This file is part of rmap.
 *
 * rmap is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rmap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rmap.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "reg_map.h"

int main(void) {
    printf("[C Harness] Testing SPI Register Map Definitions...\n");
    assert(SPI_TOP_CTRL_OFFSET == 0x0000);
    assert(SPI_TOP_STATUS_OFFSET == 0x0004);
    assert(sizeof(spi_top_regs_t) == 8);

    uint32_t val = 0;
    val = SPI_TOP_CTRL_EN_SET(val, 1);
    assert(SPI_TOP_CTRL_EN_GET(val) == 1);
    assert(val == 0x1);

    val = SPI_TOP_CTRL_MODE_SET(val, 3);
    assert(SPI_TOP_CTRL_MODE_GET(val) == 3);
    assert(val == 0x7);

    printf("[C Harness] All SPI assertions PASSED!\n");
    return 0;
}
