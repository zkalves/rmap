/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
