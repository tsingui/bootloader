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

#if defined (CONFIG_SETUP_MEMORY_TAGS) || defined (CONFIG_CMDLINE_TAG)

	static void setup_start_tag (bd_t *bd);
        
# ifdef CONFIG_SETUP_MEMORY_TAGS
	static void setup_memory_tags (bd_t *bd);
# endif         
	static void setup_commandline_tag (bd_t *bd, char *commandline);
	static void setup_end_tag (bd_t *bd);

	static struct tag *params;
#endif /* CONFIG_SETUP_MEMORY_TAGS || CONFIG_CMDLINE_TAG */
/*cmd_boot.c*/
extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

int do_bootm_linux (unsigned int src)
{
	int ret;
	unsigned int entry;
	unsigned int comprLen;
	unsigned int uncompressedLength;
	void (*theKernel)(void );

	entry = CFG_SDRAM_BASE + KERNEL_OFFSET;

	/* infate RAM file */
	comprLen = 1024*1024*2;
        
	//reset uncompressedLength again
	uncompressedLength = 0xFFFFFFFF;

	printf ("## Transferring Linux code from 0x%x to 0x%x... \n",src,entry);
	ret = uncompressLZMA(entry, &uncompressedLength, src, comprLen);
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

	prepare_tags();

	/* we assume that the kernel is in place */
	if(!ret)
	{
		printf ("Starting kernel ...\n\r");
		cleanup_before_linux ();
		theKernel();
		
		return 0;
	}
}

int prepare_tags()
{
	bd_t *bd = gd->bd;

#ifdef CONFIG_CMDLINE_TAG
	char *commandline = { CONFIG_BOOTARGS };
#endif

	/* pass tags for Linux kernel */

#if defined (CONFIG_SETUP_MEMORY_TAGS) || defined (CONFIG_CMDLINE_TAG) 
    
	bd->bi_boot_params = CFG_SDRAM_BASE + BOOTARGS_OFFSET;
	
	setup_start_tag (bd);

#ifdef CONFIG_SETUP_MEMORY_TAGS
	setup_memory_tags (bd);
#endif
#ifdef CONFIG_CMDLINE_TAG
	setup_commandline_tag (bd, commandline);
#endif

	setup_end_tag (bd);
#endif

	return 0;
}

#if defined (CONFIG_SETUP_MEMORY_TAGS) || \
    defined (CONFIG_CMDLINE_TAG) 

static void setup_start_tag (bd_t *bd)
{
	params = (struct tag *) bd->bi_boot_params;

	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size (tag_core);

	params->u.core.flags = 0;
	params->u.core.pagesize = 0;
	params->u.core.rootdev = 0;

	params = tag_next (params);
}

#ifdef CONFIG_SETUP_MEMORY_TAGS
static void setup_memory_tags (bd_t *bd)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		params->hdr.tag = ATAG_MEM;
		params->hdr.size = tag_size (tag_mem32);

		params->u.mem.start = bd->bi_dram[i].start;
		params->u.mem.size = bd->bi_dram[i].size;

		params = tag_next (params);
	}
}
#endif /* CONFIG_SETUP_MEMORY_TAGS */

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
	printf("kernel_offset_first=0x%x,rootfs_offset_first=0x%x,debuglib_offset_first=0x%x\n",syscfg->layout.zone_offset[ZONE_KERNEL_FIRST],syscfg->layout.zone_offset[ZONE_ROOTFS_FIRST],syscfg->layout.zone_offset[ZONE_DEBUGLIB_FIRST],syscfg->layout.zone_offset[ZONE_EXTFS]);
	printf("kernel_offset_second=0x%x,rootfs_offset_second=0x%x,debuglib_offset_second=0x%x\n",syscfg->layout.zone_offset[ZONE_KERNEL_SECOND],syscfg->layout.zone_offset[ZONE_ROOTFS_SECOND],syscfg->layout.zone_offset[ZONE_DEBUGLIB_SECOND]);

	if( syscfg->image_mark == 0 )
	{
		sprintf(partinfo,  "%d(boot),%d(kernel),%d(rootfs),%d(appconfig),-(llconfig)",			
			syscfg->layout.zone_offset[ZONE_KERNEL_FIRST] - syscfg->layout.zone_offset[ZONE_BOOTLOADER],
			syscfg->layout.zone_offset[ZONE_ROOTFS_FIRST] - syscfg->layout.zone_offset[ZONE_KERNEL_FIRST],
			syscfg->layout.zone_offset[ZONE_APP_MTD] - syscfg->layout.zone_offset[ZONE_ROOTFS_FIRST],
			info->start[info->sector_count-2] - syscfg->layout.zone_offset[ZONE_APP_MTD] - CFG_FLASH_BASE);
	}
	else
	{
		sprintf(partinfo,  "%d(boot),%d(kernel),%d(rootfs),%d(appconfig),-(llconfig)",			
			syscfg->layout.zone_offset[ZONE_KERNEL_SECOND] - syscfg->layout.zone_offset[ZONE_BOOTLOADER],
			syscfg->layout.zone_offset[ZONE_ROOTFS_SECOND] - syscfg->layout.zone_offset[ZONE_KERNEL_SECOND],
			syscfg->layout.zone_offset[ZONE_APP_MTD] - syscfg->layout.zone_offset[ZONE_ROOTFS_SECOND],
			info->start[info->sector_count-2] - syscfg->layout.zone_offset[ZONE_APP_MTD] - CFG_FLASH_BASE);
	}

#else

	if( syscfg->layout.zone_offset[ZONE_EXTFS] != 0 )
	{
		sprintf(partinfo,  "%d(boot),%d(kernel),%d(rootfs),%d(jffs2),%d(appconfig),-(llconfig)",			
			syscfg->layout.zone_offset[ZONE_KERNEL] - syscfg->layout.zone_offset[ZONE_BOOTLOADER],
			syscfg->layout.zone_offset[ZONE_ROOTFS] - syscfg->layout.zone_offset[ZONE_KERNEL],
			syscfg->layout.zone_offset[ZONE_EXTFS] - syscfg->layout.zone_offset[ZONE_ROOTFS],
			syscfg->layout.zone_offset[ZONE_APP_MTD] - syscfg->layout.zone_offset[ZONE_EXTFS],
			info->start[info->sector_count-2] - syscfg->layout.zone_offset[ZONE_APP_MTD] - CFG_FLASH_BASE);
	}
	else
	{
		sprintf(partinfo,  "%d(boot),%d(kernel),%d(rootfs),%d(appconfig),-(llconfig)",			
			syscfg->layout.zone_offset[ZONE_KERNEL] - syscfg->layout.zone_offset[ZONE_BOOTLOADER],
			syscfg->layout.zone_offset[ZONE_ROOTFS] - syscfg->layout.zone_offset[ZONE_KERNEL],
			syscfg->layout.zone_offset[ZONE_APP_MTD] - syscfg->layout.zone_offset[ZONE_ROOTFS],
			info->start[info->sector_count-2] - syscfg->layout.zone_offset[ZONE_APP_MTD] - CFG_FLASH_BASE);
	}
	
#endif
	
    	debug("cmdline=\"%s\"\n",partinfo);

	if (!commandline)
		return;

	/* eat leading white space */
	for (p = commandline; *p == ' '; p++);

	/* skip non-existent command lines so the kernel will still
	 * use its default command line.
	 */
	if (*p == '\0')
		return;

	params->hdr.tag = ATAG_CMDLINE;
	params->hdr.size =
		(sizeof (struct tag_header) + strlen (p) + strlen(partinfo)+ 1 + 4) >> 2;

	strcpy (params->u.cmdline.cmdline, p);
	strcat (params->u.cmdline.cmdline, partinfo);

	params = tag_next (params);
}

static void setup_end_tag (bd_t *bd)
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}

#endif /* CONFIG_SETUP_MEMORY_TAGS || CONFIG_CMDLINE_TAG */
