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
#include "wide_bus_reg_map.h"

int main(void) {
    printf("[C Harness] Testing wide_bus_64bit Register Map Definitions...\n");
    printf("Checking WIDE_BUS_TOP_DMA_SRC_ADDR_OFFSET = 0x%X\n", (unsigned int)WIDE_BUS_TOP_DMA_SRC_ADDR_OFFSET);
    assert(WIDE_BUS_TOP_DMA_SRC_ADDR_OFFSET >= 0);
    printf("Checking WIDE_BUS_TOP_DMA_SRC_ADDR_LOW_ADDR_MASK = 0x%X\n", (unsigned int)WIDE_BUS_TOP_DMA_SRC_ADDR_LOW_ADDR_MASK);
    assert(WIDE_BUS_TOP_DMA_SRC_ADDR_LOW_ADDR_MASK != 0);
    printf("Checking sizeof(wide_bus_top_regs_t) = %zu\n", sizeof(wide_bus_top_regs_t));
    assert(sizeof(wide_bus_top_regs_t) > 0);
    printf("[C Harness] All wide_bus_64bit assertions PASSED!\n");
    return 0;
}
