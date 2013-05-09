#ifndef _U_BOOT_H_
#define _U_BOOT_H_      1

typedef struct bd_info {
        int             bi_baudrate;    /* serial console baudrate */
        unsigned long   bi_ip_addr;     /* IP Address */
        unsigned char   bi_enetaddr[6]; /* Ethernet adress */
        unsigned long   bi_arch_number; /* unique id for this board */
        unsigned long   bi_boot_params; /* where this board expects params */
        unsigned long   bi_memstart;    /* start of DRAM memory */
        unsigned long   bi_memsize;     /* size  of DRAM memory in bytes */
        unsigned long   bi_flashstart;  /* start of FLASH memory */
        unsigned long   bi_flashsize;   /* size  of FLASH memory */
        unsigned long   bi_flashoffset; /* reserved area for startup monitor */
} bd_t;
#define bi_env_data bi_env->data
#define bi_env_crc  bi_env->crc

#endif  /* _U_BOOT_H_ */


#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H
/*
 * The following data structure is placed in some memory wich is
 * available very early after boot (like DPRAM on MPC8xx/MPC82xx, or
 * some locked parts of the data cache) to allow for a minimum set of
 * global variables during system initialization (until we have set
 * up the memory controller so that we can use RAM).
 *
 * Keep it *SMALL* and remember to set CFG_GBL_DATA_SIZE > sizeof(gd_t)
 */

typedef	struct	global_data {
	bd_t		*bd;
	unsigned long	flags;
	unsigned long	baudrate;
	unsigned long	have_console;	/* serial_init() was called */
	unsigned long	reloc_off;	/* Relocation Offset */
	unsigned long	env_addr;	/* Address  of Environment struct */
	unsigned long	env_valid;	/* Checksum of Environment valid? */
	unsigned long	fb_base;	/* base address of frame buffer */
#ifdef CONFIG_VFD
	unsigned char	vfd_type;	/* display type */
#endif
#if 0
	unsigned long	cpu_clk;	/* CPU clock in Hz!		*/
	unsigned long	bus_clk;
	unsigned long	ram_size;	/* RAM size */
	unsigned long	reset_status;	/* reset status register at boot */
#endif
	void		**jt;		/* jump table */
} gd_t;

/*
 * Global Data Flags
 */
#define	GD_FLG_RELOC	0x00001		/* Code was relocated to RAM		*/
#define	GD_FLG_DEVINIT	0x00002		/* Devices have been initialized	*/
#define	GD_FLG_SILENT	0x00004		/* Silent mode				*/

#define DECLARE_GLOBAL_DATA_PTR     register volatile gd_t *gd asm ("r8")

#endif /* __ASM_GBL_DATA_H */
