

#include <rt_mmap.h>
#include <common.h>             /* for cpu_to_le32() and cpu_to_le32() */
#include <ralink_gpio.h>


#define DELAY 10
#define CLK_DURATION(clk) udelay(clk)
#define _SMI_ACK_RESPONSE(ok)	{ /*if (!(ok)) return RT_ERR_FAILED; */}
typedef int gpioID;
#define smi_SCK 23;	/* GPIO used for SMI Clock Generation */	/* GPIO23 */
#define smi_SDA 22;	/* GPIO used for SMI Data signal */		/* GPIO22 */

// those setting are for 3662/3883
#define RALINK_PIO_BASE			0xB0000600
#define RT3662_GPIOMODE_REG (0xB0000000+0x60)
#define PIODIR_R  (RALINK_PIO_BASE + 0X24)
#define PIODATA_R (RALINK_PIO_BASE + 0X20)
#define RT3662_RESET_REG (0xB0000000+0x34)


void set_mdc_to_gpio_mode(void)
{
	unsigned long value;
	unsigned long bit_one = 1;	

	printf("set mdc to gpio\n");

	value = le32_to_cpu(*(volatile u_long *)RT3662_GPIOMODE_REG);
	value |= (bit_one << 7);
	*(volatile u_long *)(RT3662_GPIOMODE_REG) = cpu_to_le32(value);
}

void set_i2c_spi_to_gpio_mode_and_dir(void)
{
	unsigned long value;
	unsigned long bit_one = 1;

	printf("set i2c and spi to gpio\n");

	value = le32_to_cpu(*(volatile u_long *)RT3662_GPIOMODE_REG);
	value |= (bit_one);
	value |= (bit_one << 1);	
	*(volatile u_long *)(RT3662_GPIOMODE_REG) = cpu_to_le32(value);

// i2c sd = wifi sw = gpio 1
// spi_cs0 = btn reset = gpio 3
	value = le32_to_cpu(*(volatile u_long *)PIODIR_R);
	value &= ~(bit_one << 1); 
	value &= ~(bit_one << 3); 
	*(volatile u_long *)(PIODIR_R) = cpu_to_le32(value);				
}

void reset_wlan_block(void)
{
	// this issue is under DSL-N10/N12U
	// if user switch WPA2-ent to WPA-ent
	// the wifi may disappear and need to power off and power on
	// we put WLAN reset here

	//
	// sometimes, WLAN is disappear after change SSID
	//
	*(volatile u_long *)(RT3662_RESET_REG) = cpu_to_le32(1 << 20);	
	udelay(5);
}



//
// DO NOT PRINT OUT DEBUG WHEN RELESED
// PRINT ROUTINE INTERFERES SMI TIMING AND CAUSES DATA ERROR
//

void _rtl865x_initGpioPin(int gpioIdx, int dummyParam0, int gpioDir, int dummyParam1)
{
	unsigned long bit_one = 1;	

//	printf("init %d %d\n",gpioIdx,gpioDir);

	unsigned long value;
	if (GPIO_DIR_OUT == gpioDir)
	{
		value = le32_to_cpu(*(volatile u_long *)PIODIR_R);
		value |= (bit_one << gpioIdx);	
		*(volatile u_long *)(PIODIR_R) = cpu_to_le32(value);			
	}
	else
	{
		value = le32_to_cpu(*(volatile u_long *)PIODIR_R);
		value &= ~(bit_one << gpioIdx);	
		*(volatile u_long *)(PIODIR_R) = cpu_to_le32(value);				
	}
}

void _rtl865x_setGpioDataBit(int gpioIdx, unsigned int val)
{
	unsigned long value;
	unsigned long bit_one = 1;	

	if (val & 1)
	{
		value = le32_to_cpu(*(volatile u_long *)PIODATA_R);
		value |= (bit_one << gpioIdx);
		*(volatile u_long *)(PIODATA_R) = cpu_to_le32(value);	
	}
	else
	{
		value = le32_to_cpu(*(volatile u_long *)PIODATA_R);
		value &= ~(bit_one << gpioIdx);
		*(volatile u_long *)(PIODATA_R) = cpu_to_le32(value);		
	}
}

void _rtl865x_getGpioDataBit(int gpioIdx, unsigned int* val)
{	
	unsigned long value;
	unsigned long bit_one = 1;	

	value = le32_to_cpu(*(volatile u_long *)PIODATA_R);

	if (value & (bit_one << gpioIdx))
	{
		*val = 1;
	}
	else
	{
		*val = 0;
	}
}

