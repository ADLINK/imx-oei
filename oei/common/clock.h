/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2022-2025 NXP
 */

#ifndef CLOCK_H
#define CLOCK_H

#include <stdbool.h>
#include <stdint.h>

#define MHZ(x)	((x) * 1000000UL)

int Clock_Init(void);
void Dram_PLL_Init(uint64_t pll_val, bool ssc);
void Dram_Enable_Bypass(uint64_t clk_val);
void Dram_Disable_Bypass(void);
#endif
