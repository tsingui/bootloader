/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : flash_layout.h
 文件描述 : flash image布局的定义


 修订记录 :
          1 创建 : 轩光磊
            日期 : 2008-9-12
            描述 :

          2 修订: 轩光磊
             日期: 2009-3-16
             描述: 优化条目的操作


=========================================================================*/
#ifndef _FLASH_LAYOUT_H_
#define _FLASH_LAYOUT_H_

/*=========================================================================
	底层配置信息中产品名称、版本信息、
	image类别和序列号字符串长度的定义
=========================================================================*/

#define PRODUCT_NAME_LEN			32
#define TBS_VERSION_NAME_LEN		16
#define IMAGE_TYPES_NAME_LEN		5
#define BOARD_ID_NAME_LEN			12

#define		ZONE_BOOTLOADER			0
#define		ZONE_KERNEL_FIRST			1
#define		ZONE_ROOTFS_FIRST			2
#define		ZONE_KERNEL_SECOND		3
#define		ZONE_ROOTFS_SECOND		4
#define		ZONE_EXTFS					5
#define		ZONE_MAX_ITEM				6

/* bootloader固定占用64KB大小*/
#define	BOOTCODE_LENGTH		( 0x10000 )








/*=========================================================================
	flash image布局的具体定义，数据项在打包时初始化
=========================================================================*/

typedef struct
{
	unsigned int 			zone_offset[ZONE_MAX_ITEM];
} flash_layout_t;

typedef struct
{
	flash_layout_t		layout;
	unsigned int		ip;
	unsigned int		first_image_len;
	unsigned int		first_image_checksum;
	unsigned int		second_image_len;
	unsigned int		second_image_checksum;
	unsigned char 	mac[6];
	unsigned char		endian;
	unsigned char		image_mark;							/*0: 系统引导image1；1: 系统引导image 2*/
	unsigned char 	board_id[BOARD_ID_NAME_LEN];			/*客户电子签名*/
	unsigned char		product[PRODUCT_NAME_LEN];				/*方案标识符*/
	unsigned char		version[TBS_VERSION_NAME_LEN];			/*版本信息*/
	unsigned char		tag[4];									/*  底层配置块标识符'sysc'  */
} sys_config_t;

#define		SYSCFG_SIZE			( sizeof( sys_config_t ) )






/*=========================================================================
	image.bin尾部信息数据结构，数据项在打包时初始化
=========================================================================*/

#define	IMAGE_BIN				"bin"

typedef struct
{
	unsigned char		product[PRODUCT_NAME_LEN];				/*方案标识符*/
	unsigned char		bin_type[4];								/*BIN file标识符*/
} image_bin_tail_t;






/*=========================================================================
	升级image.img头部信息数据结构，数据项在打包时初始化
=========================================================================*/

typedef struct
{
	unsigned int 			image_checksum;					/*image校验和*/
	unsigned int			kernel_offset;
	unsigned int 			rootfs_offset;
	unsigned int 			kernel_size;
	unsigned int 			rootfs_size;
	unsigned int			image_len;
	unsigned char			img_type[IMAGE_TYPES_NAME_LEN]; 	/*升级文件标识符单image为:imgs    双备份为:imgd*/
	unsigned char 		board_id[BOARD_ID_NAME_LEN];		/*客户电子签名*/
	unsigned char			product[PRODUCT_NAME_LEN];			/*方案标识符*/
	unsigned char			version[TBS_VERSION_NAME_LEN];		/*版本信息*/
} update_hdr_t;

/*=========================================================================
	imgd: 表示系统是双image的系统
	imgs: 表示系统是单image的系统
	这个宏也可以避免单、双image系统互相升级带来的错误
	bin:标识文件为BIN文件
=========================================================================*/

#ifdef CONFIG_DOUBLE_BACKUP
#define	IMAGE_TYPES			"imgd"
#else
#define	IMAGE_TYPES			"imgs"
#endif






/*=========================================================================
	配置信息条目数据结构
=========================================================================*/

typedef struct
{
	unsigned short		crc;
	unsigned short		len;
	unsigned char			avail;
} __attribute__ ((packed))  item_hdr_t;

typedef struct
{
	item_hdr_t			hdr;
	unsigned char			data[];				/* 数据*/
} item_t;

#define		ITEM_HEAD_LEN					( sizeof( item_hdr_t ) )
#define		ITEM_SIZE( data_len )			( ( ITEM_HEAD_LEN + ( data_len ) ) + ( ( ITEM_HEAD_LEN + ( data_len ) ) % 2 ) )

#define		CONFIG_MARK		"OK"
#define		CONFIG_MARK_LEN	( strlen( CONFIG_MARK ) )

#define		ITEM_UNAVAIL	0x00
#define		ITEM_AVAIL		0x11
#define		ITEM_BAD		0x22


/*=========================================================================
	此数据结构用来应层接口做ioctl时与底层进行数据交互
=========================================================================*/

typedef struct
{
	unsigned char			*name;
	unsigned char			item_error;
	unsigned short		len;
	unsigned char			*data;				/* 数据*/
} item_app_t;

/*=========================================================================
	系统中使用的条目名称，条目名称最好用宏定义放在这里
	便于管理和维护
=========================================================================*/

#define		LLCONFIG_NAME			"llconfig"
#define		WLAN_NAME				"wlan_cfg"
#define		TBS_APP_CFG			    "tbs_app_cfg"
#define		TR069_EVENT			    "tr069_event"
#define		PPPOE_SSID			    "pppoe_ssid"
#define     TBS_BIT_FLAG            "tbs_bit_flag"
#define     TBS_DEFCFG_PATH         "def_cfg_path"
#define     TBS_USERCFG_ITEM        "user_cfg_item"
#define     TBS_USERCFG_PREFIX		"tbs_app_cfg_"

/*=========================================================================
	在对条目操作时返回的错误码
=========================================================================*/

enum item_error
{
	ERROR_ITEM_OK = 0,
	ERROR_ITEM_MTD,
	ERROR_ITEM_IOCTRL,
	ERROR_ITEM_MALLOC,
	ERROR_ITEM_NOT_FIND,
	ERROR_ITEM_CRC,
	ERROR_ITEM_BIG,
	ERROR_ITEM_REPEAT_OK,
	ERROR_ITEM_REPEAT_FAIL,
	ERROR_CONFIG_LOST
};

/*=========================================================================
	应用层获取和保存条目的接口
=========================================================================*/

int app_item_get( void *data , char *item_name ,unsigned short *len );		/* 获取相应条目信息*/
int app_item_save( void *data , char *item_name ,unsigned short len );		/* 保存新条目*/

/*=========================================================================
	底层获取和保存条目的接口
=========================================================================*/
int item_get( void *data , char *item_name ,unsigned short *len );		/* 获取相应条目信息*/
int item_save( void *data , char *item_name ,unsigned short len );		/* 保存新条目*/





/*=========================================================================
	应用层获取MAC址址接口
=========================================================================*/

typedef struct
{
	int 				id;
	int				offset;
	unsigned char		mac[6];
} mac_t;

enum device_mac
{
	LAN_MAC = 0,
	WAN_MAC,
	WLAN_MAC,
	USB_MAC
};

int tbs_read_mac( int id , int offset , unsigned char *mac );
int app_tbs_read_mac( int id , int offset , unsigned char *mac );

#endif  /*_FLASH_LAYOUT_H_*/

