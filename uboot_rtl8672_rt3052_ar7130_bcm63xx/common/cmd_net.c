/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <net.h>
#include <flash_layout_private.h>

extern void flush_cache (ulong start_addr, ulong size);

#if (CONFIG_COMMANDS & CFG_CMD_NET)

DECLARE_GLOBAL_DATA_PTR;

ulong load_addr = CONFIG_LOADADDR ;

static int netboot_common (proto_t, cmd_tbl_t *, int , char *[],unsigned int *file_size);


int do_tftpb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned int file_size;

	return netboot_common (TFTP, cmdtp, argc, argv,&file_size);
}


int do_tftp_save (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int file_size = 0;
	int ret = 0;

	ret = netboot_common(TFTP, cmdtp, argc, argv,&file_size);
	if( ret != 0 || file_size == -1)  /*if return 0,it is successful. */
		return ret;

	flash_update_img( ( unsigned char * )load_addr,  ( sys_config_t *  )  ( CFG_SDRAM_BASE + SYSCFG_OFFSET ) , file_size );
	
	return 0;
}



U_BOOT_CMD(
	tftp,	4,	3,	do_tftpb,
	"tftp	- download image via network using TFTP protocol\n",
	"[loadAddress] tftpServerIP bootfilename\n"
);



U_BOOT_CMD(
	savet,	4,	3,	do_tftp_save,
	"savet	- download IMG image via network using TFTP protocol and save it to flash\n",
	" tftpServerIP IMGFilename\n"
);

#ifdef CMD_SAVEB
int do_tftp_saveb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int file_size = 0;
	int ret = 0;
	char force_mark = 0;

	if( ( argc > 4 ) )
	{
		printf ("Usage:\n%s\n", cmdtp->usage);

		return 1;
	}

	if( ( argc == 4 ) && (strcmp(argv[3],"-f") == 0 ) )
	{
		force_mark = 1;
		argc = argc -1;
	}

	ret = netboot_common(TFTP, cmdtp, argc, argv,&file_size);
	if( ret != 0 || file_size == -1)  /*if return 0,it is successful. */
		return ret;
	
	flash_update_bin( file_size , force_mark );
	
	return 0;
}

U_BOOT_CMD(
	saveb,	4,	3,	do_tftp_saveb,
	"saveb	- download BIN image via network using TFTP protocol and save it to flash\n",
	" tftpServerIP BINFilename [-f]\n"
);
#endif

#ifdef CMD_BOOTH
int do_tftp_booth (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int file_size = 0;
	int ret = 0;
	int (*bootkernel)( void );

	ret = netboot_common(TFTP, cmdtp, argc, argv,&file_size);
	if( ret != 0 || file_size == -1)  /*if return 0,it is successful. */
		return ret;

	bootkernel = ( void * ) load_addr;

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


	printf("Now kernel starting ...\n");
	
	(*bootkernel)();
	return 0;
}

U_BOOT_CMD(
	booth,	4,	3,	do_tftp_booth,
	"booth	- boot kernel from host\n",
	" tftpServerIP vmlinux.bin\n"
);
#endif

static int
netboot_common (proto_t proto, cmd_tbl_t *cmdtp, int argc, char *argv[],unsigned int *file_size)
{
	//char *s;
	int   rcode = 0;

	/* pre-set load_addr */

	//load_addr = CONFIG_LOADADDR;

	switch (argc) {

	case 3:	NetTftpServerIP = string_to_ip(argv[1]);
		copy_filename (BootFile, argv[2], sizeof(BootFile));

		break;

	case 4:	load_addr = simple_strtoul(argv[1], NULL, 16);
		NetTftpServerIP = string_to_ip(argv[2]);
		copy_filename (BootFile, argv[3], sizeof(BootFile));

		break;

	default: printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if ((*file_size = NetLoop(proto)) < 0)
		return 1;
	/* NetLoop ok, update environment */
	//netboot_update_env();

	/* done if no file was loaded (no errors though) */
	if (*file_size == 0)
		return 0;

	/* flush cache */
	flush_cache(load_addr, *file_size);

	/* Loading ok, check if we should attempt an auto-start */
/*	if (((s = getenv("autostart")) != NULL) && (strcmp(s,"yes") == 0)) {
		char *local_args[2];
		local_args[0] = argv[0];
		local_args[1] = NULL;

		printf ("Automatic boot of image at addr 0x%08lX ...\n",
			load_addr);
		rcode = do_bootm (cmdtp, 0, 1, local_args);
	}
*/
	return rcode;
}

#if (CONFIG_COMMANDS & CFG_CMD_PING)
int do_ping (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	bd_t *bd = gd->bd;

	sys_config_t *  tmp_sysc;

	tmp_sysc = CFG_SDRAM_BASE + SYSCFG_OFFSET;

	if(is_sysdata(tmp_sysc))
	{
		bd->bi_ip_addr = tmp_sysc->ip ;
		memcpy (bd->bi_enetaddr, tmp_sysc->mac, 6);
	}
	else
	{
		printf("Can't get system configuration.Use defualt value.");

		return 1;
	}


	if (argc < 2)
		return -1;

	NetPingIP = string_to_ip(argv[1]);
	if (NetPingIP == 0) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return -1;
	}

	if (NetLoop(PING) < 0) {
		printf("ping failed; host %s is not alive\n", argv[1]);
		return 1;
	}

	printf("host %s is alive\n", argv[1]);

	return 0;
}

U_BOOT_CMD(
	ping,	2,	1,	do_ping,
	"ping\t- send ICMP ECHO_REQUEST to network host\n",
	"pingAddress\n"
);
#endif	/* CFG_CMD_PING */

#endif	/* CFG_CMD_NET */


