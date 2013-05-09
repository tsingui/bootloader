/*
 * (C) Copyright 2011
 * Jason Cooper <u-boot@lakedaemon.net>
 *
 * Based on work by:
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Siddarth Gore <gores@marvell.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <miiphy.h>
#include <asm/arch/cpu.h>
#include <asm/arch/kirkwood.h>
#include <asm/arch/mpp.h>
#include <ddnas_board.h>
#include "ddnas.h"


DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the  below configuration configures mainly initial LED status
	 */
	kw_config_gpio(DDNAS_OE_VAL_LOW,
			DDNAS_OE_VAL_HIGH,
			DDNAS_OE_LOW, DDNAS_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	u32 kwmpp_config[] = {
		MPP0_SPI_SCn,     /* SPI Flash */
		MPP1_SPI_MOSI,
		MPP2_SPI_SCK,
		MPP3_SPI_MISO,
		MPP4_GPIO,
		MPP5_GPO,
		MPP6_SYSRST_OUTn,
		MPP7_GPO,	/* GPIO_RST */
		MPP8_TW_SDA,
		MPP9_TW_SCK,
		MPP10_UART0_TXD,     /* Serial */
		MPP11_UART0_RXD,
		MPP12_GPO,
		MPP13_GPIO,
		MPP14_GPIO,
#if 0
		MPP15_SATA0_ACTn,
		MPP16_SATA1_ACTn,
#endif
		MPP15_GPIO,     /* USB Copy button */
		MPP16_GPIO,	/* Reset button */
		MPP17_GPIO,
		MPP18_GPO,
		MPP19_GPO,
		MPP20_GE1_0,     /* Gigabit Ethernet */
		MPP21_GE1_1,
		MPP22_GE1_2,
		MPP23_GE1_3,
		MPP24_GE1_4,
		MPP25_GE1_5,
		MPP26_GE1_6,
		MPP27_GE1_7,
		MPP28_GPIO,
		MPP29_GPIO,
		MPP30_GE1_10,
		MPP31_GE1_11,
		MPP32_GE1_12,
		MPP33_GE1_13,
		MPP34_GE1_14,
		MPP35_GPIO,
		MPP36_GPIO,
		MPP37_GPIO,	/* USB LED */
		MPP38_GPIO,
		MPP39_GPIO,
		MPP40_GPIO,
		MPP41_GPIO,
		MPP42_GPIO,	/* Power LED */
		MPP43_GPIO,	/* Internet LED */
		MPP44_GPIO,
		MPP45_GPIO,
		MPP46_GPIO,
		MPP47_GPIO,
		MPP48_GPIO,
		MPP49_GPIO,
		0
	};
	kirkwood_mpp_conf(kwmpp_config);
	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	
	/*Set Power LED*/
	printf("%s:PowerLED on.\n",__FUNCTION__);
	gd->bd->bi_boot_params = kw_sdram_bar(0) + 0x100;
	__set_direction(DDNAS_POWER_LED_GPIO, 0);
	
	kw_gpio_set_value(DDNAS_POWER_LED_GPIO,0);
	return 0;
}

#ifdef CONFIG_RESET_PHY_R
/* Configure and enable MV88E1116/88E1121 PHY */
void mv_phy_init(char *name)
{
	u16 reg;
	u16 devadr;

	if (miiphy_set_current_dev(name))
		return;

	/* command to read PHY dev address */
	if (miiphy_read(name, 0xEE, 0xEE, (u16 *) &devadr)) {
		printf("Err..%s could not read PHY dev address\n",
			__FUNCTION__);
		return;
	}

	/*
	 * Enable RGMII delay on Tx and Rx for CPU port
	 * Ref: sec 4.7.2 of chip datasheet
	 */
	miiphy_write(name, devadr, MV88E1116_PGADR_REG, 2);
	miiphy_read(name, devadr, MV88E1116_MAC_CTRL_REG, &reg);
	reg |= (MV88E1116_RGMII_RXTM_CTRL | MV88E1116_RGMII_TXTM_CTRL);
	miiphy_write(name, devadr, MV88E1116_MAC_CTRL_REG, reg);
	miiphy_write(name, devadr, MV88E1116_PGADR_REG, 0);

	/* reset the phy */
	miiphy_reset(name, devadr);

	printf(PHY_NO" Initialized on %s\n", name);
}
void reset_phy(void)
{
	/* configure and initialize both PHY's */
	mv_phy_init("egiga0");
	
	/*
	 * lintel add.
	 *
	 * PHY Reset for second PHY. 
	 */
	debug("Reset PHY2.\n");
	miiphy_write("egiga1", 0xEE, 0xEE, 24);
	
	mv_phy_init("egiga1");
	
}
#endif /* CONFIG_RESET_PHY_R */
