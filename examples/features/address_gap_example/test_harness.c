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
