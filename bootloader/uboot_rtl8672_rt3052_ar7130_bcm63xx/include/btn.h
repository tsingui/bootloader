/*
 * �ļ���:btn.h
 * ˵��:TBS��ť����ͷ�ļ�
 * 
 * ����:Zhang Yu
 *
 */

#ifndef __LINUX_BTN_H_INCLUDED
#define __LINUX_BTN_H_INCLUDED

#ifndef __KERNEL__
#undef CONFIG_BUTTON_TRIGGER_TIMER
#endif

#ifdef __BOOT__
#undef CONFIG_BUTTON_TRIGGER_TIMER
#endif

#ifdef CONFIG_BUTTON_TRIGGER_TIMER  /* ��ѯ��ʽ */
#include <linux/list.h>
#include <linux/spinlock.h>
#endif

/* ���尴ť���� */
typedef enum  {
    btn_reset   = 0,
    btn_wps,
    btn_wlan,
    btn_end,  /* ������� */
}btn_name;


typedef enum{
    BTN_UP      = 0,
    BTN_DOWN    = 1,
    BTN_START   = 2,
}btn_status;

typedef enum  {
    BTN_LEVEL_LOW  = 0,
    BTN_LEVEL_HIGH,
}btn_level;


struct btn_dev {
    btn_name name;
    int gpio;
    btn_level level;
	btn_status cur_status;  /* �����ж�ģʽ�±��������ť�ĵ�ǰ״̬���Է�ֹ���� */
    btn_status (*get_status)(struct btn_dev *);
    int ignore;
    
#ifdef CONFIG_BUTTON_TRIGGER_TIMER  /* ��ѯ��ʽ */
    struct list_head     node;          /* ��ť�豸�б�,������ѯ��ʽ */
#endif

};

/* ���尴ť��Ϣ��ͨ��netlink���ݸ�Ӧ�ò� */
struct btn_msg {
    btn_name name;
    btn_status state;
};


#ifdef CONFIG_BUTTON_TRIGGER_TIMER  /* ��ѯ��ʽ */
extern rwlock_t btn_list_lock;
extern struct list_head btn_list;
#endif

extern void btn_status_query(struct btn_dev *btn);
extern int btn_dev_register(struct btn_dev *btn);
extern void btn_dev_unregister(struct btn_dev *btn);
extern void btn_dev_suspend(struct btn_dev *btn);
extern void btn_dev_resume(struct btn_dev *btn);


#endif      /* __LINUX_BTN_H_INCLUDED */
