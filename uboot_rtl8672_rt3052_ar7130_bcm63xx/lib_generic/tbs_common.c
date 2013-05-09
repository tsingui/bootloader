/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : tbs_common.c
 �ļ����� : �Լ�ʵ���ں���


 �����б� :
				ConvertEndian4
				ConvertEndian2
				check_addr_null
				short_addr_transfer

 �޶���¼ :
          1 ���� : ������
            ���� : 2008-9-27
            ���� :

=========================================================================*/
#include <common.h>
#include <tbs_common.h>

int flash_read_buf(flash_info_t * info, void* addr, unsigned char ** buf, int len);
/*=========================================================================
 Function:		int ConvertEndian4(int a)

 Description:		ת������Ϊ4�ֽڵ��ֽ���
 Calls:			��
 Data Accessed:
 Data Updated:
 Input:			a:		Ҫת��������
 				 				
 Output:			��
 Return:			b:		ת���Ľ��
			
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

 Description:		ת������Ϊ2�ֽڵ��ֽ���
 Calls:			��
 Data Accessed:
 Data Updated:
 Input:			a:		Ҫת��������
 				 				
 Output:			��
 Return:			b:		ת���Ľ��
			
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
 Description:		���ָ���Ŀռ��Ƿ�������
 Calls:			��
 Data Accessed:
 Data Updated:
 Input:			*addr:	Ҫ���ĵ�ַ
 				len:		Ҫ���ĳ���
 				 				
 Output:			��
 Return:			0:	��ַ�ռ�Ϊ��
 				1:	��ַ�ռ�������
			
 Others:			��flash����ΪSPI����Ҫ��ָ���Ŀռ����ݶ����ڴ��������
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


