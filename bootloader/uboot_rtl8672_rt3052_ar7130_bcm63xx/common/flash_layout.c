/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : flash_layout.c
 文件描述 : flash layout相关操作

说明：当前文件包含很多调试信息，现在用#if 0 #endif形式关闭，这是为调试
			方便，配置操作后面要作改动，所以调试信息暂时保留

 函数列表 :
				is_sysdata
				sysdata_get
				sysdata_save
				item_get
				item_save
				item_transfer
				flash_update_bin
				flash_update_img
				check_addr_null
				short_addr_transfer

 修订记录 :
          1 创建 : 轩光磊
            日期 : 2008-7-22
            描述 :

          2 修订: 轩光磊
             日期: 2008-9-10
             描述: 将配置信息改为条目操作

          3 修订: 轩光磊
             日期: 2008-11-24
             描述: 将SPI类型的读操作改为用映射接口

           4 修订: 轩光磊
             日期: 2009-2-18
             描述: 将更换BIN image的函数移至此文件,并增加CRC校验

           5 修订: 轩光磊
             日期: 2009-3-16
             描述: 优化条目的操作

           6 修订: 轩光磊
             日期: 2009-5-6
             描述: 添加升级时擦除应用层配置

=========================================================================*/

#include <common.h>
#include <command.h>
#include <flash_layout_private.h>
#include <crc.h>
#include <tbs_common.h>

extern void * memcpy(void * dest,const void *src,size_t count);
extern int strcmp(const char * cs,const char * ct);
extern int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

int flash_read_buf(flash_info_t * info, void* addr, unsigned char ** buf, int len);

int  sysdata_get( sys_config_t* syscfg )
{	
#ifdef CONFIG_DOUBLE_BACKUP
	unsigned	int		i = 0;
#endif
	unsigned short	item_len;

	if( item_get( syscfg, LLCONFIG_NAME , &item_len ) == ERROR_ITEM_OK )
	{
#ifdef CONFIG_DOUBLE_BACKUP
		unsigned int image1_start_sector;
		if( syscfg->layout.zone_offset[ZONE_KERNEL_SECOND] == 0 )
		{
			i = 0;
			while( syscfg->layout.zone_offset[ ZONE_KERNEL_FIRST ] >  (info->start[ i ] - info->start[ 0 ] ) )
			{
				i++;	
			}

			image1_start_sector = i;

			while( ( info->start[i] - info->start[image1_start_sector] ) < ( ( info->size - syscfg->layout.zone_offset[ ZONE_KERNEL_FIRST ] ) >> 1 ) )
			{	
				i++;
			}

			syscfg->layout.zone_offset[ ZONE_KERNEL_SECOND] = info->start[i] - info->start[0];
			item_save( syscfg , LLCONFIG_NAME , SYSCFG_SIZE );
		}
		printf("kernel_offset_first=0x%x,rootfs_offset_first=0x%x\n",syscfg->layout.zone_offset[ZONE_KERNEL_FIRST],syscfg->layout.zone_offset[ZONE_ROOTFS_FIRST]);
		printf("kernel_offset_second=0x%x,rootfs_offset_second=0x%x\n",syscfg->layout.zone_offset[ZONE_KERNEL_SECOND],syscfg->layout.zone_offset[ZONE_ROOTFS_SECOND]);
		printf("system supports dual image and ");
#else
		printf("kernel_offset=0x%x,rootfs_offset=0x%x\n",syscfg->layout.zone_offset[ZONE_KERNEL_FIRST],syscfg->layout.zone_offset[ZONE_ROOTFS_FIRST]);
		printf("system supports single image and ");
#endif
		printf("version is %s\n",syscfg->version);
		prepare_tags();

		return 0;
	}
	return 1;
}

int  sysdata_save(sys_config_t * syscfg)
{

	 if( item_save( syscfg , LLCONFIG_NAME , SYSCFG_SIZE ) == ERROR_ITEM_OK )
	 {
	 	return 0;
	 }
	return 1;
}

/*=========================================================================
 Function:		int item_check( void )

 Description:		检查每个条目的完整性,如果不完整做相应的处理
 Data Accessed:
 Data Updated:
 Input:			
 Output:			
 Return:			ERROR_ITEM_OK
				ERROR_CONFIG_LOST
 Others:
=========================================================================*/

int item_check( void )
{
	item_t			*item;
	unsigned short	checksum;
	unsigned int 		i = 0;
	unsigned int		sum = 0;
	unsigned char		*item_sdram;
	char				avail;

	item_sdram = ( unsigned char * )( CFG_SDRAM_BASE + CFG_SDRAM_SIZE - SDRAM_SIZE + ITEM_OFFSET_SDRAM );

	/* 找到Flash上配置区所在扇区的起始地址*/
	while( ( info->start[i] - info->start[0] ) < BOOTCODE_LENGTH )
	{
		i++;
	}

	/* 将配置数据全部读到内存*/
	flash_read_buf( info , ( char * )info->start[i] , &item_sdram , info->start[i+1] - info->start[i] );	

	/*通过检查配置区头部以判断配置区数据是否完好*/
	if( is_ok( ( char * ) item_sdram )  == 0 )
	{
		/*配置区数据不可信,检查中转区数据是否完好*/
#ifdef CONFIG_FLASH_TRANSFER

		/* 将中转区数据全部读到内存*/
		flash_read_buf( info , ( char * )info->start[i+1] , &item_sdram , info->start[i+2] - info->start[i+1] );

		if( is_ok( ( char * ) item_sdram )  == 0 )
		{
			/*中转区数据也不完整,配置丢失,返回错误*/
			printf("The config data has lost!\n");
			
			return ERROR_CONFIG_LOST;
		}
		else
		{
			/*中转区数据完整,修复配置区*/
			/*顺序:擦配置区->  拷贝中转区条目数据到配置区->  置配置区为有效->  擦中转区*/
			flash_erase( info, i , i );
			flash_write( ( char * )( info->start[i+1] + CONFIG_MARK_LEN ) , info->start[i] + CONFIG_MARK_LEN , info->start[i+1] - info->start[i] - CONFIG_MARK_LEN );
			flash_write( CONFIG_MARK ,  info->start[i] , CONFIG_MARK_LEN );
			flash_erase( info, i+1 , i+1 );
		}
#else
		printf("The config data has lost!\n");

		return ERROR_CONFIG_LOST;
#endif
	}

	sum = CONFIG_MARK_LEN;
	
	/*扫描所有条目*/
	while( ( sum + ITEM_HEAD_LEN ) <= ( info->start[i+1] - info->start[i] ) )
	{
		item = ( item_t * ) ( item_sdram + sum );

		if( item->hdr.avail == 0xff )
		{
			if( check_addr_null( ( unsigned int * )item , ITEM_HEAD_LEN ) == 1 )
			{			
				/*检查到条目只写了头部的一部分,置此条目为坏块,并且可以肯定这是最后一个条目*/
				avail = ITEM_BAD;
				flash_write( ( char * )&avail , info->start[i] + ( unsigned int )&( item->hdr.avail ) - ( unsigned int )item_sdram , 0x1 );				
			}
			break;
		}

		/*条目无效,指向下一条目*/
		if( item->hdr.avail == ITEM_UNAVAIL )
		{
			sum += ITEM_SIZE( item->hdr.len );

			continue;
		}

		/*条目已经是坏的,指向下一条目*/
		if( item->hdr.avail == ITEM_BAD )
		{
			sum += ITEM_SIZE( 0 );

			continue;
		}

		/*有效条目*/
		if( item->hdr.avail == ITEM_AVAIL )
		{
			crc16( item->data , item->hdr.len ,&checksum );
			if( checksum == item->hdr.crc )		/*条目完整,指向下一条目*/
			{
				sum += ITEM_SIZE( item->hdr.len );
				
				continue;
			}
			else		/*条目不完整,置此条目为无效,并且可以肯定这是最后一个条目*/
			{
				avail = ITEM_UNAVAIL;
				flash_write( ( char * )&avail , info->start[i] + ( unsigned int )&( item->hdr.avail ) - ( unsigned int )item_sdram , 0x1 );
				break;
			}
		}
	}

	return ERROR_ITEM_OK;
}

/*=========================================================================
 Function:		int item_get( void *data , char *item_name ,unsigned short *len ) 

 Description:		获取指定条目的数据
 Data Accessed:
 Data Updated:
 Input:			*item_name:	 条目名称
 				*data:		条目数据的存放指针
 Output:			*len:		条目有效数据的长度
 Return:			ERROR_ITEM_OK
				ERROR_ITEM_NOT_FIND
 Others:
=========================================================================*/

int item_get( void *data , char *item_name ,unsigned short *len )
{
	item_t			*item;
	unsigned int 		i = 0;
	unsigned int		sum = 0;
	unsigned char		*item_sdram;

	item_sdram = ( unsigned char * )( CFG_SDRAM_BASE + CFG_SDRAM_SIZE - SDRAM_SIZE + ITEM_OFFSET_SDRAM );

	while( ( info->start[i] - info->start[0] ) < BOOTCODE_LENGTH )
	{
		i++;
	}

	flash_read_buf( info , ( char * ) info->start[i] , &item_sdram , info->start[i+1] - info->start[i] );	
	sum = CONFIG_MARK_LEN;
		
	while( ( sum + ITEM_HEAD_LEN ) <= ( info->start[i+1] - info->start[i] ) )
	{
		item = ( item_t * ) ( item_sdram + sum );

		if( item->hdr.avail == 0xff )
		{
			break;
		}

		if( item->hdr.avail == ITEM_UNAVAIL )
		{
			sum += ITEM_SIZE( item->hdr.len );

			continue;
		}

		if( item->hdr.avail == ITEM_BAD )
		{
			sum += ITEM_SIZE( 0 );

			continue;
		}

		if( item->hdr.avail == ITEM_AVAIL )
		{
			if( strcmp( item->data , item_name ) == 0 )
			{
				memcpy( data, item->data + strlen( item_name ) + 1 , item->hdr.len - strlen( item_name ) - 1 );
				*len = item->hdr.len - strlen( item_name ) - 1;

				return ERROR_ITEM_OK;
			}
			
			sum += ITEM_SIZE( item->hdr.len );
				
			continue;
		}		
	}

	return ERROR_ITEM_NOT_FIND;
}

/*=========================================================================
 Function:		int item_save( void *data , char *item_name ,unsigned short len )
 Description:		将配置转化为条目保存
 Data Accessed:
 Data Updated:
 Input:			*item_name:	 条目名称
 				*data:		要存放的条目数据的指针
 				
 Output:			*len:		条目有效数据的长度
 Return:			ERROR_ITEM_OK
 				ERROR_ITEM_BIG				
 Others:
=========================================================================*/

int item_save( void *data , char *item_name ,unsigned short len )
{
	item_t			*item;
	item_t			*item_data;
	unsigned int 		i = 0;
	unsigned int 		j = 0;
	unsigned int		sum = 0;
	unsigned short	checksum;	
	char				avail;
	unsigned char		*item_sdram;
	unsigned int		item_repeat_addr[2];
	
	item_sdram = ( unsigned char * )( CFG_SDRAM_BASE + CFG_SDRAM_SIZE - SDRAM_SIZE + ITEM_OFFSET_SDRAM );

	while( ( info->start[i] - info->start[0] ) < BOOTCODE_LENGTH )
	{
		i++;
	}

	flash_read_buf( info , ( char * ) info->start[i] , &item_sdram , info->start[i+1] - info->start[i] );	
	sum = CONFIG_MARK_LEN;
		
	while( ( sum + ITEM_HEAD_LEN ) <= ( info->start[i+1] - info->start[i] ) )
	{
		item = ( item_t * ) ( item_sdram + sum );
		
		if( item->hdr.avail == 0xff )	/*找到FLASH空地址*/
		{
			break;
		}

		if( ( item->hdr.avail == ITEM_UNAVAIL ) )
		{
			sum += ITEM_SIZE( item->hdr.len );

			continue;
		}

		if( item->hdr.avail == ITEM_BAD )
		{
			sum += ITEM_SIZE( 0 );

			continue;
		}

		if( item->hdr.avail == ITEM_AVAIL )
		{
			/*发现将要置为无效的条目,并记下它的地址*/
			if( strcmp( item->data , item_name ) == 0 )
			{
				item_repeat_addr[j++] = info->start[i] + ( unsigned int )&( item->hdr.avail ) - ( unsigned int )item_sdram;

				/*发现两个重复的条目,将前一个条目置为无效*/
				if( j == 2 )
				{
					avail = ITEM_UNAVAIL;
					flash_write( ( char * )&avail , item_repeat_addr[0] , 0x1 );
					item_repeat_addr[0] = item_repeat_addr[1];
					j = 1;
				}
			}
			
			sum += ITEM_SIZE( item->hdr.len );

			continue;
		}		
	}

	/*合成新条目*/
	item_data = ( item_t * )( CFG_SDRAM_BASE + CFG_SDRAM_SIZE - SDRAM_SIZE + ITEM_OFFSET );

	item_data->hdr.avail = ITEM_AVAIL;	
	memcpy( item_data->data , item_name , strlen( item_name ) );
	memcpy( item_data->data + strlen( item_name ) , "\0" , 1 );	
	memcpy( item_data->data + strlen( item_name ) + 1 , data , len );	
	item_data->hdr.len = strlen( item_name ) + 1 + len;
	crc16( item_data->data , item_data->hdr.len ,&checksum );
	item_data->hdr.crc = checksum;

	/*判断新条目的大小是否超出配置区的剩余空间*/
	if( ITEM_SIZE( item_data->hdr.len ) > ( info->start[i+1] - info->start[i] - sum ) )
	{
		/*是,通过中转的方式将配置区中的无效和坏条目丢掉*/
		item_repeat();

		flash_read_buf( info , ( char * ) info->start[i] , &item_sdram , info->start[i+1] - info->start[i] );	
		sum = CONFIG_MARK_LEN;		
		j = 0;
		
		while( ( sum + ITEM_HEAD_LEN ) <= ( info->start[i+1] - info->start[i] ) )
		{
			item = ( item_t * ) ( item_sdram + sum );
			
			if( item->hdr.avail == 0xff )	/*找到FLASH空地址*/
			{
				break;
			}

			if( item->hdr.avail == ITEM_AVAIL )
			{
				/*发现将要置为无效的条目,并记下它的地址*/
				if( strcmp( item->data , item_name ) ==0 )
				{
					item_repeat_addr[j++] = info->start[i] + ( unsigned int )&( item->hdr.avail ) - ( unsigned int )item_sdram;
				}
				
				sum += ITEM_SIZE( item->hdr.len );
				
				continue;
			}			
		}
		
		/*再次判断是否超出空间*/
		if( ITEM_SIZE( item_data->hdr.len ) > ( info->start[i+1] - info->start[i] - sum ) )
		{
			/*条目太大,无法保存,返回错误*/
			return ERROR_ITEM_BIG;
		}
	}

	flash_write( ( unsigned char * ) item_data , info->start[i] + ( unsigned int )item - ( unsigned int )item_sdram , ITEM_HEAD_LEN + item_data->hdr.len );
	if( j == 1 )
	{
		avail = ITEM_UNAVAIL;
		flash_write( ( char * )&avail , item_repeat_addr[0] , 0x1 );
	}

	return ERROR_ITEM_OK;
}

/*=========================================================================
 Function:		int item_repeat( void )

 Description:		当配置区空间不足时通过中转区中转数据，中转区可以是
 				flash一个扇区或内存
 Data Accessed:
 Data Updated:
 Input:			无
 				 				
 Output:			无
 Return:			ERROR_ITEM_OK
				
 Others:
=========================================================================*/
int item_repeat( void )
{
	item_t			*item;
	unsigned int 		i = 0;
	unsigned int		sum = 0;
	unsigned int		len = 0;
	unsigned char		*item_sdram;
	unsigned int		addr;
	
	item_sdram = ( unsigned char * )( CFG_SDRAM_BASE + CFG_SDRAM_SIZE - SDRAM_SIZE + ITEM_OFFSET_SDRAM );

	while( ( info->start[i] - info->start[0] ) < BOOTCODE_LENGTH )
	{
		i++;
	}

#ifdef CONFIG_FLASH_TRANSFER
	flash_erase( info, i+1 , i+1 );
	addr = info->start[i+1] + CONFIG_MARK_LEN;
#else
	addr = CFG_SDRAM_BASE + CFG_SDRAM_SIZE - SDRAM_SIZE + ITEM_DRAM_REPEAT;
#endif

	flash_read_buf( info , ( char * ) info->start[i] , &item_sdram , info->start[i+1] - info->start[i] );	
	sum = CONFIG_MARK_LEN;
	
	while( ( sum + ITEM_HEAD_LEN ) <= ( info->start[i+1] - info->start[i] ) )
	{
		item = ( item_t * ) ( item_sdram + sum );
		
		if( item->hdr.avail == 0xff )
		{
#ifdef CONFIG_FLASH_TRANSFER
			flash_write( CONFIG_MARK ,  info->start[i+1] , CONFIG_MARK_LEN );
#endif
			break;
		}

		if( ( item->hdr.avail == ITEM_UNAVAIL ) )
		{
			sum += ITEM_SIZE( item->hdr.len );

			continue;
		}

		if( item->hdr.avail == ITEM_BAD )
		{
			sum += ITEM_SIZE( 0 );

			continue;
		}

		if( item->hdr.avail == ITEM_AVAIL )
		{
#ifdef CONFIG_FLASH_TRANSFER
			flash_write( ( char * )item , addr , ITEM_SIZE( item->hdr.len ) );
#else
			memcpy( ( unsigned int * )addr ,  ( unsigned char * ) item , ITEM_SIZE( item->hdr.len ) );
#endif
			sum += ITEM_SIZE( item->hdr.len );
			addr += ITEM_SIZE( item->hdr.len );
			len += ITEM_SIZE( item->hdr.len );
			
			continue;
		}
	}

	flash_erase( info, i , i );
#ifdef CONFIG_FLASH_TRANSFER	
	flash_write( ( char * )( info->start[i+1] + CONFIG_MARK_LEN ) , info->start[i] + CONFIG_MARK_LEN , len );	
#else
	addr = CFG_SDRAM_BASE + CFG_SDRAM_SIZE - SDRAM_SIZE + ITEM_DRAM_REPEAT;
	flash_write( ( char * )addr , info->start[i] + CONFIG_MARK_LEN , len );
#endif
	flash_write( CONFIG_MARK ,  info->start[i] , CONFIG_MARK_LEN );

	return ERROR_ITEM_REPEAT_OK;
}

/*=========================================================================
 Function:		int flash_update_bin( unsigned int file_size , char force_mark )
 Description:		对系统进行升级
 				
 Calls:			tbs_calc_sum_addr
 				flash_erase
 				flash_write
 				
 Data Accessed:
 Data Updated:
 Input:			file_size:升级image大小
 Output:			无
 Return:			ERROR_CRC:		crc校验错误,升级image遭到破坏
				ERROR_PRODUCT:	升级image非本产品
				ERROR_BIG:		image大于升级所允许的空间
				ERROR_OK:		升级成功
 Others:
=========================================================================*/

int flash_update_bin( unsigned int file_size ,char force_mark )
{
	int block ;
	int len = 0 ;
	int end = 0 ;
	image_bin_tail_t 	*image_bin_tail;
	unsigned long checksum;
	unsigned int bin_file_checksum;

	if( file_size > info->size )
	{
		printf("update failure! Your image is too long !\n");

		return ERROR_BIG;
	}
	if( force_mark == 0 )
	{
		bin_file_checksum = *( unsigned int * )( load_addr + file_size - 4 );	
		tbs_calc_sum_addr( ( unsigned char * ) load_addr , &checksum , file_size - 4 );

		if( checksum != bin_file_checksum )
		{
			printf("Calculated checksum is %x\n", checksum);
			printf("Checksum validation failed! you should translate image again!\n");

			return ERROR_CRC;
		}

		image_bin_tail = ( image_bin_tail_t * )( load_addr + file_size - 4 - sizeof( image_bin_tail_t ) );
		if( strcmp( PRODUCT, image_bin_tail->product ) != 0 )
		{
			printf("Update failure !\nSystem is %s ,but you translate a %s image!\n", PRODUCT,image_bin_tail->product );
			printf("Please translate a %s BIN file!\n",PRODUCT);

			return ERROR_PRODUCT;
		}
	}

	printf("Erasing flash");
	flash_erase(  info , 0 , info->sector_count - 1 );
	printf("Burning flash");

  	while( len < file_size )
  	{
		if( end == ( info->sector_count - 1 ) )
		{
			block = info->start[0] + info->size - info->start[end];
		}
		else
		{
			block = info->start[end+1] - info->start[end];
		}
		flash_write( ( char * ) load_addr ,  info->start[end] , block );		
		len += block;
		load_addr += block;
		end++;
		printf(".");
	}
	printf("\n");
	printf("System update completely! Restarting system!\n");
	do_reset(NULL, 0, 0, NULL);

	return ERROR_OK;
}

/*=========================================================================
 Function:		int flash_update_img(unsigned char * data, sys_config_t* sys_data,unsigned int *file_size)

 Description:		对系统进行升级
 				
 Calls:			tbs_calc_sum_addr
 				flash_erase
 				flash_write
 				sysdata_save
 Data Accessed:
 Data Updated:
 Input:			data:	升级image数据
 				sys_data:底层配置信息
				*file_size:升级image大小
 Output:			无
 Return:			ERROR_CRC:		crc校验错误,升级image遭到破坏
				ERROR_PRODUCT:	升级image非本产品
				ERROR_NIMG:		不是升级image
				ERROR_BIG:		image大于升级所允许的空间
				ERROR_OK:		升级成功
 Others:
=========================================================================*/

int flash_update_img( unsigned char * data , sys_config_t*  sys_data , unsigned int file_size )
{
	update_hdr_t * update_hdr = ( update_hdr_t * ) data;

	unsigned int start = 0;
	unsigned int end = 0;
	int len = 0;
	int block = 0;
	unsigned long checksum ;
	unsigned int img_file_checksum;
#ifdef CONFIG_DOUBLE_BACKUP
	unsigned int image1_start_sector;
#endif

/*=========================================================================
		对image作CRC校验
=========================================================================*/
	img_file_checksum = *( unsigned int * )( load_addr + file_size - 4 );
	
	tbs_calc_sum_addr( ( unsigned char * ) load_addr , &checksum , file_size - 4 );

	if( checksum != img_file_checksum )
	{
		printf("Calculated checksum is %x\n", checksum);
		printf("Checksum validation failed! you should translate image again!\n");

		return ERROR_CRC;
	}

	data += update_hdr->kernel_offset;

/*=========================================================================
		对image头部作合法性检查
=========================================================================*/

	if( strcmp( PRODUCT, update_hdr->product ) != 0 )
	{
		printf("Update failure !\nSystem is %s ,but you download a %s image!\n", PRODUCT,update_hdr->product );
		printf("Please translate a %s IMG file!\n",PRODUCT);

		return ERROR_PRODUCT;
	}

	if( strcmp( update_hdr->img_type, IMAGE_TYPES ) != 0 )
	{
		printf("Update failure !\nSystem is %s ,but you down a %s image!\n", IMAGE_TYPES,update_hdr->img_type );
		printf("Please translate a %s IMG file!\n",IMAGE_TYPES);

		return ERROR_IMG_TYPE;
	}

	debug("kernel_offset=%x\nrootfs_offset=%x\n",update_hdr->kernel_offset,update_hdr->rootfs_offset);

	if( !is_sysdata( sys_data ) )
	{
		memset( sys_data , 0 , sizeof( sys_config_t ) );
#ifdef __BIG_ENDIAN
		sys_data->ip= 0xc0a80101;
#else	
		sys_data->ip = 0x101a8c0;
#endif
		sys_data->mac[0] = 0x00;
		sys_data->mac[1] = 0x02;
		sys_data->mac[2] = 0x03;
		sys_data->mac[3] = 0x04;
		sys_data->mac[4] = 0x05;
		sys_data->mac[5] = 0x06;

		strcpy( sys_data->product, update_hdr->product);
		strcpy( sys_data->tag , "sysc");
		
		start = 0;
		while( ( info->start[start] - info->start[0] ) < BOOTCODE_LENGTH )
			start++;
		
#ifdef CONFIG_FLASH_TRANSFER
		flash_erase( info , start , start + 1 );
		start += 2;
#else
		flash_erase( info , start , start );
		start += 1;
#endif
		sys_data->layout.zone_offset[ZONE_KERNEL_FIRST] = info->start[start] - info->start[0];

#ifdef CONFIG_DOUBLE_BACKUP
		sys_data->image_mark = 1;

		image1_start_sector = start;

		while( ( info->start[start] - info->start[image1_start_sector] ) < ( ( info->size - sys_data->layout.zone_offset[ ZONE_KERNEL_FIRST ] ) >> 1 ) )
		{	
			start++;
		}

		sys_data->layout.zone_offset[ ZONE_KERNEL_SECOND] = info->start[start] - info->start[0];
#endif

	}

	strcpy( sys_data->version , update_hdr->version );
	strcpy( sys_data->board_id, update_hdr->board_id);

#ifdef CONFIG_DOUBLE_BACKUP
	if( update_hdr->image_len > ( info->size - sys_data->layout.zone_offset[ ZONE_KERNEL_SECOND] ) ) 
	{
		printf("update failure! Because your update image is too long !\n");

		return ERROR_BIG;
	}
	start = 0;
	if( sys_data->image_mark == 0 )
	{
		while( sys_data->layout.zone_offset[ZONE_KERNEL_SECOND] > ( info->start[start] - info->start[0] ) )
		{
			start++;
		}
	}
	else
	{
		while( sys_data->layout.zone_offset[ZONE_KERNEL_FIRST] > ( info->start[start] - info->start[0] ) )
		{
			start++;
		}		
	}

	end=start;
	len=0;

	while( update_hdr->image_len > ( info->start[end]  -info->start[start] ) )
	{
		if( end == ( info->sector_count - 1 ) )
		{
			break;
		}

		end++;
	}

	printf("Erasing flash");
	flash_erase( info , start , end );
	printf("Burning flash");

	while( len < update_hdr->image_len )
  	{
		if( start == ( info->sector_count - 1 ) )
		{
			block = info->start[0] + info->size - info->start[start];
		}
		else
		{
			block = info->start[start+1] - info->start[start];
		}	
  		
		flash_write( ( char * ) data ,  info->start[start] , block );		
		len += block;
		data += block;
		start++;
		printf(".");
	}
	printf("\n");

	if( sys_data->image_mark == 0 )
	{
		sys_data->layout.zone_offset[ZONE_ROOTFS_SECOND] = sys_data->layout.zone_offset[ZONE_KERNEL_SECOND] + update_hdr->rootfs_offset - update_hdr->kernel_offset ;
		sys_data->second_image_checksum = update_hdr->image_checksum;
		sys_data->second_image_len = update_hdr->image_len;
		sys_data->image_mark =1;
	}
	else
	{
		sys_data->layout.zone_offset[ZONE_ROOTFS_FIRST] = sys_data->layout.zone_offset[ZONE_KERNEL_FIRST] + update_hdr->rootfs_offset - update_hdr->kernel_offset ;
		sys_data->first_image_checksum = update_hdr->image_checksum;
		sys_data->first_image_len = update_hdr->image_len;
		sys_data->image_mark = 0;
	}	
	
#else
	if( update_hdr->image_len > ( info->size - sys_data->layout.zone_offset[ZONE_KERNEL_FIRST] ) ) 
	{
		printf("update failure! Because your update image is too long !\n");

		return ERROR_BIG;
	}
	
	start = 0;
	while( sys_data->layout.zone_offset[ZONE_KERNEL_FIRST] > ( info->start[start] - info->start[0] ) )
	{
		start++;
	}		

	end = start;
	len = 0;

	while( update_hdr->image_len > ( info->start[end]  -info->start[start] ) )
	{
		if( end == ( info->sector_count - 1 ) )
		{
			break;
		}

		end++;
	}		

	printf("Erasing flash");
	flash_erase( info , start , end );
	printf("Burning flash");

	while( len < update_hdr->image_len )
	{
		if( start == ( info->sector_count - 1 ) )
		{
			block = info->start[0] + info->size - info->start[start];
		}
		else
		{
			block = info->start[start+1] - info->start[start];
		}	
  		
		flash_write( ( char * ) data ,  info->start[start] , block );		
		len += block;
		data += block;
		start++;
		printf(".");
	}
	printf("\n");

	sys_data->layout.zone_offset[ZONE_ROOTFS_FIRST] = sys_data->layout.zone_offset[ZONE_KERNEL_FIRST] + update_hdr->rootfs_offset - update_hdr->kernel_offset ;
	sys_data->first_image_checksum = update_hdr->image_checksum;
	sys_data->first_image_len = update_hdr->image_len;
#endif

	item_save( "clear", TBS_APP_CFG ,5 ); //clear app config
	sysdata_save(sys_data);  
	printf("System update completely! Restarting system!\n");
	do_reset(NULL, 0, 0, NULL);
	
	return ERROR_OK;
}

