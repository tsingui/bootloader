/*
 * (C) Copyright 2007
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <version.h>
#include <flash.h>
#include <net.h>
#include <flash_layout_private.h>
#include <tbs_common.h>

#undef DEBUG
flash_info_t * info;
extern flash_info_t flash_info[CFG_MAX_FLASH_BANKS];           /* FLASH chips info */

extern int timer_init(void);

extern ulong uboot_end_data;
extern ulong uboot_end;

DECLARE_GLOBAL_DATA_PTR;
ulong monitor_flash_len;
const char version_string[] =
	U_BOOT_VERSION" (" __DATE__ " - " __TIME__ ")";

static char *failed = "*** failed ***\n";

/* Begin and End of memory area for malloc(), and current "brk" */
static ulong mem_malloc_start;
static ulong mem_malloc_end;
static ulong mem_malloc_brk;
extern void print_size (ulong size, const char *s);

/* The Malloc area is immediately below the monitor copy in DRAM*/
static void mem_malloc_init (void)
{
	ulong dest_addr = CFG_SDRAM_BASE + CFG_SDRAM_SIZE - SDRAM_SIZE + BOOTSTART_OFFSET;

	mem_malloc_end = dest_addr;
	mem_malloc_start = dest_addr - TOTAL_MALLOC_LEN;
	mem_malloc_brk = mem_malloc_start;
	memset ((void *) mem_malloc_start,
		0,
		mem_malloc_end - mem_malloc_start);
	
	alloc_init( ( unsigned char *)( CFG_SDRAM_BASE + CFG_SDRAM_SIZE - SDRAM_SIZE + ALLOC_INIT_OFFSET ) );

}

void *sbrk (ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new_addr = old + increment;

	if ((new_addr < mem_malloc_start) || (new_addr > mem_malloc_end)) 
	{
		return (NULL);
	}
	mem_malloc_brk = new_addr;
	return ((void *) old);
}

static int init_func_ram (void)
{
	puts ("DRAM:  ");

	if ((gd->ram_size = CFG_SDRAM_SIZE) > 0) 
	{
		print_size (gd->ram_size, "\n");
		return (0);
	}
	puts (failed);
	return (1);
}

static int display_banner(void)
{

	printf ("\n\n%s\n\n", version_string);
	return (0);
}

static void display_flash_config(ulong size)
{
	puts ("Flash: ");
	print_size (size, "\n");
}

static int init_baudrate (void)
{
	gd->baudrate = CONFIG_BAUDRATE;
	return (0);
}

/*
 * Breath some life into the board...
 *
 * The first part of initialization is running from Flash memory;
 * its main purpose is to initialize the RAM so that we
 * can relocate the monitor code to RAM.
 */

/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependend #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */
typedef int (init_fnc_t) (void);

init_fnc_t *init_sequence[] = 
{
	init_led,
	timer_init,
	init_baudrate,		/* initialze baudrate settings */
	serial_init,
	console_init_f,
	display_banner,		/* say that we are here */
	init_func_ram,
	NULL,
};

/************************************************************************
 *
 * This is the next part if the initialization sequence: we are now
 * running from RAM and have a "normal" C environment, i. e. global
 * data can be written, BSS has been cleared, the stack size in not
 * that critical any more, etc.
 *
 ************************************************************************
 */
void board_init_r ( void )
{
	cmd_tbl_t *cmdtp;
	bd_t *bd;
	init_fnc_t **init_fnc_ptr;
	ulong addr;	
	ulong size;
	sys_config_t *  sys_data;
	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("": : :"memory");

	sys_data= ( sys_config_t * ) ( CFG_SDRAM_BASE + SYSCFG_OFFSET );

	 /* Reserve memory for malloc() arena. */
	addr = CFG_SDRAM_BASE + CFG_SDRAM_SIZE - SDRAM_SIZE + SP_START;
	addr -= sizeof(gd_t);
	gd= (gd_t*) addr;
	memset ((void *)gd, 0, sizeof (gd_t));
	/*
	 * (permanently) allocate a Board Info struct
	 * and a permanent copy of the "global" data
	 */
	addr -= sizeof(bd_t);
	bd = (bd_t *)addr;
	gd->bd = bd;
	memset ((void *)bd, 0, sizeof (bd_t));

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) 
	{
		if ((*init_fnc_ptr)() != 0) 
		{
			hang ();
		}
	}
	
	/* Save local variables to board info struct */
	bd->bi_memstart	= CFG_SDRAM_BASE;	/* start of  DRAM memory */
	bd->bi_memsize	= gd->ram_size;		/* size  of  DRAM memory in bytes */
	bd->bi_baudrate	= gd->baudrate;		/* Console Baudrate */
	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */	

	gd->reloc_off = 0;	

	monitor_flash_len = (ulong)&uboot_end_data - TEXT_BASE;	

 	for (cmdtp = &__u_boot_cmd_start; cmdtp !=  &__u_boot_cmd_end; cmdtp++) 
	{
		ulong addr;

		addr = (ulong) (cmdtp->cmd) + gd->reloc_off;	
		debug ("Command \"%s\": 0x%08lx => 0x%08lx\n",
				cmdtp->name, (ulong) (cmdtp->cmd), addr);
	
		cmdtp->cmd =
			(int (*)(struct cmd_tbl_s *, int, int, char *[]))addr;

		if (cmdtp->usage) 
		{
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (char *)addr;
		}
#ifdef	CFG_LONGHELP
		if (cmdtp->help) 
		{
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (char *)addr;
		}
#endif
	}

	/* initialize malloc() area */
	mem_malloc_init();
	
	size = flash_init();
	display_flash_config (size);

	if( size )
	{
		if( item_check() == ERROR_ITEM_OK )
		{			
			sysdata_get(sys_data);
		}		
	}

	bd = gd->bd;
	bd->bi_flashstart = CFG_FLASH_BASE;
	bd->bi_flashsize = flash_info[0].size;

#if CFG_MONITOR_BASE == CFG_FLASH_BASE
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for U-Boot */
#else
	bd->bi_flashoffset = 0;
#endif

	if( is_sysdata( sys_data ) )
	{
		bd->bi_ip_addr = sys_data->ip ;		
		memcpy (bd->bi_enetaddr, sys_data->mac, 6);
	}
	else
	{
		printf("Can't get system configuration.Use default vlaue.\n");
	
#ifdef __BIG_ENDIAN
		bd->bi_ip_addr = 0xc0a80101;
#else	
		bd->bi_ip_addr = 0x101a8c0;
#endif
		bd->bi_enetaddr[0] = 0x00;
		bd->bi_enetaddr[1] = 0x02;
		bd->bi_enetaddr[2] = 0x03;
		bd->bi_enetaddr[3] = 0x04;
		bd->bi_enetaddr[4] = 0x05;
		bd->bi_enetaddr[5] = 0x06;
	}

	/* Initialize the console (after the relocation and devices init) */
	console_init_r ();

#if (CONFIG_COMMANDS & CFG_CMD_NET)
	eth_initialize(gd->bd);
#endif


	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;) 
	{
		main_loop ();
	}
}

void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}

