#include <common.h>
#include <malloc.h>
#include "spi_flash.h"


flash_info_t flash_info[CFG_MAX_FLASH_BANKS];           /* FLASH chips info */
flash_info_t * info;

//tylo, for ic ver. detect
unsigned char ICver=0;

/* SPI Flash Controller */
unsigned int SFCR=0;
unsigned int SFCSR=0;
unsigned int SFDR=0;


static void spi_ready(void)
{
	while (1)
	{
		if ( (*(volatile unsigned int *) SFCSR) & READY(1))
		break;
	}
}


/*
 * This function shall be called when switching from MMIO to PIO mode
 */
static void spi_pio_init(void)
{
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);

	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);

	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}


static void checkICver(void)
{
	unsigned int sccr;

	sccr=*(volatile unsigned int*)SCCR;
	if((sccr & 0x00100000) == 0)
	{
		ICver = IC8671B_costdown;
	}
	else
	{
		ICver = IC8671B;
	}	

	//set SPI related register
	if(ICver == IC8671B)
	{
		SFCR = 0xB8001200;
		SFCSR= 0xB8001204;
		SFDR = 0xB8001208;
	}
	else if(ICver == IC8671B_costdown)
	{ // 8671B costdown
		SFCR = 0xB8001200;
		SFCSR = 0xB8001208;
		SFDR = 0xB800120C;
	//move to loader.c, ethernet_patch() -shlee		*(volatile unsigned int *) 0xb8003304 = 0x00000008;
	}
	else
	{
		printf("can not detect spi controller!");
	}
}

static void read_id(spi_flash_id * flash_id)
{
	unsigned int temp;
	int cnt =0;

	checkICver();

	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR |SPI_CLK_DIV(1);//
	spi_pio_init();      


	/* Here set the default setting */
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	//printf("\n");
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);

	/* One More Toggle (May not Necessary) */
	// *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	// *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);

	/* RDID Command */
	*(volatile unsigned int *) SFDR = 0x9F << 24;
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+cnt) | READY(1);
	temp = *(volatile unsigned int *) SFDR;

	flash_id->device_id = (temp >> 8) & 0xFFFFFF;
	flash_id->ext_device_id= 0;
	//spi_flash_info[cnt].maker_id = (temp >> 24) & 0xFF;
	//spi_flash_info[cnt].type_id = (temp >> 16) & 0xFF;
	//spi_flash_info[cnt].capacity_id = (temp >> 8) & 0xFF;

	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	spi_ready();	  
}

static void spi_erase_sector(int sector)
{
	int chip=0;

	spi_pio_init();

	/* WREN Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

	*(volatile unsigned int *) SFDR = 0x06 << 24;
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* SE Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
	*(volatile unsigned int *) SFDR = (0xD8 << 24) | (sector * 65536);
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

	/* RDSR Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
	*(volatile unsigned int *) SFDR = 0x05 << 24;

	while (1)
	{
		/* RDSR Command */
		if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
		{
			break;
		}
	}

	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
}

static void spi_unprotect_block_other(unsigned int chip)
{
	spi_pio_init();

	/* De-select Chip */
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* RDSR Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
	*(volatile unsigned int *) SFDR = 0x05 << 24;

	while (1)
	{
		unsigned int status;
		status = *(volatile unsigned int *) SFDR;

		/* RDSR Command */
		if ( (status & 0x01000000) == 0x00000000)
		{
			//ql: if block protected, then write status register 0
			if (status & (0x1C<<24))
			{
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				/* WREN Command */
				spi_ready();
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
				*(volatile unsigned int *) SFDR = 0x06 << 24;
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
				/*EWSR Command*/
				spi_ready();
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
				*(volatile unsigned int *) SFDR = 0x50 << 24;
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

				spi_ready();
				//WRSR command
				*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
				*(volatile unsigned int *) SFDR = (0x01 << 24);
				*(volatile unsigned int *) SFDR = 0;
			}

			break;
		}
	}

	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
}

static void rtl8672_spi_write_page(unsigned int addr, unsigned char *data, int len)
{
	unsigned char *cur_addr;
	int cur_size;
	int chip;
	int k;
	unsigned int temp;
	unsigned int cnt;

	chip = RTL8672_FLASH_CHIP;
	cur_addr = data;
	cur_size = len;

	spi_unprotect_block_other(chip);

	/* WREN Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
	*(volatile unsigned int *) SFDR = 0x06 << 24;
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* PP Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
	*(volatile unsigned int *) SFDR = (0x02 << 24) | addr;

	for (k = 0; k < 64; k++)
	{
		temp = (*(cur_addr)) << 24 | (*(cur_addr + 1)) << 16 | (*(cur_addr + 2)) << 8 | (*(cur_addr + 3));

		spi_ready();
		if (cur_size >= 4)
		{
			*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
			cur_size -= 4;
		}
		else
		{
			*(volatile unsigned int *) SFCSR = LENGTH(cur_size-1) | CS(1+chip) | READY(1);
			cur_size = 0;
		}

		*(volatile unsigned int *) SFDR = temp;

		cur_addr += 4;

		if (cur_size == 0)
			break;
	}

	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

	/* RDSR Command */
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
	*(volatile unsigned int *) SFDR = 0x05 << 24;

	cnt = 0;
	while (1)
	{
		unsigned int status = *(volatile unsigned int *) SFDR;

		/* RDSR Command */
		if ((status & 0x01000000) == 0x00000000)
		{
			break;
		}

		if (cnt > 200000)
		{
			printf("\nBusy Loop for RSDR: %d, Address at 0x%08X\n", status, addr);
			busy:
			goto busy;
		}
		cnt++;
	}
	
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

}

#if 0
int spi_program_image(unsigned int chip,unsigned long flash_address ,unsigned char *image_addr, unsigned int image_size)
{
	unsigned int temp;
	unsigned int i, j, k;
	unsigned char *cur_addr;
	unsigned int cur_size;
	unsigned int cnt;
	unsigned int sect_cnt;
	unsigned long uladdr;

	flash_address -= CFG_FLASH_BASE;
	cur_addr = image_addr;
	cur_size = image_size;

	if (flash_address & ((1 << 16) -1))
	{
		printf("\r\nflash_address must be 64KB aligned!!");
		return 1;
	}

	printf("spi_program_image :flash_address :%x  image_addr:%x, image_size :%d",
		flash_address, image_addr, image_size);


	sect_cnt = flash_address >> 16; //flash_address must be 64KB aligned
	spi_unprotect_block_other(chip);
	/* Iterate Each Sector */
	for (i = sect_cnt; i < info->sector_count; i++)
	{
		unsigned int spi_data;
		printf(".");

		/* Iterate Each Page,  64 KB = 256(page size)*256(page number) */
		for (j = 0; j < 256; j++)
		{
			if (cur_size == 0)
				break;

			/* WREN Command */
			spi_ready();
			*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
			*(volatile unsigned int *) SFDR = 0x06 << 24;
			*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

			/* PP Command */
			spi_ready();
			*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
			*(volatile unsigned int *) SFDR = (0x02 << 24) | (i * 65536) | (j * 256);

			for (k = 0; k < 64; k++)
			{
				temp = (*(cur_addr)) << 24 | (*(cur_addr + 1)) << 16 | (*(cur_addr + 2)) << 8 | (*(cur_addr + 3));

				spi_ready();
				if (cur_size >= 4)
				{
					*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
					cur_size -= 4;
				}
				else
				{
					*(volatile unsigned int *) SFCSR = LENGTH(cur_size-1) | CS(1+chip) | READY(1);
					cur_size = 0;
				}

				*(volatile unsigned int *) SFDR = temp;

				cur_addr += 4;

				if (cur_size == 0)
					break;
			}

			*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

			/* RDSR Command */
			spi_ready();
			*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
			*(volatile unsigned int *) SFDR = 0x05 << 24;

			cnt = 0;
			while (1)
			{
				unsigned int status = *(volatile unsigned int *) SFDR;

				/* RDSR Command */
				if ((status & 0x01000000) == 0x00000000)
				{
					break;
				}

				if (cnt > 200000)
				{
					printf("\nBusy Loop for RSDR: %d, Address at 0x%08X\n", status, i*65536+j*256);
					busy:
					goto busy;
				}
				cnt++;
			}
			*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
		}

		if (cur_size == 0)
		break;
	} /* Iterate Each Sector */

	return 0;
}
#endif

//----------------------------------------------------------------------------
// Program Buffer
// 0. Assumption: Caller has already erased the appropriate sectors.
// 1. call page programming for every 256 bytes

static int flash_program_buf(void* addr, void* data, int len)
{
	int total = 0, len_this_lp, bytes_this_page;
	unsigned long dst;
	unsigned char *src;
	unsigned long uladdr = (unsigned long)addr - CFG_FLASH_BASE;

	while(total < len) 
	{
		src              = (unsigned char*)data + total;
		dst              = uladdr + total;
		bytes_this_page  = RTL8672_SPI_PAGE_SIZE - ((unsigned long)dst % RTL8672_SPI_PAGE_SIZE);
		len_this_lp      = ((len - total) > bytes_this_page) ? bytes_this_page: (len - total);

		debug("src=0x%x,dst=0x%x,len_this_lp=0x%x,bytes_this_page=0x%x\n",src,dst,len_this_lp,bytes_this_page);

		rtl8672_spi_write_page(dst, src, len_this_lp);
		total += len_this_lp;
	}

	return 0;
}

int flash_read_buf(flash_info_t * info, void* addr, unsigned char ** buf, int len)
{
	unsigned char *buf_tmp ;
	unsigned char *addr_tmp ;

	if( len  > ( CFG_FLASH_BASE + info->size  - ( ulong )addr ) )
	{
		printf("The length is out of flash space.\n");
		return 0;
	}

	buf_tmp = *buf;
	addr_tmp = addr;
	memcpy( buf_tmp , addr_tmp , len);

	return len;	
}


/*-----------------------------------------------------------------------
 * Copy memory to flash.
 * Make sure all target addresses are within Flash bounds,
 * and no protected sectors are hit.
 * Returns:
 * ERR_OK          0 - OK
 * ERR_TIMOUT      1 - write timeout
 * ERR_NOT_ERASED  2 - Flash not erased
 * ERR_PROTECTED   4 - target range includes protected sectors
 * ERR_INVAL       8 - target address not in Flash memory
 * ERR_ALIGN       16 - target address not aligned on boundary
 *                      (only some targets require alignment)
 */
int flash_write (char *src, ulong addr, ulong cnt)
{

	int i;
	ulong         end        = addr + cnt - 1;
	flash_info_t *info_first = addr2info (addr);
	flash_info_t *info_last  = addr2info (end );
	flash_info_t *info;

	if (cnt == 0) 
	{
		return (ERR_OK);
	}

	if (!info_first || !info_last) 
	{
		return (ERR_INVAL);
	}

	for (info = info_first; info <= info_last; ++info) 
	{
		ulong b_end = info->start[0] + info->size;      /* bank end addr */
		short s_end = info->sector_count - 1;
		
		for (i=0; i<info->sector_count; ++i) 
		{
			ulong e_addr = (i == s_end) ? b_end : info->start[i + 1];

			if ((end >= info->start[i]) && (addr < e_addr) &&(info->protect[i] != 0) ) 
			{
				return (ERR_PROTECTED);
			}
		}
	}

	for (info = info_first; info <= info_last && cnt>0; ++info)
	{
		ulong len;
		len = info->start[0] + info->size - addr;

		if (len > cnt)
			len = cnt;
		/*-----------------------------------------------------------------------
		  * FUNCTION:flash_program_buf
		  * Copy memory to flash, returns:
		  * 0 - OK
		  * 1 - write timeout
		  * 2 - Flash not erased
		  */

		if( ( ( unsigned int ) src >= info_first->start[0] ) && ( ( unsigned int ) src <= info_first->start[0] + info_first->size ) )
		{
			unsigned char *p;
			unsigned int i,j,tmp,addr_tmp,src_tmp;
		
			addr_tmp=addr;
			src_tmp = ( unsigned int ) src;
			for(i=0;i<(cnt>>10);i++)
			{
				p=malloc(1024);
				flash_read_buf( info , src , &p , 1024 );
				if ( ( j = flash_program_buf( ( unsigned char *) addr , p , 1024 ) ) != 0) 
				{
					return (j);
				}				
				src+=1024;
				addr+=1024;
				free(p);
			}

			if(len%1024!=0)
			{
				tmp=len%1024;
				p=malloc(tmp);
				flash_read_buf( info , src ,  &p , tmp );
				if ((j = flash_program_buf( ( unsigned char * ) addr , p , tmp )) != 0) 
				{
					return (j);
				}
				free(p);
			}

			addr = addr_tmp;
			src = (char *) src_tmp;
		}
		else
		{
			if ( ( i = flash_program_buf( ( unsigned char * ) addr , src , cnt ) ) != 0) 
			{
				return (i);
			}
		}
		
		cnt  -= len;
		addr += len;
		src  += len;
        }

        return (ERR_OK);
}

void flash_print_info (flash_info_t * info)
{
	int i;

	printf ("  Spi flash ID: 0x%x\n", info->flash_id);
	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);
	puts ("  Sector Start Addresses:");
	
	for (i = 0; i < info->sector_count; ++i) 
	{
		if ((i % 5) == 0)
				printf ("\n   ");
		printf (" %08lX%s",
		info->start[i], info->protect[i] ? " (RO)" : "     ");

	}
	
	putc ('\n');
	return;
	
}

int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int i = s_first;

	do 
	{
		printf(".");
		spi_erase_sector(i);
	}while (i++ < s_last);

	printf("done\n");

	return 0;
}


flash_info_t * addr2info (ulong addr)
{
	flash_info_t *info;
	int i;

	for (i=0, info=&flash_info[0]; i<CFG_MAX_FLASH_BANKS; ++i, ++info) 
	{
		if (info->flash_id != FLASH_UNKNOWN && addr >= info->start[0] &&
			/* WARNING - The '- 1' is needed if the flash
			  * is at the end of the address space, since
			  * info->start[0] + info->size wraps back to 0.
			  * Please don't change this unless you understand this.
			  */
			addr <= info->start[0] + info->size - 1) 
			{
				return (info);
			}
	}

	return (NULL);
}


/* adapter for TBS bootloader */

unsigned long flash_init (void)
{
	spi_flash_id  flash_id;
	int i;

	read_id(&flash_id);
	
	if(flash_id.device_id == 0x12018)			/*spansion spi flash:S25FL128P */
	{
		if(flash_id.ext_device_id == 0x300)		/* 256k byte sector */
		{
			flash_info[0].size = 0x40000 * 64;
			flash_info[0].sector_count = 64;
			flash_info[0].flash_id = flash_id.device_id;
			debug("Found flash 25FL128P 256K sector!\n");
		}
		else if(flash_id.ext_device_id == 0x301)	/* 64k byte sector */
		{
			flash_info[0].size = 0x10000 * 256;
			flash_info[0].sector_count = 256;
			flash_info[0].flash_id = flash_id.device_id;
			debug("Found flash 25FL128P 64K sector!\n");
		}
		else
		{
			flash_info[0].size = 0;
			flash_info[0].sector_count = 0;
			flash_info[0].flash_id = FLASH_UNKNOWN;
			debug("Unknow extend flash ID:0x%x.\n",flash_id.ext_device_id);
		}
	}
	else if(flash_id.device_id == 0x10216)			/*spansion spi flash:S25FL64A */
	{
		flash_info[0].size = 0x10000 * 128;
		flash_info[0].sector_count = 128;
		flash_info[0].flash_id = flash_id.device_id;
		debug("Found flash SPANSION S25FL64A:8M size,64K sector!\n");
	}
	else if(flash_id.device_id == 0xc22018)			/*Mx spi flash:MX25L128AD */
	{
		flash_info[0].size = 0x10000 * 256;
		flash_info[0].sector_count = 256;
		flash_info[0].flash_id = flash_id.device_id;
		debug("Found flash MX25L12805d:16M size,64K sector!\n");
	}
	else if(flash_id.device_id == 0x202018)			/*Mx spi flash:M25P128 */
	{
		flash_info[0].size = 0x40000 * 64;
		flash_info[0].sector_count = 64;
		flash_info[0].flash_id = flash_id.device_id;
		debug("Found flash M25P128:16M size,256K sector!\n");
	}
	else if(flash_id.device_id == 0xc22015)			/*Mx spi flash:M25L1605D*/
	{
		flash_info[0].size = 0x10000 * 32;
		flash_info[0].sector_count = 32;
		flash_info[0].flash_id = flash_id.device_id;
		debug("Found flash M25L1605D:2M size,64K sector!\n");
	}
	else if(flash_id.device_id == 0xc22016)			/*Mx spi flash:M25L3205D*/
	{
		flash_info[0].size = 0x10000 * 64;
		flash_info[0].sector_count = 64;
		flash_info[0].flash_id = flash_id.device_id;
		debug("Found flash M25L3205D:4M size,64K sector!\n");
	}
	else if(flash_id.device_id == 0xc22017)			/*Mx spi flash:M25L6405D*/
	{
		flash_info[0].size = 0x10000 * 128;
		flash_info[0].sector_count = 128;
		flash_info[0].flash_id = flash_id.device_id;
		debug("Found flash M25L6405D:8M size,64K sector!\n");
	}
	else
	{
		flash_info[0].size = 0;
		flash_info[0].sector_count = 0;
		flash_info[0].flash_id = FLASH_UNKNOWN;
		debug("Unknow flash id 0x%x!\n",flash_id.device_id);
		return 0;
	}

	for(i=0;i < flash_info[0].sector_count;i++)
	{
		flash_info[0].start[i] = flash_info[0].size /flash_info[0].sector_count *  i  + CFG_FLASH_BASE;
		flash_info[0].protect[i] = flash_info[0].start[i];
	}

	info=&flash_info[0];
	
	return flash_info[0].size;
}

