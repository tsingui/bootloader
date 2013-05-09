#ifndef __MKIMAGE_H__
#define __MKIMAGE_H__

typedef struct 
{	
	volatile unsigned int		image_load;				/*bootloader_second入口地址*/
	volatile unsigned int		image_len;				/*bootloader_second压缩后的长度*/
	unsigned char		tag[3];					/*  'two'  */
}bootloader_second_image_hdr_t;

#endif  /*endif __MKIMAGE_H__*/

