/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2023-2025 NXP
 */
#include <stdint.h>
#include <stdio.h>

#include "oei.h"
#include "board.h"
#include "rom_api.h"
#include "soc_ddr.h"
#include "time.h"
#include "build_info.h"

#include "fsl_sysctr.h"

#ifdef DDR_IEE
#include "iee.h"
#endif
#ifdef DDR_MEM_TEST
#include "memtest.h"
#endif

#ifdef SKUADC
extern int sku_adc(void);

#define SKU_CFG_DDR_2G      0x00
#define SKU_CFG_DDR_4G      0x01
#define SKU_CFG_DDR_8G      0x02
#define SKU_CFG_DDR_16G     0x03
#endif
#ifdef SKUGPIO
extern void gpio_config(void);
extern uint32_t gpio5_value(void);

#define SKU_CFG_DDR_2G      0x00
#define SKU_CFG_DDR_4G      0x02
#define SKU_CFG_DDR_8G      0x04
#define SKU_CFG_DDR_16G     0x06
#endif

#if defined(SKUGPIO) || defined(SKUADC)

#define DRAM_TIMING_ADDR0 0x4aa80000
#define DRAM_TIMING_ADDR1 0x4aa80100
#define DRAM_TIMING_ADDR2 0x4aa80200
#define DRAM_TIMING_ADDR3 0x4aa80300
#define DRAM_TIMING_ADDR4 0x4aa80400

#endif

/**
 * Load training data needed for quick boot flow from container
 *
 * @param offset    training data offset within the container
 *            = 0 if ROM has no support for dummy entry, non-zero otherwise
 *
 * @return        ROM_API_OKAY if data of expected size was loaded
 *            ROM_API_ERR_INV_PAR otherwise
 */
static uint32_t Ddr_Load_Training_Data(uint32_t offset)
{
    void *dest = (void *)QB_STATE_LOAD_ADDR;
    uint32_t size, off = offset;
    uint32_t lsize;

#if (defined(DDR_NO_PHY))
    /** No need to load training data */
    return ROM_API_OKAY;
#endif

    if (!off && Get_Training_Data_Offset(&off) != ROM_API_OKAY)
    {
        return ROM_API_ERR_INV_PAR;
    }

    /**
     * For stream devices such as USB the stream must point
     * to the following image body when OEI returns control to
     * ROM, therefore the loaded data size must be the entire
     * space allocated for DDR training data.
     *
     * For all other devices the optimal size of loaded data
     * is the size of ddrphy_qb_state structure.
     */
    lsize = Rom_Api_Boot_Dev_Is_Stream() ? QB_STATE_STORAGE_SIZE : sizeof(ddrphy_qb_state);
    size = Rom_Api_Read(off, lsize, dest);

    return (size == lsize ? ROM_API_OKAY : ROM_API_ERR_INV_PAR);
}

int oei_main(uint32_t argc, uint32_t *argv)
{
    int ret = 0;
    uint32_t sku_cfg;
    struct dram_timing_info *dram_timing_sku=NULL;
    uint32_t offset = 0, id = 0;
#if !defined(DEBUG)
    uint32_t ts, te, *tdiff;
#endif

    if (!timer_is_enabled())
        timer_enable();

#if !defined(DEBUG)
    ts = SYSCTR_GetUsec64();
#endif

    /* Board specific hardware initialization */
    BOARD_InitHardware();
    BOARD_InitPins();

#ifdef DDR_IEE
    prepare_iee();
#endif
    #ifdef SKUADC
    sku_cfg = sku_adc();
#endif
#ifdef SKUGPIO
    gpio_config();
    sku_cfg = ((gpio5_value() & 0x70)>>4); //GPIO5_IO_BIT(6~4)
#endif
#if defined(SKUGPIO) || defined(SKUADC)
    printf("** DDR OEI: sku_cfg:0x%x **\n",sku_cfg);
#endif

    printf("\nDDR OEI: (Build %lu, Commit %08lx, %s %s)\n\n",
        OEI_BUILD, OEI_COMMIT, OEI_DATE, OEI_TIME);

    printf("DDR OEI: Compiled for SOC %s, Board %s\n", OEI_DEVICES, OEI_BOARD);
    /**
     * Pass offset = 0 for iMX95 A0 since there is no ROM support
     * for training data dummy entry
     */
    if (argc >= 3 && argv[0] == OEI_ARG_TYPE_IN_IMG_OFF)
    {
        offset = argv[1];
        id = argv[2];
    }

    Ddr_Pre_Init();

    ret = Ddr_Load_Training_Data(offset);
    if (ret != ROM_API_OKAY)
    {
        return OEI_FAIL;
    }
    #if defined(SKUGPIO) || defined(SKUADC)

    if(sku_cfg==SKU_CFG_DDR_2G) //LEC-iMX95 2GB_MT62F1G16D1DS-023 2GB
        dram_timing_sku = (struct dram_timing_info * )DRAM_TIMING_ADDR2;
    else if(sku_cfg==SKU_CFG_DDR_4G) //LEC-iMX95 MT62F1G32D2DS-023 IT:C 4GB
        dram_timing_sku = (struct dram_timing_info * )DRAM_TIMING_ADDR1;
    else if(sku_cfg==SKU_CFG_DDR_8G) //LEC-iMX95 8GB_MT62F2G32D4DS-023 8GB
        dram_timing_sku = (struct dram_timing_info * )DRAM_TIMING_ADDR3;
    else if(sku_cfg==SKU_CFG_DDR_16G) //LEC-iMX95 16G_MT62F4G32D8DV-023 16GB
        dram_timing_sku = (struct dram_timing_info * )DRAM_TIMING_ADDR4;
    else dram_timing_sku = NULL;

    if(dram_timing_sku != NULL)
        ret = Ddrc_Init(dram_timing_sku, id);
    else
    {   // sku_cfg == 0
        /* used to test the dat image with EVK's DDR timing data.
           dram_timing_sku= (struct dram_timing_info * )DRAM_TIMING_ADDR0;
           ret = ddr_init(dram_timing_sku);
        */
#endif

    ret = Ddrc_Init(&dram_timing, id);
#if defined(SKUGPIO) || defined(SKUADC)
    }
#endif
    Ddr_Post_Init();

#ifdef DDR_IEE
    if (ret == 0)
    {
        ret = enable_iee();
    }
#endif
#ifdef DDR_MEM_TEST
    if (ret == 0)
    {
        ret = memtest();
    }
#endif
    printf("DDR OEI: done, err = %d\n", ret);

#if !defined(DEBUG)
    te = SYSCTR_GetUsec64();

    tdiff = (uint32_t *) (QB_STATE_SAVE_ADDR - sizeof(*tdiff));
    (*tdiff) = te - ts;
#endif

    return ret;
}
