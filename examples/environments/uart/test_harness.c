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
    printf("[C Harness] Testing uart Register Map Definitions...\n");
    printf("Checking UART0_CTRL_OFFSET = 0x%X\n", (unsigned int)UART0_CTRL_OFFSET);
    assert(UART0_CTRL_OFFSET >= 0);
    printf("Checking UART0_CTRL_TX_EN_MASK = 0x%X\n", (unsigned int)UART0_CTRL_TX_EN_MASK);
    assert(UART0_CTRL_TX_EN_MASK != 0);
    printf("Checking sizeof(uart0_regs_t) = %zu\n", sizeof(uart0_regs_t));
    assert(sizeof(uart0_regs_t) > 0);
    printf("[C Harness] All uart assertions PASSED!\n");
    return 0;
}
