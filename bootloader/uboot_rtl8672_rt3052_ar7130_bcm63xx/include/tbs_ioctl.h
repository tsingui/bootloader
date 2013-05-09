/*
 *  TBS ioctl �����ֶ���
 *
 *
 *  ˵��:TBS˽��ioctlʹ��ʹ��G�ֶ����ɵ�������
 *
 *       by Zhang Yu
 *
 *
 *	���TBS��Ŀ�Ļ�ȡ�ͱ���IOCTL��
 *	2008-10-27 by xuanguanglei
 *
 */

#ifndef __LINUX_TBS_IOCTL_H_INCLUDED
#define __LINUX_TBS_IOCTL_H_INCLUDED


#define TBS_IOCTL_MAGIC	 'G'

/* ioctl led�����ֶ��� */

#define TBS_IOCTL_LED_SET		_IOWR(TBS_IOCTL_MAGIC, 0, struct tbs_ioctl_led_parms)
#define TBS_IOCTL_LED_GET		_IOWR(TBS_IOCTL_MAGIC, 1, struct tbs_ioctl_led_parms)

#define TBS_IOCTL_ITEM_GET		_IOWR(TBS_IOCTL_MAGIC, 2, item_t)
#define TBS_IOCTL_ITEM_SAVE	_IOWR(TBS_IOCTL_MAGIC, 3, item_t)

#define TBS_IOCTL_MAC_READ		_IOWR(TBS_IOCTL_MAGIC, 4, mac_t)

#endif /* __LINUX_TBS_IOCTL_H_INCLUDED */

