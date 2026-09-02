/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2025 NXP
 */
#include <stdint.h>
#include <stdio.h>
#include "oei.h"
#include "memtest.h"

#define DDR_MEM_BASE    0x80000000U
#define SIZE_512M       0x20000000U

static uint32_t mem_wrc(uint32_t addr, uint32_t val, uint32_t index, uint32_t len)
{
    uint32_t fail = 0;
    uint32_t i;
    uint32_t j;

    j = val;
    for (i = index; i < len; i++)
    {
        uint32_t dest = addr + (0x4U * i);

        Write32(dest, j);
        j++;
    }

    j = val;
    for (i = index; i < len; i++)
    {
        uint32_t dest = addr + (0x4U * i);
        uint32_t data = Read32(dest);

        if (data != j)
        {
            fail++;
        }
        j++;
    }

    return fail;
}

int memtest(void)
{
    uint32_t fail = 0U;

    fail += mem_wrc(DDR_MEM_BASE, 0xfabefaceU, 0U, 10U);
    fail += mem_wrc(DDR_MEM_BASE, 0xdeadbeefU, 10U, 0x100U);

    fail += mem_wrc(DDR_MEM_BASE + SIZE_512M, 0x98760000U, 0U, 10U);
    fail += mem_wrc(DDR_MEM_BASE + SIZE_512M, 0xabcd0000U, 10U, 0x100U);

    if (fail > 0U)
    {
        printf("DDR OEI: memtest fails: %u\n", fail);
    }
    else
    {
        printf("DDR OEI: memtest pass!\n");
    }

    return ((fail > 0U) ? -1 : 0);
}
