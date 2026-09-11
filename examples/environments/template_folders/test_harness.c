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
    printf("[C Harness] Testing template_folders Register Map Definitions...\n");
    printf("Checking SAMPLE_BLOCK_SAMPLE_REG_OFFSET = 0x%X\n", (unsigned int)SAMPLE_BLOCK_SAMPLE_REG_OFFSET);
    assert(SAMPLE_BLOCK_SAMPLE_REG_OFFSET >= 0);
    printf("Checking SAMPLE_BLOCK_SAMPLE_REG_VAL_MASK = 0x%X\n", (unsigned int)SAMPLE_BLOCK_SAMPLE_REG_VAL_MASK);
    assert(SAMPLE_BLOCK_SAMPLE_REG_VAL_MASK != 0);
    printf("Checking sizeof(sample_block_regs_t) = %zu\n", sizeof(sample_block_regs_t));
    assert(sizeof(sample_block_regs_t) > 0);
    printf("[C Harness] All template_folders assertions PASSED!\n");
    return 0;
}
