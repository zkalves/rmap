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
