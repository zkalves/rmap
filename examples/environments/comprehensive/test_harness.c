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
    printf("[C Harness] Testing comprehensive Register Map Definitions...\n");
    printf("Checking CORE_SUBSYSTEM_CONTROL_OFFSET = 0x%X\n", (unsigned int)CORE_SUBSYSTEM_CONTROL_OFFSET);
    assert(CORE_SUBSYSTEM_CONTROL_OFFSET == 0x0000);
    printf("Checking CORE_SUBSYSTEM_CONTROL_ENABLE_MASK = 0x%X\n", (unsigned int)CORE_SUBSYSTEM_CONTROL_ENABLE_MASK);
    assert(CORE_SUBSYSTEM_CONTROL_ENABLE_MASK == 0x1);

    printf("Checking CORE_SUBSYSTEM_ONCE_AND_TOGGLE_OFFSET = 0x%X\n", (unsigned int)CORE_SUBSYSTEM_ONCE_AND_TOGGLE_OFFSET);
    assert(CORE_SUBSYSTEM_ONCE_AND_TOGGLE_OFFSET == 0x0010);
    assert(CORE_SUBSYSTEM_ONCE_AND_TOGGLE_WRITE_ONCE_MASK == 0x0F);
    assert(CORE_SUBSYSTEM_ONCE_AND_TOGGLE_TOGGLE_ON_1_MASK == 0xF00);

    printf("Checking CORE_SUBSYSTEM_WRITE_READ_SIDE_EFFECTS_OFFSET = 0x%X\n", (unsigned int)CORE_SUBSYSTEM_WRITE_READ_SIDE_EFFECTS_OFFSET);
    assert(CORE_SUBSYSTEM_WRITE_READ_SIDE_EFFECTS_OFFSET == 0x0014);
    assert(CORE_SUBSYSTEM_WRITE_READ_SIDE_EFFECTS_WRITE_CLEAR_WHOLE_MASK == 0x0F);

    printf("Checking CORE_SUBSYSTEM_COMBINED_BIT_SIDE_EFFECTS_OFFSET = 0x%X\n", (unsigned int)CORE_SUBSYSTEM_COMBINED_BIT_SIDE_EFFECTS_OFFSET);
    assert(CORE_SUBSYSTEM_COMBINED_BIT_SIDE_EFFECTS_OFFSET == 0x0018);
    assert(CORE_SUBSYSTEM_COMBINED_BIT_SIDE_EFFECTS_W1_SET_READ_CLEAR_MASK == 0x0F);

    printf("Checking CORE_SUBSYSTEM_WRITE_ONLY_SIDE_EFFECTS_OFFSET = 0x%X\n", (unsigned int)CORE_SUBSYSTEM_WRITE_ONLY_SIDE_EFFECTS_OFFSET);
    assert(CORE_SUBSYSTEM_WRITE_ONLY_SIDE_EFFECTS_OFFSET == 0x001C);
    assert(CORE_SUBSYSTEM_WRITE_ONLY_SIDE_EFFECTS_WRITE_ONLY_CLEAR_MASK == 0x0F);

    printf("Checking sizeof(core_subsystem_regs_t) = %zu (expected 32)\n", sizeof(core_subsystem_regs_t));
    assert(sizeof(core_subsystem_regs_t) == 32);
    printf("[C Harness] All comprehensive assertions PASSED!\n");
    return 0;
}
