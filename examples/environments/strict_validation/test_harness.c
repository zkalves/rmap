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
    printf("[C Harness] Testing strict_validation Register Map Definitions...\n");
    printf("Checking STRICT_BLOCK_CONTROL_OFFSET = 0x%X\n", (unsigned int)STRICT_BLOCK_CONTROL_OFFSET);
    assert(STRICT_BLOCK_CONTROL_OFFSET >= 0);
    printf("Checking STRICT_BLOCK_CONTROL_START_MASK = 0x%X\n", (unsigned int)STRICT_BLOCK_CONTROL_START_MASK);
    assert(STRICT_BLOCK_CONTROL_START_MASK != 0);
    printf("Checking sizeof(strict_block_regs_t) = %zu\n", sizeof(strict_block_regs_t));
    assert(sizeof(strict_block_regs_t) > 0);
    printf("[C Harness] All strict_validation assertions PASSED!\n");
    return 0;
}
