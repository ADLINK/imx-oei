/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2025 NXP
 */
#ifndef COMMON_H
#define COMMON_H

#define ALIGN_MASK(x, m)    (((x) + (m)) & ~(m))
#define ALIGN(x, a)         ALIGN_MASK((x), (__typeof__(x))(a) - 1)
#define ALIGN_DOWN(x, a)    ALIGN((x) - ((a) - 1), (a))

#endif
