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
    printf("[C Harness] Testing SPI Register Map Definitions...\n");
    assert(SPI_TOP_CTRL_OFFSET == 0x0000);
    assert(SPI_TOP_STATUS_OFFSET == 0x0004);
    assert(sizeof(spi_top_regs_t) == 8);

    uint32_t val = 0;
    val = SPI_TOP_CTRL_EN_SET(val, 1);
    assert(SPI_TOP_CTRL_EN_GET(val) == 1);
    assert(val == 0x1);

    val = SPI_TOP_CTRL_MODE_SET(val, 3);
    assert(SPI_TOP_CTRL_MODE_GET(val) == 3);
    assert(val == 0x7);

    printf("[C Harness] All SPI assertions PASSED!\n");
    return 0;
}
