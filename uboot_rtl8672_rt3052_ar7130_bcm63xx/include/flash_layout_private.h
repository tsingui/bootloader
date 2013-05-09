#ifndef _FLASH_LAYOUT_PRIVATE_H_
#define _FLASH_LAYOUT_PRIVATE_H_

enum upate_erros 
{ 
	ERROR_OK , 
	ERROR_CRC , 
	ERROR_BIG , 
	ERROR_PRODUCT ,  
	ERROR_IMG_TYPE 
};

int  sysdata_get(sys_config_t* syscfg);
int flash_update_img( unsigned char * data, sys_config_t* sys_data,unsigned int file_size );
int flash_update_bin( unsigned int file_size , char force_mark );

int item_check( void );
int item_get( void *data , char *item_name ,unsigned short *len );
int item_save( void *data , char *item_name ,unsigned short len );
int item_repeat( void );
#if 0
int betole(int a);
#endif

#endif  /*_FLASH_LAYOUT_PRIVATE_H_*/


