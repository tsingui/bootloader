/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : tbs_common.c
 文件描述 : 自己实现在函数


 函数列表 :
				ConvertEndian4
				ConvertEndian2
				check_addr_null
				short_addr_transfer

 修订记录 :
          1 创建 : 轩光磊
            日期 : 2008-9-27
            描述 :

=========================================================================*/
#include <common.h>
#include <tbs_common.h>

int flash_read_buf(flash_info_t * info, void* addr, unsigned char ** buf, int len);
/*=========================================================================
 Function:		int ConvertEndian4(int a)

 Description:		转换长度为4字节的字节序
 Calls:			无
 Data Accessed:
 Data Updated:
 Input:			a:		要转换的数据
 				 				
 Output:			无
 Return:			b:		转换的结果
			
 Others:			
=========================================================================*/

int ConvertEndian4(int a)
{
	unsigned char * byte;
	int b = 0;
	byte = ( unsigned char * )&a;
	b = (byte[3]<<3)|(byte[2]<<2)|(byte[1]<<1)|byte[0];
	return b;
}

/*=========================================================================
 Function:		int ConvertEndian2(int a)

 Description:		转换长度为2字节的字节序
 Calls:			无
 Data Accessed:
 Data Updated:
 Input:			a:		要转换的数据
 				 				
 Output:			无
 Return:			b:		转换的结果
			
 Others:			
=========================================================================*/

int ConvertEndian2(short a)
{
	unsigned char * byte;
	short b = 0;
	byte = ( unsigned char * )&a;
	b = (byte[1]<<1)|byte[0];
	return b;
}

/*=========================================================================
 Function:		char check_addr_null( unsigned int *addr, unsigned int len )
 Description:		检查指定的空间是否有数据
 Calls:			无
 Data Accessed:
 Data Updated:
 Input:			*addr:	要检查的地址
 				len:		要检查的长度
 				 				
 Output:			无
 Return:			0:	地址空间为空
 				1:	地址空间有数据
			
 Others:			若flash类型为SPI，则要将指定的空间数据读到内存再做检查
=========================================================================*/

int check_addr_null( unsigned int *addr, unsigned int len )
{
	unsigned int i = 0 ;
	unsigned int j = 0 ;
	unsigned char *src_lib_temp;
	
	src_lib_temp = ( unsigned char * )( CFG_SDRAM_BASE + CFG_SDRAM_SIZE - SDRAM_SIZE + FLASH_SDRAM_SECTOR );

	if( ( unsigned int ) addr >= CFG_FLASH_BASE && ( unsigned int ) addr <= ( CFG_FLASH_BASE + info->size ) )
	{
			flash_read_buf( info , addr , &src_lib_temp , len );			
			addr = ( unsigned int * ) src_lib_temp;
	}
	
	for( i = 0 ; i < len ; i++ )
	{
		if( *( ( unsigned char * )addr + i ) == 0xff )
			j++;	
	}

	if( i == j )
		return 0;
	else 
		return 1;
}

int	is_sysdata(sys_config_t *syscfg)
{
	if(syscfg->tag[0]=='s' && syscfg->tag[1]=='y' && syscfg->tag[2]=='s' && syscfg->tag[3]=='c')
		return 1;
	else
		return 0;
}

int	is_ok( char * data )
{
	if( data[0] == 'O' && data[1] == 'K' )
		return 1;
	else
		return 0;
}


