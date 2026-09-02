/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2023-2025 NXP
 */

#ifndef TIME_H
#define TIME_H

#include <stdbool.h>
#include <stdint.h>

bool timer_is_enabled(void);
bool time_after(uint32_t us, uint32_t timeout);
void timer_enable(void);
uint32_t timer_get_us(void);
void udelay(uint32_t us);

#endif
