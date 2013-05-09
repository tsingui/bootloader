/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : flash_layout.h
 �ļ����� : flash image���ֵĶ���


 �޶���¼ :
          1 ���� : ������
            ���� : 2008-9-12
            ���� :

          2 �޶�: ������
             ����: 2009-3-16
             ����: �Ż���Ŀ�Ĳ���


=========================================================================*/
#ifndef _FLASH_LAYOUT_H_
#define _FLASH_LAYOUT_H_

/*=========================================================================
	�ײ�������Ϣ�в�Ʒ���ơ��汾��Ϣ��
	image�������к��ַ������ȵĶ���
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

/* bootloader�̶�ռ��64KB��С*/
#define	BOOTCODE_LENGTH		( 0x10000 )








/*=========================================================================
	flash image���ֵľ��嶨�壬�������ڴ��ʱ��ʼ��
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
	unsigned char		image_mark;							/*0: ϵͳ����image1��1: ϵͳ����image 2*/
	unsigned char 	board_id[BOARD_ID_NAME_LEN];			/*�ͻ�����ǩ��*/
	unsigned char		product[PRODUCT_NAME_LEN];				/*������ʶ��*/
	unsigned char		version[TBS_VERSION_NAME_LEN];			/*�汾��Ϣ*/
	unsigned char		tag[4];									/*  �ײ����ÿ��ʶ��'sysc'  */
} sys_config_t;

#define		SYSCFG_SIZE			( sizeof( sys_config_t ) )






/*=========================================================================
	image.binβ����Ϣ���ݽṹ���������ڴ��ʱ��ʼ��
=========================================================================*/

#define	IMAGE_BIN				"bin"

typedef struct
{
	unsigned char		product[PRODUCT_NAME_LEN];				/*������ʶ��*/
	unsigned char		bin_type[4];								/*BIN file��ʶ��*/
} image_bin_tail_t;






/*=========================================================================
	����image.imgͷ����Ϣ���ݽṹ���������ڴ��ʱ��ʼ��
=========================================================================*/

typedef struct
{
	unsigned int 			image_checksum;					/*imageУ���*/
	unsigned int			kernel_offset;
	unsigned int 			rootfs_offset;
	unsigned int 			kernel_size;
	unsigned int 			rootfs_size;
	unsigned int			image_len;
	unsigned char			img_type[IMAGE_TYPES_NAME_LEN]; 	/*�����ļ���ʶ����imageΪ:imgs    ˫����Ϊ:imgd*/
	unsigned char 		board_id[BOARD_ID_NAME_LEN];		/*�ͻ�����ǩ��*/
	unsigned char			product[PRODUCT_NAME_LEN];			/*������ʶ��*/
	unsigned char			version[TBS_VERSION_NAME_LEN];		/*�汾��Ϣ*/
} update_hdr_t;

/*=========================================================================
	imgd: ��ʾϵͳ��˫image��ϵͳ
	imgs: ��ʾϵͳ�ǵ�image��ϵͳ
	�����Ҳ���Ա��ⵥ��˫imageϵͳ�������������Ĵ���
	bin:��ʶ�ļ�ΪBIN�ļ�
=========================================================================*/

#ifdef CONFIG_DOUBLE_BACKUP
#define	IMAGE_TYPES			"imgd"
#else
#define	IMAGE_TYPES			"imgs"
#endif






/*=========================================================================
	������Ϣ��Ŀ���ݽṹ
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
	unsigned char			data[];				/* ����*/
} item_t;

#define		ITEM_HEAD_LEN					( sizeof( item_hdr_t ) )
#define		ITEM_SIZE( data_len )			( ( ITEM_HEAD_LEN + ( data_len ) ) + ( ( ITEM_HEAD_LEN + ( data_len ) ) % 2 ) )

#define		CONFIG_MARK		"OK"
#define		CONFIG_MARK_LEN	( strlen( CONFIG_MARK ) )

#define		ITEM_UNAVAIL	0x00
#define		ITEM_AVAIL		0x11
#define		ITEM_BAD		0x22


/*=========================================================================
	�����ݽṹ����Ӧ��ӿ���ioctlʱ��ײ�������ݽ���
=========================================================================*/

typedef struct
{
	unsigned char			*name;
	unsigned char			item_error;
	unsigned short		len;
	unsigned char			*data;				/* ����*/
} item_app_t;

/*=========================================================================
	ϵͳ��ʹ�õ���Ŀ���ƣ���Ŀ��������ú궨���������
	���ڹ����ά��
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
	�ڶ���Ŀ����ʱ���صĴ�����
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
	Ӧ�ò��ȡ�ͱ�����Ŀ�Ľӿ�
=========================================================================*/

int app_item_get( void *data , char *item_name ,unsigned short *len );		/* ��ȡ��Ӧ��Ŀ��Ϣ*/
int app_item_save( void *data , char *item_name ,unsigned short len );		/* ��������Ŀ*/

/*=========================================================================
	�ײ��ȡ�ͱ�����Ŀ�Ľӿ�
=========================================================================*/
int item_get( void *data , char *item_name ,unsigned short *len );		/* ��ȡ��Ӧ��Ŀ��Ϣ*/
int item_save( void *data , char *item_name ,unsigned short len );		/* ��������Ŀ*/





/*=========================================================================
	Ӧ�ò��ȡMACַַ�ӿ�
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

