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
    printf("[C Harness] Testing software_locks Register Map Definitions...\n");
    printf("Checking SECURITY_CORE_SEC_CTRL_OFFSET = 0x%X\n", (unsigned int)SECURITY_CORE_SEC_CTRL_OFFSET);
    assert(SECURITY_CORE_SEC_CTRL_OFFSET == 0x0000);

    printf("Checking SECURITY_CORE_SEC_KEY_OFFSET = 0x%X\n", (unsigned int)SECURITY_CORE_SEC_KEY_OFFSET);
    assert(SECURITY_CORE_SEC_KEY_OFFSET == 0x0004);

    printf("Checking SECURITY_CORE_CRYPTO_CFG_OFFSET = 0x%X\n", (unsigned int)SECURITY_CORE_CRYPTO_CFG_OFFSET);
    assert(SECURITY_CORE_CRYPTO_CFG_OFFSET == 0x0008);

    printf("Checking SECURITY_CORE_MULTI_FIELD_REG_OFFSET = 0x%X\n", (unsigned int)SECURITY_CORE_MULTI_FIELD_REG_OFFSET);
    assert(SECURITY_CORE_MULTI_FIELD_REG_OFFSET == 0x000C);

    printf("Checking field masks in MULTI_FIELD_REG...\n");
    assert(SECURITY_CORE_MULTI_FIELD_REG_PUBLIC_STATUS_MASK == 0x000000FF);
    assert(SECURITY_CORE_MULTI_FIELD_REG_KEY_SLICE_MASK == 0x0000FF00);
    assert(SECURITY_CORE_MULTI_FIELD_REG_PASSCODE_MASK == 0x00FF0000);
    assert(SECURITY_CORE_MULTI_FIELD_REG_SECURE_CTRL_MASK == 0xFF000000);

    printf("Checking sizeof(security_core_regs_t) = %zu\n", sizeof(security_core_regs_t));
    assert(sizeof(security_core_regs_t) == 16);

    printf("[C Harness] All software_locks C assertions PASSED!\n");
    return 0;
}
