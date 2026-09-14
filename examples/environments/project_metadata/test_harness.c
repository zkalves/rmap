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
    printf("[C Harness] Testing project_metadata Register Map Definitions...\n");
    printf("Checking CORE_SUBSYSTEM_DEVICE_ID_OFFSET = 0x%X\n", (unsigned int)CORE_SUBSYSTEM_DEVICE_ID_OFFSET);
    assert(CORE_SUBSYSTEM_DEVICE_ID_OFFSET >= 0);
    printf("Checking CORE_SUBSYSTEM_DEVICE_ID_PART_NUM_MASK = 0x%X\n", (unsigned int)CORE_SUBSYSTEM_DEVICE_ID_PART_NUM_MASK);
    assert(CORE_SUBSYSTEM_DEVICE_ID_PART_NUM_MASK != 0);
    printf("Checking sizeof(core_subsystem_regs_t) = %zu\n", sizeof(core_subsystem_regs_t));
    assert(sizeof(core_subsystem_regs_t) > 0);
    printf("[C Harness] All project_metadata assertions PASSED!\n");
    return 0;
}
