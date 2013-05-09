/******************************************************************************
* Filename : gpio.c
* This part is used to control LED and detect button-press
* 
******************************************************************************/

#include <common.h>
#include <rt_mmap.h>
#include <ralink_gpio.h>

#define LED_POWER	0
#define BTN_RESET	4
#define BTN_WPS		3
#define BTN_WIFI_SW	1
#define LED_INTERNET 2


unsigned long gpiomode_org;

void asus_gpio_init(void);
void asus_gpio_uninit(void);

int ralink_gpio_ioctl2(unsigned int req, int idx, unsigned long arg)
{
	unsigned long tmp;
/*
	unsigned long PIODATA0;
	unsigned long PIODATA1;
	unsigned long PIODATA2;
*/
	int *value;

	req &= RALINK_GPIO_DATA_MASK;

	switch(req) {
	case RALINK_GPIO_READ_BIT:
/*
		printf("RALINK_REG_PIODATA: %x\n",	RALINK_REG_PIODATA);
		printf("RALINK_REG_PIO3924DATA: %x\n",	RALINK_REG_PIO3924DATA);
		printf("RALINK_REG_PIO5140DATA: %x\n",	RALINK_REG_PIO5140DATA);
		PIODATA0 = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		PIODATA1 = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
		PIODATA2 = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));
		printf("*RALINK_REG_PIODATA: %x\n",	PIODATA0);
		printf("*RALINK_REG_PIO3924DATA: %x\n",	PIODATA1);
		printf("*RALINK_REG_PIO5140DATA: %x\n",	PIODATA2);
*/

		if (idx <= 23)
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		else if (idx <= 39)
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
		else
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));

		if (idx <= 23)
			tmp = (tmp >> idx) & 1L;
		else if (idx <= 39)
			tmp = (tmp >> (idx-24)) & 1L;
		else
			tmp = (tmp >> (idx-40)) & 1L;
		value = (int *)arg;
		*value = tmp;
		return tmp;

		break;
	case RALINK_GPIO_WRITE_BIT:
		if (idx <= 23) {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
			if (arg & 1L)
				tmp |= (1L << idx);
			else
				tmp &= ~(1L << idx);
			*(volatile u32 *)(RALINK_REG_PIODATA)= cpu_to_le32(tmp);
		}
		else if (idx <= 39) {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
			if (arg & 1L)
				tmp |= (1L << (idx-24));
			else
				tmp &= ~(1L << (idx-24));
			*(volatile u32 *)(RALINK_REG_PIO3924DATA)= cpu_to_le32(tmp);
		}
		else {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));
			if (arg & 1L)
				tmp |= (1L << (idx-40));
			else
				tmp &= ~(1L << (idx-40));
			*(volatile u32 *)(RALINK_REG_PIO5140DATA)= cpu_to_le32(tmp);
		}

		break;
	default:
		return -1;
	}
	return 0;
}

int
ralink_gpio_write_bit2(int idx, int value)
{
	unsigned int req;
	value &= 1;
       
	if (0L <= idx && idx < RALINK_GPIO_NUMBER)
		req = RALINK_GPIO_WRITE_BIT;
	else
		return -1;

	return ralink_gpio_ioctl2(req, idx, value);
}

int
ralink_initGpioPin2(unsigned int idx, int dir)
{
	unsigned long tmp;

	if (idx < 0 || RALINK_GPIO_NUMBER <= idx)
		return -1;

	if (dir == GPIO_DIR_OUT)
	{
		if (idx <= 23) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
			tmp |= (1L << idx);
		}
		else if (idx <= 39) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
			tmp |= (1L << (idx-24));
		}
		else {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
			tmp |= (1L << (idx-40));
		}
	}
	else if (dir == GPIO_DIR_IN)
	{		
		if (idx <= 23) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
			tmp &= ~(1L << idx);
		}
		else if (idx <= 39) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
			tmp &= ~(1L << (idx-24));
		}
		else {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
			tmp &= ~(1L << (idx-40));
		}
	}
	else
		return -1;

	if (idx <= 23) {
		*(volatile u32 *)(RALINK_REG_PIODIR) = cpu_to_le32(tmp);
	}
	else if (idx <= 39) {
		*(volatile u32 *)(RALINK_REG_PIO3924DIR) = cpu_to_le32(tmp);
	}
	else {
		*(volatile u32 *)(RALINK_REG_PIO5140DIR) = cpu_to_le32(tmp);
	}

	return 0;
}

int
ralink_gpio_read_bit2(int idx)
{
	unsigned int req;
	int value = 0;

	if (0L <= idx && idx < RALINK_GPIO_NUMBER)
		req = RALINK_GPIO_READ_BIT;
	else
		return -1;

	if (ralink_gpio_ioctl2(req, idx, &value) < 0)
		return -1;

	return value;
}

// those setting are for 3662/3883
#define RALINK_PIO_BASE			0xB0000600
#define PIODIR3924_R  (RALINK_PIO_BASE + 0x4c)
#define PIODATA3924_R (RALINK_PIO_BASE + 0x48)
#define PIODIR_R  (RALINK_PIO_BASE + 0X24)
#define PIODATA_R (RALINK_PIO_BASE + 0X20)
#define RT3662_GPIOMODE_REG (0xB0000000+0x60)

extern int flash_sect_erase (ulong addr_first, ulong addr_last);

unsigned long DETECT(void)
{
//	asus_gpio_uninit();
	// btn reset gpio is different with RT-N56U

	if (!ralink_gpio_read_bit2(BTN_RESET))
	{
//		asus_gpio_uninit();
		printf("BTN_RESET pressed\n");
		return 1;
	}
	else
	{
//		asus_gpio_uninit();
		unsigned long value;
		unsigned long bit_one = 1;	
		int rc;
		
		value = le32_to_cpu(*(volatile u_long *)PIODATA3924_R);
		
		if (value & (bit_one << (BTN_WPS-1)))
		{
			//printf("BTN_WPS did not press\n");
		}
		else
		{
			printf("BTN_WPS pressed\n");		
			printf("\n Erase NVRAM block (config) !!");
			flash_sect_protect(0, 0xbc000000+0x30000, 0xbc000000+0x30000+0x10000-1);
			rc = flash_sect_erase(0xbc000000+0x30000, 0xbc000000+0x30000+0x10000-1);
			flash_sect_protect(1, 0xbc000000+0x30000, 0xbc000000+0x30000+0x10000-1);			
		}		


		return 0;
	}

#if 0

	unsigned long value;
	unsigned long bit_one = 1;	


	value = le32_to_cpu(*(volatile u_long *)PIODATA_R);
	printf("btn value = %x\n",value);

	value = le32_to_cpu(*(volatile u_long *)PIODATA3924_R);
	printf("btn value = %x\n",value);

	
	if (value & (bit_one << BTN_RESET))
	{
		printf("BTN_RESET did not press\n");
		return 0;	
	}
	else
	{
		printf("BTN_RESET pressed\n");
		return 1;	
	}

#endif

#if 0
	value = le32_to_cpu(*(volatile u_long *)PIODATA3924_R);
	printf("btn value = %x\n",value);

	if (value & (bit_one << (BTN_RESET-1)))
	{
		printf("BTN_RESET did not press\n");
		//return 0;	
		// let some modified board could use rescue mode
		if (value & (bit_one << (BTN_WPS-1)))
		{
			printf("BTN_WPS did not press\n");
			return 0;	
		}
		else
		{
			printf("BTN_WPS pressed\n");
			return 1;	
		}		
	}
	else
	{
		printf("BTN_RESET pressed\n");
		return 1;	
	}


	
#endif	

}

void LEDON(void)
{
	ralink_gpio_write_bit2(LED_POWER, 0);
}

void LEDOFF(void)
{
	ralink_gpio_write_bit2(LED_POWER, 1);
}


void dsl_n55u_ralink_initGpio(unsigned long pin_num, int dir)
{
	unsigned long value;
	unsigned long bit_one = 1;

	printf("\n GPIO pin %d to %d\n",pin_num,dir);

	//Set Gpio pin to input
	if (dir == GPIO_DIR_IN)
	{
		value = le32_to_cpu(*(volatile u_long *)PIODIR3924_R);
		value &= ~(bit_one << (pin_num-1));
		*(volatile u_long *)(PIODIR3924_R) = cpu_to_le32(value);
	}
	else
	{
		value = le32_to_cpu(*(volatile u_long *)PIODIR3924_R);
		value |= (bit_one << (pin_num-1));
		*(volatile u_long *)(PIODIR3924_R) = cpu_to_le32(value);	
	}
}


void asus_gpio_init(void)
{
	unsigned long gpiomode;

//	printf("asus_gpio_init()...\n");
	gpiomode_org = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));
	printf("GPIOMODE before: %x\n",  gpiomode_org);
	gpiomode = gpiomode_org;
	gpiomode &= ~RALINK_GPIOMODE_DFT;
        gpiomode |= RALINK_GPIOMODE_DFT;
	printf("GPIOMODE writing: %x\n", gpiomode);
	*(volatile u32 *)(RALINK_REG_GPIOMODE) = cpu_to_le32(gpiomode);

	ralink_initGpioPin2(LED_POWER, GPIO_DIR_OUT);
	dsl_n55u_ralink_initGpio(LED_INTERNET, GPIO_DIR_OUT);	
	ralink_initGpioPin2(BTN_RESET, GPIO_DIR_IN);	
	//dsl_n55u_ralink_initGpio(BTN_RESET, GPIO_DIR_IN);
	dsl_n55u_ralink_initGpio(BTN_WPS, GPIO_DIR_IN);
	ralink_initGpioPin2(BTN_WIFI_SW, GPIO_DIR_IN);	
}

void asus_gpio_uninit(void)
{
	unsigned long gpiomode;

//	printf("asus_gpio_uninit()...\n");
	gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));
//	printf("GPIOMODE before: %x\n",  gpiomode);
	printf("GPIOMODE restoring: %x\n", gpiomode_org);
	*(volatile u32 *)(RALINK_REG_GPIOMODE) = cpu_to_le32(gpiomode_org);
}
