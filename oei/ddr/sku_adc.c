#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include "MIMX95_COMMON.h"
#include "oei.h"
#include "adc.h"
//#include "fsl_sysctr.h"
//#include "time.h"

typedef uint32_t u32;
#define BIT(n) (1UL << (n))
#define __bf_shf(x) (__builtin_ffs(x) - 1)

#define FIELD_PREP(mask, val) (((val) << __bf_shf(mask)) & (mask))

#define FIELD_GET(mask, reg) \
    (((reg) & (mask)) >> __bf_shf(mask))

#define BITS_PER_LONG 32   // 32-bit
#define GENMASK(h, l) (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#define Write32_(v,a)  (*(volatile uint32_t *)(a) = (v))

#define IMX93_ADC_MCR			0x00
#define IMX93_ADC_MSR			0x04
#define IMX93_ADC_ISR			0x10
#define IMX93_ADC_IMR			0x20
#define IMX93_ADC_CIMR0			0x24
#define IMX93_ADC_CTR0			0x94
#define IMX93_ADC_NCMR0			0xA4
#define IMX93_ADC_PCDR0			0x100
#define IMX93_ADC_PCDR1			0x104
#define IMX93_ADC_PCDR2			0x108
#define IMX93_ADC_PCDR3			0x10c
#define IMX93_ADC_PCDR4			0x110
#define IMX93_ADC_PCDR5			0x114
#define IMX93_ADC_PCDR6			0x118
#define IMX93_ADC_PCDR7			0x11c
#define IMX93_ADC_CALSTAT		0x39C

#define IMX93_ADC_MCR_MODE_MASK		BIT(29)
#define IMX93_ADC_MCR_NSTART_MASK	BIT(24)
#define IMX93_ADC_MCR_CALSTART_MASK	BIT(14)
#define IMX93_ADC_MCR_ADCLKSE_MASK	BIT(8)
#define IMX93_ADC_MCR_PWDN_MASK		BIT(0)

#define IMX93_ADC_MSR_CALFAIL_MASK	BIT(30)
#define IMX93_ADC_MSR_CALBUSY_MASK	BIT(29)
#define IMX93_ADC_MSR_ADCSTATUS_MASK	GENMASK(2, 0)

#define IMX93_ADC_ISR_EOC_MASK		BIT(1)

#define IMX93_ADC_IMR_EOC_MASK		BIT(1)
#define IMX93_ADC_IMR_ECH_MASK		BIT(0)

#define IMX93_ADC_PCDR_CDATA_MASK	GENMASK(11, 0)

#define IDLE				0
#define POWER_DOWN			1
#define WAIT_STATE			2
#define BUSY_IN_CALIBRATION		3
#define SAMPLE				4
#define CONVERSION			6

#define IMX93_ADC_MAX_CHANNEL		3
#define IMX93_ADC_DAT_MASK		0xfff
#define IMX93_ADC_TIMEOUT		100000

//#define ETIMEDOUT 110 

#define readl_poll_timeout(addr, val, cond, timeout_us)                \
    ({                                                           \
        const uint32_t __delay = 10;                             \
        uint32_t __retry = (timeout_us) / __delay;               \
        int __ret = 0;                                           \
        uint32_t __count = __retry;                              \
        do {                                                     \
            (val) = Read32(addr);                                 \
            if (cond)                                            \
                break;                                           \
            SystemTimeDelay(__delay);                                   \
        } while (--__count > 0);                                 \
        if (__count == 0)                                        \
            __ret = -ETIMEDOUT;                                          \
        __ret;                                                   \
    })



struct imx93_adc_priv {
	uint32_t regs;
};


static void imx93_adc_power_down(struct imx93_adc_priv *adc)
{
	u32 mcr, msr;
	int ret;

	mcr = Read32(adc->regs + IMX93_ADC_MCR);
	mcr |= FIELD_PREP(IMX93_ADC_MCR_PWDN_MASK, 1);
	Write32_(mcr, adc->regs + IMX93_ADC_MCR);

	ret = readl_poll_timeout(adc->regs + IMX93_ADC_MSR, msr,
		((msr & IMX93_ADC_MSR_ADCSTATUS_MASK) == POWER_DOWN), 50);

	if (ret == -ETIMEDOUT)
        printf("ADC not in power down mode, current MSR: %x\n", msr);


}



static void imx93_adc_power_up(struct imx93_adc_priv *adc)
{
	u32 mcr;

	/* bring ADC out of power down state, in idle state */
	mcr = Read32(adc->regs + IMX93_ADC_MCR);
	mcr &= ~FIELD_PREP(IMX93_ADC_MCR_PWDN_MASK, 1);
	Write32_(mcr, adc->regs + IMX93_ADC_MCR);
}


static int imx93_adc_calibration(struct imx93_adc_priv *adc)
{
	u32 mcr, msr;
	int ret;

	/* make sure ADC is in power down mode */
	imx93_adc_power_down(adc);

	/* config SAR controller operating clock */
	mcr = Read32(adc->regs + IMX93_ADC_MCR);
	mcr &= ~FIELD_PREP(IMX93_ADC_MCR_ADCLKSE_MASK, 1);
	Write32_(mcr, adc->regs + IMX93_ADC_MCR);

	/* bring ADC out of power down state */
	imx93_adc_power_up(adc);

	/*
	 * we use the default TSAMP/NRSMPL/AVGEN in MCR,
	 * can add the setting of these bit if need
	 */

	/* run calibration */
	mcr = Read32(adc->regs + IMX93_ADC_MCR);
	mcr |= FIELD_PREP(IMX93_ADC_MCR_CALSTART_MASK, 1);
	Write32_(mcr, adc->regs + IMX93_ADC_MCR);

	/* wait calibration to be finished */
	ret = readl_poll_timeout(adc->regs + IMX93_ADC_MSR, msr,
		!(msr & IMX93_ADC_MSR_CALBUSY_MASK), 2000000);
	if (ret == -ETIMEDOUT) {
		printf("ADC calibration timeout\n");
		return ret;
	}

	/* check whether calbration is successful or not */
	msr = Read32(adc->regs + IMX93_ADC_MSR);
	if (msr & IMX93_ADC_MSR_CALFAIL_MASK) {
		printf("ADC calibration failed!\n");
		return -EAGAIN;
	}

	return 0;
}

static void imx93_adc_config_ad_clk(struct imx93_adc_priv *adc)
{
	u32 mcr;

	/* put adc in power down mode */
	imx93_adc_power_down(adc);

	/* config the AD_CLK equal to bus clock */
	mcr = Read32(adc->regs + IMX93_ADC_MCR);
	mcr |= FIELD_PREP(IMX93_ADC_MCR_ADCLKSE_MASK, 1);
	Write32_(mcr, adc->regs + IMX93_ADC_MCR);

	/* bring ADC out of power down state, in idle state */
	imx93_adc_power_up(adc);
}

static int imx93_adc_channel_data(struct imx93_adc_priv *adc, int channel,
			    unsigned int *data)
{

	u32 isr, pcda;
	int ret;
/*
	if (channel != adc->active_channel) {
		pr_err("Requested channel is not active!\n");
		return -EINVAL;
	}
*/
	ret = readl_poll_timeout(adc->regs + IMX93_ADC_ISR, isr,
		(isr & IMX93_ADC_ISR_EOC_MASK), IMX93_ADC_TIMEOUT);

	/* clear interrupts */
	Write32_(isr, adc->regs + IMX93_ADC_ISR);

	if (ret == -ETIMEDOUT) {
		printf("ADC conversion timeout!\n");
		return ret;
	}

	pcda = Read32(adc->regs + IMX93_ADC_PCDR0 + channel * 4);

	*data = FIELD_GET(IMX93_ADC_PCDR_CDATA_MASK, pcda);

	return 0;
}


static int imx93_adc_start_channel(struct imx93_adc_priv *adc, int channel)
{

	u32 imr, mcr;

	/* config channel mask register */
	Write32_(1 << channel, adc->regs + IMX93_ADC_NCMR0);

	/* config interrupt mask */
	imr = FIELD_PREP(IMX93_ADC_IMR_EOC_MASK, 1);
	Write32_(imr, adc->regs + IMX93_ADC_IMR);
	Write32_(1 << channel, adc->regs + IMX93_ADC_CIMR0);

	/* config one-shot mode */
	mcr = Read32(adc->regs + IMX93_ADC_MCR);
	mcr &= ~FIELD_PREP(IMX93_ADC_MCR_MODE_MASK, 1);
	Write32_(mcr, adc->regs + IMX93_ADC_MCR);

	/* start normal conversion */
	mcr = Read32(adc->regs + IMX93_ADC_MCR);
	mcr |= FIELD_PREP(IMX93_ADC_MCR_NSTART_MASK, 1);
	Write32_(mcr, adc->regs + IMX93_ADC_MCR);

	//adc->active_channel = channel;

	return 0;
}


int sku_adc(void)
{
    struct imx93_adc_priv adc_;
    struct imx93_adc_priv *adc=&adc_;
    unsigned int data=-1;
	int sku = 0;

    adc->regs = ADC_BASE;

    imx93_adc_calibration(adc);
    imx93_adc_config_ad_clk(adc);

    imx93_adc_start_channel(adc, 0);
    imx93_adc_channel_data(adc, 0, &data);
    printf("sku : adc ch0 data :0x%x ,%d\n",data,data);

    imx93_adc_start_channel(adc, 1);
    imx93_adc_channel_data(adc, 1, &data);
    printf("sku : adc ch1 data :0x%x ,%d\n",data,data);

    imx93_adc_start_channel(adc, 2);
    imx93_adc_channel_data(adc, 2, &data);
    printf("sku : adc ch2 data :0x%x ,%d\n",data,data);
	if(data > 1000)
		sku+=1;
	
    imx93_adc_start_channel(adc, 3);
    imx93_adc_channel_data(adc, 3, &data);
    printf("sku : adc ch3 data :0x%x ,%d\n",data,data);	
	if(data > 1000)
		sku+=2;

    return sku;
}

