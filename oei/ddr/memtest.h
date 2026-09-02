/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2025 NXP
 */
#ifndef MEMTEST_H

#define MEMTEST_H

/**
 * Memtest function performs rudimentary memory write/read/check test
 *
 * @return 0 if no failures found, -1 otherwise
 */
int memtest(void);
#endif
