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
    printf("[C Harness] Testing narrow_bus_8bit Register Map Definitions...\n");
    printf("Checking NARROW_UART_8BIT_BAUD_DIV_OFFSET = 0x%X\n", (unsigned int)NARROW_UART_8BIT_BAUD_DIV_OFFSET);
    assert(NARROW_UART_8BIT_BAUD_DIV_OFFSET == 0x0000);
    printf("Checking NARROW_UART_8BIT_STATUS_OFFSET = 0x%X\n", (unsigned int)NARROW_UART_8BIT_STATUS_OFFSET);
    assert(NARROW_UART_8BIT_STATUS_OFFSET == 0x0001);
    printf("Checking NARROW_UART_8BIT_DATA_OFFSET = 0x%X\n", (unsigned int)NARROW_UART_8BIT_DATA_OFFSET);
    assert(NARROW_UART_8BIT_DATA_OFFSET == 0x0002);
    printf("Checking NARROW_UART_8BIT_CONFIG_OFFSET = 0x%X\n", (unsigned int)NARROW_UART_8BIT_CONFIG_OFFSET);
    assert(NARROW_UART_8BIT_CONFIG_OFFSET == 0x0003);

    printf("Checking sizeof(narrow_uart_8bit_regs_t) = %zu\n", sizeof(narrow_uart_8bit_regs_t));
    assert(sizeof(narrow_uart_8bit_regs_t) == 4);
    printf("[C Harness] All narrow_bus_8bit assertions PASSED!\n");
    return 0;
}
