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
 * 
 *  2012.02.14 lintel<lintel.huang@gmail.com>
 * 
 */

#ifndef _CONFIG_DDNAS_H
#define _CONFIG_DDNAS_H

#if 0
/*
 * FIXME: This belongs in mach-types.h.  However, we only pull mach-types
 * from Linus' kernel.org tree.  This hasn't been updated primarily due to
 * the recent arch/arm reshuffling.  So, in the meantime, we'll place it
 * here.
 */
#include <asm/mach-types.h>
#ifdef MACH_TYPE_DDNAS
#error "MACH_TYPE_DDNAS has been defined properly, please remove this."
#else
#define MACH_TYPE_DDNAS            3550
#endif
#endif

#if 0
#define CONFIG_POST	CONFIG_SYS_POST_MEMORY
#define _POST_WORD_ADDR	0x1000000
#endif
/*
 * Version number information
 */
#define CONFIG_IDENT_STRING	"\n\nBroad:DDNAS-Wireless V1.0\n\nLintel<lintel.huang@gmail.com> 2012.02.14"
#define CONFIG_DDNAS	1
/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_SHEEVA_88SV131	1	/* CPU Core subversion */
#define CONFIG_KIRKWOOD		1	/* SOC Family Name */
#define CONFIG_KW88F6281	1	/* SOC Name */
#define CONFIG_MACH_TYPE	MACH_TYPE_DDNAS
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

/* GPIO setup */
#define CONFIG_KIRKWOOD_GPIO	1
//#define CONFIG_CMD_GPIO	1

#define ENABLE_CMD_LOADB_X	1
//download menu
//#define CONFIG_MENU	1
#define CONFIG_UBOOT_MENU 1
#define	CONFIG_MENUPROMPT "Hit any key to enter boot menu: %2d "


#define  CONFIG_CMD_TFTPSRV	1
#define  CONFIG_TFTP_BLOCKSIZE 512
#define  CONFIG_CMD_TFTPPUT	1
/*
 * Commands configuration
 */
#define CONFIG_SYS_NO_FLASH		/* Declare no flash (NOR/SPI) */
#include <config_cmd_default.h>
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ENV
#define CONFIG_CMD_FAT
#define CONFIG_CMD_SF
#define CONFIG_CMD_PING
#define CONFIG_CMD_USB
#define CONFIG_CMD_IDE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_NAND
#define CONFIG_CMD_ELF

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/*
 *  Environment variables configurations
 */
#ifdef CONFIG_SPI_FLASH
#define CONFIG_ENV_IS_IN_SPI_FLASH	1
#define CONFIG_ENV_SECT_SIZE		0x20000	/* 128k */
#else
#define CONFIG_ENV_IS_NOWHERE		1	/* if env in SDRAM */
#endif

#ifdef CONFIG_CMD_SF
#define CONFIG_SPI_FLASH		1
#define CONFIG_HARD_SPI			1
#define CONFIG_KIRKWOOD_SPI		1
#define CONFIG_SPI_FLASH_MACRONIX	1
#define CONFIG_SPI_FLASH_ATMEL		1
#define CONFIG_SPI_FLASH_EON		1
#define CONFIG_SPI_FLASH_SPANSION	1
#define CONFIG_SPI_FLASH_SST		1
#define CONFIG_SPI_FLASH_STMICRO	1
#define CONFIG_SPI_FLASH_WINBOND	1
#define CONFIG_SPI_FRAM_RAMTRON		1
#define CONFIG_ENV_SPI_BUS		0
#define CONFIG_ENV_SPI_CS		0
#define CONFIG_ENV_SPI_MAX_HZ		50000000 /* 50 MHz */
#endif

/*
 * max 4k env size is enough, but in case of nand
 * it has to be rounded to sector size
 */
#define CONFIG_ENV_SIZE			0x40000  /* 256k */
#define CONFIG_ENV_ADDR			0x80000
#define CONFIG_ENV_OFFSET		0x80000 /* env starts here */

#define	CONFIG_SYS_PROMPT_SETUP
#define	CONFIG_SYS_PROMPT	"[u-boot@DDNAS]# "	/* Command Prompt */

/*
 * Default environment variables
 */

#if 0
#define CONFIG_BOOTCOMMAND		"setenv ethact egiga0; " \
	"${x_bootcmd_spi}; setenv ethact egiga1; " \
	"${x_bootcmd_ethernet}; setenv ethact egiga1; " \
	"${x_bootcmd_ethernet}; ${x_bootcmd_usb}; ${x_bootcmd_kernel}; "\
	"setenv bootargs ${x_bootargs} ${x_bootargs_root}; "	\
	"bootm 0x6400000;"
#endif	

#define CONFIG_BOOTCOMMAND	"sf probe 0;sf read 0x6400000 0x100000 0x200000;bootm 0x6400000\0;"
	
#define CONFIG_BOOTARGS		"root=/dev/mtdblock2 noinitrd console=ttyS0,115200"
	
#define CONFIG_EXTRA_ENV_SETTINGS	\
 	"x_bootcmd_ethernet=ping 192.168.1.121\0"	\
	"x_bootcmd_usb=usb start\0"	\
	"x_bootcmd_kernel=fatload usb 0 0x6400000 uImage\0" \
	"x_bootargs=console=ttyS0,115200\0"	\
	"x_bootargs_root=root=/dev/mtdblock rootdelay=10\0" \
	"x_bootcmd_spi=sf probe 0;sf read 0x6400000 0x100000 0x200000;bootm 0x6400000\0"

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define PHY_NO		"88E1116"
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR		192.168.1.1
#define CONFIG_SERVERIP		192.168.1.100
#define CONFIG_GATEWAYIP	192.168.1.1
#define CONFIG_MVGBE_PORTS	{1, 1}	/* enable both ports */
#define CONFIG_PHY_BASE_ADR	0x8
//#define CONFIG_PHY_BASE_ADR	0x18
#endif /* CONFIG_CMD_NET */


/*
 * RTC driver configuration
 */
#ifdef CONFIG_CMD_DATE
#define CONFIG_RTC_MV
#endif /* CONFIG_CMD_DATE */

#define CONFIG_SYS_ALT_MEMTEST

/*
 * display enhanced info about the cpu at boot.
 */
#define CONFIG_DISPLAY_CPUINFO


/*
 * SATA Driver configuration
 */
#ifdef CONFIG_MVSATA_IDE
#define CONFIG_SYS_ATA_IDE0_OFFSET	MV_SATA_PORT0_OFFSET
#define CONFIG_SYS_ATA_IDE1_OFFSET	MV_SATA_PORT1_OFFSET
#endif /*CONFIG_MVSATA_IDE*/


#endif /* _CONFIG_DDNAS_H */
