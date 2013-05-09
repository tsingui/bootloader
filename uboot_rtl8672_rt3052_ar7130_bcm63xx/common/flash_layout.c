/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : flash_layout.c
 �ļ����� : flash layout��ز���

˵������ǰ�ļ������ܶ������Ϣ��������#if 0 #endif��ʽ�رգ�����Ϊ����
			���㣬���ò�������Ҫ���Ķ������Ե�����Ϣ��ʱ����

 �����б� :
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

 �޶���¼ :
          1 ���� : ������
            ���� : 2008-7-22
            ���� :

          2 �޶�: ������
             ����: 2008-9-10
             ����: ��������Ϣ��Ϊ��Ŀ����

          3 �޶�: ������
             ����: 2008-11-24
             ����: ��SPI���͵Ķ�������Ϊ��ӳ��ӿ�

           4 �޶�: ������
             ����: 2009-2-18
             ����: ������BIN image�ĺ����������ļ�,������CRCУ��

           5 �޶�: ������
             ����: 2009-3-16
             ����: �Ż���Ŀ�Ĳ���

           6 �޶�: ������
             ����: 2009-5-6
             ����: �������ʱ����Ӧ�ò�����

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

 Description:		���ÿ����Ŀ��������,�������������Ӧ�Ĵ���
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

	/* �ҵ�Flash��������������������ʼ��ַ*/
	while( ( info->start[i] - info->start[0] ) < BOOTCODE_LENGTH )
	{
		i++;
	}

	/* ����������ȫ�������ڴ�*/
	flash_read_buf( info , ( char * )info->start[i] , &item_sdram , info->start[i+1] - info->start[i] );	

	/*ͨ�����������ͷ�����ж������������Ƿ����*/
	if( is_ok( ( char * ) item_sdram )  == 0 )
	{
		/*���������ݲ�����,�����ת�������Ƿ����*/
#ifdef CONFIG_FLASH_TRANSFER

		/* ����ת������ȫ�������ڴ�*/
		flash_read_buf( info , ( char * )info->start[i+1] , &item_sdram , info->start[i+2] - info->start[i+1] );

		if( is_ok( ( char * ) item_sdram )  == 0 )
		{
			/*��ת������Ҳ������,���ö�ʧ,���ش���*/
			printf("The config data has lost!\n");
			
			return ERROR_CONFIG_LOST;
		}
		else
		{
			/*��ת����������,�޸�������*/
			/*˳��:��������->  ������ת����Ŀ���ݵ�������->  ��������Ϊ��Ч->  ����ת��*/
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
	
	/*ɨ��������Ŀ*/
	while( ( sum + ITEM_HEAD_LEN ) <= ( info->start[i+1] - info->start[i] ) )
	{
		item = ( item_t * ) ( item_sdram + sum );

		if( item->hdr.avail == 0xff )
		{
			if( check_addr_null( ( unsigned int * )item , ITEM_HEAD_LEN ) == 1 )
			{			
				/*��鵽��Ŀֻд��ͷ����һ����,�ô���ĿΪ����,���ҿ��Կ϶��������һ����Ŀ*/
				avail = ITEM_BAD;
				flash_write( ( char * )&avail , info->start[i] + ( unsigned int )&( item->hdr.avail ) - ( unsigned int )item_sdram , 0x1 );				
			}
			break;
		}

		/*��Ŀ��Ч,ָ����һ��Ŀ*/
		if( item->hdr.avail == ITEM_UNAVAIL )
		{
			sum += ITEM_SIZE( item->hdr.len );

			continue;
		}

		/*��Ŀ�Ѿ��ǻ���,ָ����һ��Ŀ*/
		if( item->hdr.avail == ITEM_BAD )
		{
			sum += ITEM_SIZE( 0 );

			continue;
		}

		/*��Ч��Ŀ*/
		if( item->hdr.avail == ITEM_AVAIL )
		{
			crc16( item->data , item->hdr.len ,&checksum );
			if( checksum == item->hdr.crc )		/*��Ŀ����,ָ����һ��Ŀ*/
			{
				sum += ITEM_SIZE( item->hdr.len );
				
				continue;
			}
			else		/*��Ŀ������,�ô���ĿΪ��Ч,���ҿ��Կ϶��������һ����Ŀ*/
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

 Description:		��ȡָ����Ŀ������
 Data Accessed:
 Data Updated:
 Input:			*item_name:	 ��Ŀ����
 				*data:		��Ŀ���ݵĴ��ָ��
 Output:			*len:		��Ŀ��Ч���ݵĳ���
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
 Description:		������ת��Ϊ��Ŀ����
 Data Accessed:
 Data Updated:
 Input:			*item_name:	 ��Ŀ����
 				*data:		Ҫ��ŵ���Ŀ���ݵ�ָ��
 				
 Output:			*len:		��Ŀ��Ч���ݵĳ���
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
		
		if( item->hdr.avail == 0xff )	/*�ҵ�FLASH�յ�ַ*/
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
			/*���ֽ�Ҫ��Ϊ��Ч����Ŀ,���������ĵ�ַ*/
			if( strcmp( item->data , item_name ) == 0 )
			{
				item_repeat_addr[j++] = info->start[i] + ( unsigned int )&( item->hdr.avail ) - ( unsigned int )item_sdram;

				/*���������ظ�����Ŀ,��ǰһ����Ŀ��Ϊ��Ч*/
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

	/*�ϳ�����Ŀ*/
	item_data = ( item_t * )( CFG_SDRAM_BASE + CFG_SDRAM_SIZE - SDRAM_SIZE + ITEM_OFFSET );

	item_data->hdr.avail = ITEM_AVAIL;	
	memcpy( item_data->data , item_name , strlen( item_name ) );
	memcpy( item_data->data + strlen( item_name ) , "\0" , 1 );	
	memcpy( item_data->data + strlen( item_name ) + 1 , data , len );	
	item_data->hdr.len = strlen( item_name ) + 1 + len;
	crc16( item_data->data , item_data->hdr.len ,&checksum );
	item_data->hdr.crc = checksum;

	/*�ж�����Ŀ�Ĵ�С�Ƿ񳬳���������ʣ��ռ�*/
	if( ITEM_SIZE( item_data->hdr.len ) > ( info->start[i+1] - info->start[i] - sum ) )
	{
		/*��,ͨ����ת�ķ�ʽ���������е���Ч�ͻ���Ŀ����*/
		item_repeat();

		flash_read_buf( info , ( char * ) info->start[i] , &item_sdram , info->start[i+1] - info->start[i] );	
		sum = CONFIG_MARK_LEN;		
		j = 0;
		
		while( ( sum + ITEM_HEAD_LEN ) <= ( info->start[i+1] - info->start[i] ) )
		{
			item = ( item_t * ) ( item_sdram + sum );
			
			if( item->hdr.avail == 0xff )	/*�ҵ�FLASH�յ�ַ*/
			{
				break;
			}

			if( item->hdr.avail == ITEM_AVAIL )
			{
				/*���ֽ�Ҫ��Ϊ��Ч����Ŀ,���������ĵ�ַ*/
				if( strcmp( item->data , item_name ) ==0 )
				{
					item_repeat_addr[j++] = info->start[i] + ( unsigned int )&( item->hdr.avail ) - ( unsigned int )item_sdram;
				}
				
				sum += ITEM_SIZE( item->hdr.len );
				
				continue;
			}			
		}
		
		/*�ٴ��ж��Ƿ񳬳��ռ�*/
		if( ITEM_SIZE( item_data->hdr.len ) > ( info->start[i+1] - info->start[i] - sum ) )
		{
			/*��Ŀ̫��,�޷�����,���ش���*/
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

 Description:		���������ռ䲻��ʱͨ����ת����ת���ݣ���ת��������
 				flashһ���������ڴ�
 Data Accessed:
 Data Updated:
 Input:			��
 				 				
 Output:			��
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
 Description:		��ϵͳ��������
 				
 Calls:			tbs_calc_sum_addr
 				flash_erase
 				flash_write
 				
 Data Accessed:
 Data Updated:
 Input:			file_size:����image��С
 Output:			��
 Return:			ERROR_CRC:		crcУ�����,����image�⵽�ƻ�
				ERROR_PRODUCT:	����image�Ǳ���Ʒ
				ERROR_BIG:		image��������������Ŀռ�
				ERROR_OK:		�����ɹ�
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

 Description:		��ϵͳ��������
 				
 Calls:			tbs_calc_sum_addr
 				flash_erase
 				flash_write
 				sysdata_save
 Data Accessed:
 Data Updated:
 Input:			data:	����image����
 				sys_data:�ײ�������Ϣ
				*file_size:����image��С
 Output:			��
 Return:			ERROR_CRC:		crcУ�����,����image�⵽�ƻ�
				ERROR_PRODUCT:	����image�Ǳ���Ʒ
				ERROR_NIMG:		��������image
				ERROR_BIG:		image��������������Ŀռ�
				ERROR_OK:		�����ɹ�
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
		��image��CRCУ��
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
		��imageͷ�����Ϸ��Լ��
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

