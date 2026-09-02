/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2025 NXP
 */
#ifndef IEE_H

#define IEE_H

/**
 * Enable Inline Encription Engine (IEE) prerequisites
 */
void prepare_iee(void);

/**
 * Enable Inline Encryption Engine (IEE) on a pre-defined region
 *
 * @return 0 if IEE enabled succesfully, -1 otherwise
 */
int enable_iee(void);
#endif
