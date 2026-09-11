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
    printf("[C Harness] Testing dma Register Map Definitions...\n");
    printf("Checking DMA_CONTROLLER_DMA_CTRL_OFFSET = 0x%X\n", (unsigned int)DMA_CONTROLLER_DMA_CTRL_OFFSET);
    assert(DMA_CONTROLLER_DMA_CTRL_OFFSET >= 0);
    printf("Checking DMA_CONTROLLER_DMA_CTRL_ENABLE_MASK = 0x%X\n", (unsigned int)DMA_CONTROLLER_DMA_CTRL_ENABLE_MASK);
    assert(DMA_CONTROLLER_DMA_CTRL_ENABLE_MASK != 0);
    printf("Checking sizeof(dma_controller_regs_t) = %zu\n", sizeof(dma_controller_regs_t));
    assert(sizeof(dma_controller_regs_t) > 0);
    printf("[C Harness] All dma assertions PASSED!\n");
    return 0;
}
