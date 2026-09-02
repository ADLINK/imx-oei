#include <stdint.h>
#include <stdio.h>

#define GPIO5_RBASE     (((uint32_t)0x53850000))
#define GIOR5(x)		(GPIO5_RBASE + x)
#define PDDR (0x54)
#define PDIR (0x50)



static const struct gpio_cfg {
	uint32_t pddr;
	uint32_t pddrVal;

} gpio5_cfg[] = {
	{ GIOR5(PDDR), 0 }, 
	{ 0, 0},
};

void gpio_config(void)
{
	const struct gpio_cfg *cfg;

	for (cfg = gpio5_cfg; cfg->pddr != 0; cfg++) {
		*((volatile uint32_t *)cfg->pddr) = cfg->pddrVal;
	}
}

uint32_t gpio5_value(void)
{
	return *((volatile uint32_t *)(GIOR5(PDIR)));
}
