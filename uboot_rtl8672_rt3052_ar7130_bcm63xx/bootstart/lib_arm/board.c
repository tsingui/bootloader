/*==============================================================
 #Copyright (c), 1991-2008, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 #Function:解压bootloader的第二部分
 #Designed by xuanguanglei 
 #Date:2008-07-11
 ==============================================================*/

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <version.h>
#include <mkimage.h>
#include <zlib.h>

extern int timer_init(void);
extern  int uncompressLZMA ( Bytef *ptrDest,uLongf *destLen, Bytef *ptrSrc, uLong sourceLen);

/* Begin and End of memory area for malloc(), and current "brk" */
static ulong mem_malloc_start;
static ulong mem_malloc_end;
static ulong mem_malloc_brk;

/* The Malloc area is immediately below the monitor copy in DRAM */
static void mem_malloc_init (void)
{
	ulong dest_addr = CFG_SDRAM_BASE + CFG_SDRAM_SIZE - SDRAM_SIZE + BOOTSTART_OFFSET;

	mem_malloc_end = dest_addr;
	mem_malloc_start = dest_addr - TOTAL_MALLOC_LEN;
	mem_malloc_brk = mem_malloc_start;

	memset ((void *) mem_malloc_start,
		0,
		mem_malloc_end - mem_malloc_start);
	
	alloc_init( CFG_SDRAM_BASE + CFG_SDRAM_SIZE - SDRAM_SIZE + ALLOC_INIT_OFFSET);
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

int	is_lzmadata(bootloader_second_image_hdr_t	*bootloader_second_image_hdr)
{
	if(	bootloader_second_image_hdr->tag[0]=='l' && 
 		bootloader_second_image_hdr->tag[1]=='z' && 
 		bootloader_second_image_hdr->tag[2]=='m' && 
 		bootloader_second_image_hdr->tag[3]=='a'	)
		return 1;
	else
		return 0;
}

void uncompress_bootloader_second ( unsigned int addr )
{
	unsigned int uncompressedLength = 0xFFFFFFFF;
	int ret;
	bootloader_second_image_hdr_t	*bootloader_second_image_hdr ;
	int (*p_image_second)( void );

	timer_init();
	/* initialize malloc() area */
	mem_malloc_init();
	//serial_init();
	
//printf("addr=0x%x\n",addr);

	bootloader_second_image_hdr = ( bootloader_second_image_hdr_t	* )addr;

	if( is_lzmadata( bootloader_second_image_hdr ) )
	{
		//printf("bootloader_second_image_hdr->image_len=0x%x\n",bootloader_second_image_hdr->image_len);
		//printf("bootloader_second_image_hdr->image_load=0x%x\n",bootloader_second_image_hdr->image_load);

		p_image_second = ( void * ) bootloader_second_image_hdr->image_load;	
		ret = uncompressLZMA(bootloader_second_image_hdr->image_load, &uncompressedLength, addr + sizeof( bootloader_second_image_hdr_t ), bootloader_second_image_hdr->image_len);
		if( ret == 0 )
		{
			(*p_image_second)();
		}
		else
		{
			while(1);
		}
	}
	else
	{
		while(1);
	}
}

