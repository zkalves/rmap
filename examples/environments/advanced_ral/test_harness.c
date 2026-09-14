/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "reg_map.h"

int main(void) {
    printf("[C Harness] Testing advanced_ral Register Map Definitions...\n");

    printf("Checking CFG_INDEX offset = 0x%X\n", (unsigned int)RAL_ADVANCED_BLOCK_CFG_INDEX_OFFSET);
    assert(RAL_ADVANCED_BLOCK_CFG_INDEX_OFFSET == 0x0);

    printf("Checking CFG_DATA offset = 0x%X\n", (unsigned int)RAL_ADVANCED_BLOCK_CFG_DATA_OFFSET);
    assert(RAL_ADVANCED_BLOCK_CFG_DATA_OFFSET == 0x4);

    printf("Checking FIFO_PORT offset = 0x%X\n", (unsigned int)RAL_ADVANCED_BLOCK_FIFO_PORT_OFFSET);
    assert(RAL_ADVANCED_BLOCK_FIFO_PORT_OFFSET == 0x8);

    printf("Checking STATUS_CBS offset = 0x%X\n", (unsigned int)RAL_ADVANCED_BLOCK_STATUS_CBS_OFFSET);
    assert(RAL_ADVANCED_BLOCK_STATUS_CBS_OFFSET == 0xC);

    printf("Checking INTR_FLAGS offset = 0x%X\n", (unsigned int)RAL_ADVANCED_BLOCK_INTR_FLAGS_OFFSET);
    assert(RAL_ADVANCED_BLOCK_INTR_FLAGS_OFFSET == 0x10);

    printf("Checking CFG_INDEX_INDEX_MASK = 0x%X\n", (unsigned int)RAL_ADVANCED_BLOCK_CFG_INDEX_INDEX_MASK);
    assert(RAL_ADVANCED_BLOCK_CFG_INDEX_INDEX_MASK == 0xFFFF);

    printf("Checking CFG_INDEX_AUTO_INC_MASK = 0x%X\n", (unsigned int)RAL_ADVANCED_BLOCK_CFG_INDEX_AUTO_INC_MASK);
    assert(RAL_ADVANCED_BLOCK_CFG_INDEX_AUTO_INC_MASK == 0x10000);

    printf("Checking STATUS_CBS_READY_MASK = 0x%X\n", (unsigned int)RAL_ADVANCED_BLOCK_STATUS_CBS_READY_MASK);
    assert(RAL_ADVANCED_BLOCK_STATUS_CBS_READY_MASK == 0x2);

    printf("Checking sizeof(ral_advanced_block_regs_t) = %zu\n", sizeof(ral_advanced_block_regs_t));
    assert(sizeof(ral_advanced_block_regs_t) >= 0x14);

    printf("[C Harness] All advanced_ral assertions PASSED!\n");
    return 0;
}
