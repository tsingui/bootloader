/*
 * �ļ���:led.h
 * ˵��:TBS LED����ͷ�ļ�
 *
 * ����:Zhang Yu
 *
 */

#ifndef __LINUX_LEDS_CORE_H_INCLUDED
#define __LINUX_LEDS_CORE_H_INCLUDED

#define LED_NAME_MAX	32
#define TRIG_NAME_MAX	32


typedef enum  {
	led_power	= 0,
	led_power_green,
	led_power_red,
	led_internet,
	led_internet_green,
	led_internet_red,
	led_usb,
	led_wlan,
	led_wps,
	led_wps_green,
	led_wps_red,
	led_wps_yellow,
	led_dsl,
	led_end, /* ������� */
}led_name;

typedef enum  {
	led_off_trig	= 0,
	led_on_trig,
	led_flash_trig,
	led_dataflash_trig,
	led_wps_inprogress_trig,
	led_wps_error_trig,
	led_wps_overlap_trig,
	led_wps_success_trig,
	led_end_trig,
}trig_name;

typedef enum  {
	LED_GPIO_LOW  = 0,
	LED_GPIO_HIGH,
}led_level;


typedef enum  {
	LED_BOOT_OFF  = 0,
	LED_BOOT_ON,
}led_boot_default;

struct led_dev {
	led_name    name;
	int	        gpio;    /* GPIO�� */
	led_level   level;   /* �ߵ͵�ƽ���� */
	trig_name   kernel_default; /* kernelĬ��״̬ */
	led_boot_default boot_default; /* BootloaderĬ��״̬ */
	struct led_trigger *cur_trig;  /* ��ǰtrigger */
	void	*trigger_data;
};

struct led_trigger {
	trig_name name;
	void	(*activate)(struct led_dev *led_cdev);
	void	(*deactivate)(struct led_dev *led_cdev);
};


struct led_hw_handler {
	void	(*led_on)(struct led_dev *led_cdev);
	void	(*led_off)(struct led_dev *led_cdev);
};



extern struct led_hw_handler* globe_led_hw_handler;   /* LED��ƾ����ȫ��ʹ�� */

int led_dev_register(struct led_dev *led);
int led_dev_unregister(struct led_dev *led);

int led_trigger_register(struct led_trigger *trig);
int led_trigger_unregister(struct led_trigger *trig);

int led_hw_handle_register(struct led_hw_handler *handler);
int led_hw_handle_unregister(struct led_hw_handler *handler);

int led_trigger_set(led_name led, trig_name trigger);
trig_name led_trigger_get(led_name led);

/* WPS LED �Ƿ�ʹ�ò�ɫ�� */
//#define WPS_LED_COLOR   1
int led_wps_set(trig_name trigger);

void led_set_on(struct led_dev *led);
void led_set_off(struct led_dev *led);



/* ioctl led �������� */

struct tbs_ioctl_led_parms
{
	led_name led;
	trig_name trig;
};

#endif		/* __LINUX_LEDS_CORE_H_INCLUDED */
