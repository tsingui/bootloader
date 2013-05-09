/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 2001  Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	 USA
 *
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <zlib.h>
#include <asm/byteorder.h>

DECLARE_GLOBAL_DATA_PTR;

extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern int uncompressLZMA ( Bytef *ptrDest,uLongf *destLen, Bytef *ptrSrc, uLong sourceLen);
extern int	is_sysdata(sys_config_t *syscfg);
extern int flash_read_buf(flash_info_t * info, void* addr, unsigned char ** buf, int len);
static void setup_commandline_tag (bd_t *bd, char *commandline);

/*cmd_boot.c*/

int do_bootm_linux (unsigned int src)
{
	int ret;
	unsigned int entry;
	unsigned int comprLen;
	unsigned int uncompressedLength;
	void (*theKernel)(void );
	unsigned char *buf;

	entry = CFG_SDRAM_BASE + KERNEL_OFFSET;

	/* infate RAM file */
	comprLen = 1024*1024*2;
        
	//reset uncompressedLength again
	uncompressedLength = 0xFFFFFFFF;

	printf ("## Transferring Linux code from 0x%x to 0x%x... ",src,entry);


	buf = ( unsigned char * )( CFG_SDRAM_BASE + KERNEL_OFFSET_TMP );
	flash_read_buf( info , ( unsigned char * ) src , &buf , comprLen );	
	
	src = ( unsigned int )buf;

	ret = uncompressLZMA( ( Bytef *) entry , ( uLongf *) &uncompressedLength , ( Bytef *) src, comprLen);

	theKernel = (void (*)(void))entry;	   

	if(!ret)
	{
		printf("##ok!\n\r");

	}
	else
	{
		printf("##failed!\n\r");
		return 1;
	}

	/* we assume that the kernel is in place */
	if(!ret)
	{
		printf ("Starting kernel at ...\n\r");

		//cleanup_before_linux ();
#ifdef CONFIG_RTL8672
		__asm__ volatile(
			"mtc0 $0,$20\n\t"
			"nop\n\t"
			"li $8,0x00000200\n\t"
			"mtc0 $8,$20\n\t"
			"nop\n\t"
			"nop\n\t"
			"mtc0 $0,$20\n\t"
			"nop"
			: /* no output */
			: /* no input */
				);
#endif

		theKernel();
		
		return 0;
	}
	return 0;
}

int prepare_tags()
{
	bd_t *bd = gd->bd;

#ifdef CONFIG_CMDLINE_TAG
	char *commandline = { CONFIG_BOOTARGS };
#endif

	/* pass tags for Linux kernel */    
	bd->bi_boot_params = CFG_SDRAM_BASE + BOOTARGS_OFFSET;

#ifdef CONFIG_CMDLINE_TAG
	setup_commandline_tag (bd, commandline);
#endif
	return 0;
}

static void setup_commandline_tag (bd_t *bd, char *commandline)
{
	char *p;
	char partinfo[128];
	sys_config_t* syscfg;
	
	syscfg =(sys_config_t*)(CFG_SDRAM_BASE + SYSCFG_OFFSET);

	if(!is_sysdata(syscfg))
	{
		printf ("syscfg error\n");
		return;
	}
#ifdef CONFIG_DOUBLE_BACKUP
	if( syscfg->image_mark == 0 )
	{
		sprintf(partinfo,  "%d(boot),%d(kernel),-(rootfs)",			
			syscfg->layout.zone_offset[ZONE_KERNEL_FIRST] - syscfg->layout.zone_offset[ZONE_BOOTLOADER],
			syscfg->layout.zone_offset[ZONE_ROOTFS_FIRST] - syscfg->layout.zone_offset[ZONE_KERNEL_FIRST]);
	}
	else
	{
		sprintf(partinfo,  "%d(boot),%d(kernel),-(rootfs)",			
			syscfg->layout.zone_offset[ZONE_KERNEL_SECOND] - syscfg->layout.zone_offset[ZONE_BOOTLOADER],
			syscfg->layout.zone_offset[ZONE_ROOTFS_SECOND] - syscfg->layout.zone_offset[ZONE_KERNEL_SECOND]);
	}

#else
	sprintf(partinfo,  "%d(boot),%d(kernel),-(rootfs)",			
		syscfg->layout.zone_offset[ZONE_KERNEL_FIRST] - syscfg->layout.zone_offset[ZONE_BOOTLOADER],
		syscfg->layout.zone_offset[ZONE_ROOTFS_FIRST] - syscfg->layout.zone_offset[ZONE_KERNEL_FIRST]);
#endif

	if (!commandline)
		return;

	/* eat leading white space */
	for (p = commandline; *p == ' '; p++);

	/* skip non-existent command lines so the kernel will still
	 * use its default command line.
	 */
	if (*p == '\0')
	{
		printf("please define CONFIG_BOOTARGS in bootloader header file.\n");
		return;
	}
	
	strcpy ( ( unsigned char * ) bd->bi_boot_params , p );
	strcat ( ( unsigned char * ) bd->bi_boot_params , partinfo );

    	debug("bootargs:%s \n",bd->bi_boot_params);
}
