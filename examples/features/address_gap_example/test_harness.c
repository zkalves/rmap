/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "sparse_reg_map.h"

int main(void) {
    printf("[C Harness] Testing address_gap_example Register Map Definitions...\n");
    printf("Checking SPARSE_DEVICE_CTRL_OFFSET = 0x%X\n", (unsigned int)SPARSE_DEVICE_CTRL_OFFSET);
    assert(SPARSE_DEVICE_CTRL_OFFSET >= 0);
    printf("Checking SPARSE_DEVICE_CTRL_ENABLE_MASK = 0x%X\n", (unsigned int)SPARSE_DEVICE_CTRL_ENABLE_MASK);
    assert(SPARSE_DEVICE_CTRL_ENABLE_MASK != 0);
    printf("Checking sizeof(sparse_device_regs_t) = %zu\n", sizeof(sparse_device_regs_t));
    assert(sizeof(sparse_device_regs_t) > 0);
    printf("[C Harness] All address_gap_example assertions PASSED!\n");
    return 0;
}
