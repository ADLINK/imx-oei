/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2023 NXP
 */

/**
 * M33 Code TCM (TCML) Free Space: 0x1FFC0000 - 0x1FFFFFFF - according to RM
 * M33 Sys  TCM (TCMU) Free Space: 0x30000000 - 0x3002DFFF - according to CM33 ROM
 */
MEMORY {
	s_code (rx) : ORIGIN = 0x4aa90000, LENGTH = 0x00070000
	s1 (rx) : ORIGIN = 0x4aa80000, LENGTH = 0x00000100
	s2 (rx) : ORIGIN = 0x4aa80100, LENGTH = 0x00000100
	s3 (rx) : ORIGIN = 0x4aa80200, LENGTH = 0x00000100
	s4 (rx) : ORIGIN = 0x4aa80300, LENGTH = 0x00000100
	s5 (rx) : ORIGIN = 0x4aa80400, LENGTH = 0x00000100
}

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm) 

SECTIONS
{
	.ddr1 : {
		. = ALIGN(4);
		KEEP(*(.ddr1))
		*(.ddr1*)
	} > s1
	.ddr2 : {
		. = ALIGN(4);
		KEEP(*(.ddr2))
		*(.ddr2*)
	} > s2
	.ddr3 : {
		. = ALIGN(4);
		KEEP(*(.ddr3))
		*(.ddr3*)
	} > s3
	.ddr4 : {
		. = ALIGN(4);
		KEEP(*(.ddr4))
		*(.ddr4*)
	} > s4
	.ddr5 : {
		. = ALIGN(4);
		KEEP(*(.ddr5))
		*(.ddr5*)
	} > s5
	.text : {
		. = ALIGN(4);
		*(.text*)
	} > s_code

	.rodata : {
		. = ALIGN(4);
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	} > s_code

	.data : {
		. = ALIGN(4);
		*(.data*)
	} > s_code

	.end : {
		. = ALIGN(4);
		*(.__end)
	} > s_code
}

