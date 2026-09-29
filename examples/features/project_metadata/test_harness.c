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
