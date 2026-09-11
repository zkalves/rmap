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
    printf("[C Harness] Testing sensor_hub Register Map Definitions...\n");
    printf("Checking SENSOR_CORE_CONFIG_OFFSET = 0x%X\n", (unsigned int)SENSOR_CORE_CONFIG_OFFSET);
    assert(SENSOR_CORE_CONFIG_OFFSET >= 0);
    printf("Checking SENSOR_CORE_CONFIG_ENABLE_MASK = 0x%X\n", (unsigned int)SENSOR_CORE_CONFIG_ENABLE_MASK);
    assert(SENSOR_CORE_CONFIG_ENABLE_MASK != 0);
    printf("Checking sizeof(sensor_core_regs_t) = %zu\n", sizeof(sensor_core_regs_t));
    assert(sizeof(sensor_core_regs_t) > 0);
    printf("[C Harness] All sensor_hub assertions PASSED!\n");
    return 0;
}
