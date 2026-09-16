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
    printf("[C Harness] Testing hw_precedence Register Map Definitions...\n");
    printf("Checking ARBITRATION_BLOCK_STATUS_FLAGS_OFFSET = 0x%X\n", (unsigned int)ARBITRATION_BLOCK_STATUS_FLAGS_OFFSET);
    assert(ARBITRATION_BLOCK_STATUS_FLAGS_OFFSET == 0x0000);
    printf("Checking ARBITRATION_BLOCK_STATUS_FLAGS_EVENT_FLAG_MASK = 0x%X\n", (unsigned int)ARBITRATION_BLOCK_STATUS_FLAGS_EVENT_FLAG_MASK);
    assert(ARBITRATION_BLOCK_STATUS_FLAGS_EVENT_FLAG_MASK == 0x1);
    printf("Checking ARBITRATION_BLOCK_STATUS_FLAGS_COUNTER_MASK = 0x%X\n", (unsigned int)ARBITRATION_BLOCK_STATUS_FLAGS_COUNTER_MASK);
    assert(ARBITRATION_BLOCK_STATUS_FLAGS_COUNTER_MASK == 0xFE);
    printf("Checking ARBITRATION_BLOCK_CONTROL_OFFSET = 0x%X\n", (unsigned int)ARBITRATION_BLOCK_CONTROL_OFFSET);
    assert(ARBITRATION_BLOCK_CONTROL_OFFSET == 0x0004);
    printf("Checking sizeof(arbitration_block_regs_t) = %zu\n", sizeof(arbitration_block_regs_t));
    assert(sizeof(arbitration_block_regs_t) == 8);
    printf("[C Harness] All hw_precedence assertions PASSED!\n");
    return 0;
}
