//#ifndef __SDRAM_LAYOUT_H__
//#define __SDRAM_LAYOUT_H__

#define	SDRAM_SIZE		0x400000

#define	BOOTARGS_OFFSET		0x1000
#define 	SYSCFG_OFFSET		0x1100
#define	FLASH_INFO		0x1200

#ifdef SOLOSW
#define	KERNEL_OFFSET		0x18000
#endif

#ifdef ATHEROS
#define	KERNEL_OFFSET		0x60000
#endif

#ifdef AR2317
#define	KERNEL_OFFSET		0x60000
#endif

#ifdef AMAZON_SE
#define	KERNEL_OFFSET		0x2000
#endif

#ifdef  CONFIG_VX160
#define	KERNEL_OFFSET		0x10000
#endif

#ifdef  CONFIG_VX180
#define	KERNEL_OFFSET		0x10000
#endif

#ifdef  CONFIG_RT3052
#define	KERNEL_OFFSET		0x10000
#endif

#ifdef  CONFIG_RTL8672
#define	KERNEL_OFFSET		0x10000
#endif

#define	BOOLOADER_OFFSET				( SDRAM_SIZE - 0x80000 )						/* 0x380000 -- 0x400000 */
#define	BOOTSTART_OFFSET				( SDRAM_SIZE - 0xc0000 )						/* 0x340000 -- 0x380000 */

#define	CFG_MALLOC_LEN				( 128 * 1024 )
#define	ALLOC_INIT_SIZE				( 10 * 1024 )
#define	TOTAL_MALLOC_LEN				( 256 * 1024 )									/*( CFG_MALLOC_LEN + ALLOC_INIT_SIZE + 118*1024 )   0x340000 -- 0x380000 */
#define	ALLOC_INIT_OFFSET				( BOOTSTART_OFFSET - CFG_MALLOC_LEN - ALLOC_INIT_SIZE )

#define	SP_END							( BOOTSTART_OFFSET - TOTAL_MALLOC_LEN )		/*  0x300000  */
#define	SP_OFFSET						( SP_END - 0x20000 )							/*  0x2E0000  */
#define	SP_START						( SP_END - 0x40000 )							/*  SP: 0x2C0000 -- 0x300000  */

/* bootloader gd_t bd_t  */																/* 0x2A0000 -- 0x2C0000 */

#define	FLASH_SDRAM_SECTOR			( SDRAM_SIZE - 0x1a0000 )        				/* 0x260000 -- 0x2A0000 For spiflash */
#define	ITEM_OFFSET					( SDRAM_SIZE - 0x1e0000 )						/* 0x220000 -- 0x260000 For item */
#define	ITEM_DRAM_REPEAT				( SDRAM_SIZE - 0x220000 )						/* 0x1E0000 -- 0x220000 For item */
#define	ITEM_OFFSET_SDRAM			( SDRAM_SIZE - 0x260000 )						/* 0x1A0000 -- 0x1E0000 For item */

#define	KERNEL_OFFSET_TMP				(  0x400000  )        							/*  For spiflash vmlinux.bin.gz sdram unlzma */

//#endif

