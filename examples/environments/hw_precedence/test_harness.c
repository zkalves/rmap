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
