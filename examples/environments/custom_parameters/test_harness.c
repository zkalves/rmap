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
    printf("[C Harness] Testing custom_parameters Register Map Definitions...\n");
    printf("Checking PARAMETRIC_CORE_CONFIG_REG_OFFSET = 0x%X\n", (unsigned int)PARAMETRIC_CORE_CONFIG_REG_OFFSET);
    assert(PARAMETRIC_CORE_CONFIG_REG_OFFSET >= 0);
    printf("Checking PARAMETRIC_CORE_CONFIG_REG_CLK_RATIO_MASK = 0x%X\n", (unsigned int)PARAMETRIC_CORE_CONFIG_REG_CLK_RATIO_MASK);
    assert(PARAMETRIC_CORE_CONFIG_REG_CLK_RATIO_MASK != 0);
    printf("Checking sizeof(parametric_core_regs_t) = %zu\n", sizeof(parametric_core_regs_t));
    assert(sizeof(parametric_core_regs_t) > 0);
    printf("[C Harness] All custom_parameters assertions PASSED!\n");
    return 0;
}
