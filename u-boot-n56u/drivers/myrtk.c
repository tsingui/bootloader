#include <rtk_api.h>
#include <rtk_api_ext.h>
#include <rtl8367b_asicdrv_port.h>
//#include <rtl8367b_asicdrv_portsecurity.h>


#include <rt_mmap.h>
#include <common.h>		/* for cpu_to_le32() and cpu_to_le32() */
#include <ralink_gpio.h>

void _rtl865x_initGpioPin(int gpioIdx, int dummyParam0, int gpioDir, int dummyParam1);
void _rtl865x_setGpioDataBit(int gpioIdx, unsigned int val);
void _rtl865x_getGpioDataBit(int gpioIdx, unsigned int* val);
typedef int gpioID;
#define smi_SCK 23	/* GPIO used for SMI Clock Generation */	/* GPIO23 */
#define smi_SDA	22 /* GPIO used for SMI Data signal */		/* GPIO22 */
// dummy
#define GPIO_PERI_GPIO 0
#define GPIO_INT_DISABLE 0



//unsigned long gpiomode_backup;

#define	PHY_CONTROL_REG			0
#define	CONTROL_REG_PORT_POWER_BIT	0x800

extern int rtl8367r_switch_inited;


void LANWANPartition_8367r(void)
{
// connect CPU port to all LAN port

	rtk_portmask_t portmask;
	portmask.bits[0]=0x1e;
	rtk_led_enable_set(LED_GROUP_0, portmask);	
	rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_LINK_ACT);
	rtk_led_operation_set(LED_OP_PARALLEL);

/*
	{
		unsigned short RegIdx;
		for (RegIdx = 0x1b00; RegIdx <= 0x1b30; RegIdx++)
		{
			rtk_api_ret_t retVal;
			rtk_uint32 data;			
			if((retVal = rtl8367b_getAsicReg(RegIdx, &data)) != RT_ERR_OK)
			{
				printf("error = %d\n", retVal);
			}
			printf("RegIdx = %x , data = %x\n", RegIdx, data);
		}
	}
*/	

	rtk_cpu_enable_set(ENABLE);
	rtk_cpu_tagPort_set(RTK_EXT_1_MAC,CPU_INSERT_TO_NONE);

//	it should not call rtk_vlan_init() since this router uses SVLAN
}



// those setting are for 3662/3883
#define RALINK_PIO_BASE			0xB0000600
#define PIODIR3924_R  (RALINK_PIO_BASE + 0x4c)
#define PIODATA3924_R (RALINK_PIO_BASE + 0x48)
#define RT3662_GPIOMODE_REG (0xB0000000+0x60)


void smi_init_gpio_realtek(void)
{
    _rtl865x_initGpioPin(smi_SDA, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
    _rtl865x_initGpioPin(smi_SCK, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);

    _rtl865x_setGpioDataBit(smi_SDA, 1);
    _rtl865x_setGpioDataBit(smi_SCK, 1);
}


void reset_phy_gpio_realtek(void)
{
	unsigned long value;

	printf("\n GPIO pin 24 reset to switch\n");

	//Set Gpio pin 24 to output
	value = le32_to_cpu(*(volatile u_long *)PIODIR3924_R);
	value |= 1;
	*(volatile u_long *)(PIODIR3924_R) = cpu_to_le32(value);

	//Set Gpio pin 24 to low
	value = le32_to_cpu(*(volatile u_long *)PIODATA3924_R);
	value &= ~1;
	*(volatile u_long *)(PIODATA3924_R) = cpu_to_le32(value);

	udelay(50000);
	
	//Set Gpio pin 24 to high
	value = le32_to_cpu(*(volatile u_long *)PIODATA3924_R);
	value |= 1;
	*(volatile u_long *)(PIODATA3924_R) = cpu_to_le32(value);

	udelay(50000);

	//Set Gpio pin 24 to low
	value = le32_to_cpu(*(volatile u_long *)PIODATA3924_R);
	value &= ~1;
	*(volatile u_long *)(PIODATA3924_R) = cpu_to_le32(value);

	udelay(50000);
	
	//Set Gpio pin 24 to high
	value = le32_to_cpu(*(volatile u_long *)PIODATA3924_R);
	value |= 1;
	*(volatile u_long *)(PIODATA3924_R) = cpu_to_le32(value);
	
	udelay(50000);

}


#define CLK_DURATION(clk) udelay(clk)


void test_smi_signal_and_wait()
{
        int i;
        for (i=0; i<100; i++)
        {
                rtk_uint32 data;
                rtk_api_ret_t retVal;

            if((retVal = rtl8367b_getAsicReg(0x1202, &data)) != RT_ERR_OK)
            {
                        printf("error = %d\n", retVal);
                }
                printf("data = %x\n", data);
                if (data == 0x88a8) break;

            CLK_DURATION(50000);
        }
}



int
rtl8367r_switch_init_pre()
{
	rtk_api_ret_t retVal;
	unsigned long gpiomode;

	set_mdc_to_gpio_mode();

	smi_reset();	

	// after reset, switch need to delay 1 ms
	// if not, SMI may send out unknown data
	udelay(1000);

	smi_init();

	test_smi_signal_and_wait();

	retVal = rtk_switch_init();
	printf("rtk_switch_init(): return %d\n", retVal);
	if (retVal != RT_ERR_OK) return retVal;

//
// RALINK uses RGMII to connect switch IC directly
// we need to set the MDIO mode here
//
	rtk_port_mac_ability_t mac_cfg;
	mac_cfg.forcemode = MAC_FORCE; 
	mac_cfg.speed = SPD_1000M; 
	mac_cfg.duplex = FULL_DUPLEX; 
	mac_cfg.link = PORT_LINKUP;
	mac_cfg.nway = DISABLED; 
	mac_cfg.txpause = ENABLED; 
	mac_cfg.rxpause = ENABLED; 
	retVal = rtk_port_macForceLinkExt_set (1, MODE_EXT_RGMII,&mac_cfg);
	printf("rtk_port_macForceLinkExt_set(): return %d\n", retVal);	

	int input_txDelay = 1;
	int input_rxDelay = 2;

	printf("input_txDelay:%d, input_rxDelay:%d\n", input_txDelay, input_rxDelay);
	retVal = rtk_port_rgmiiDelayExt_set(1, input_txDelay, input_rxDelay);
	printf("rtk_port_rgmiiDelayExt_set(): return %d\n", retVal);
	

	// power down all LAN ports
	// this is to force DHCP IP address new when PC cable connects to LAN port
	
	rtk_port_phy_data_t pData;
	
	rtk_port_phyReg_get(1, PHY_CONTROL_REG, &pData);
	printf("** rtk_port_phyReg_get = %x\n", pData);		
	pData |= CONTROL_REG_PORT_POWER_BIT;
	rtk_port_phyReg_set(1, PHY_CONTROL_REG, pData);
	
	rtk_port_phyReg_get(2, PHY_CONTROL_REG, &pData);
	printf("** rtk_port_phyReg_get = %x\n", pData);		
	pData |= CONTROL_REG_PORT_POWER_BIT;
	rtk_port_phyReg_set(2, PHY_CONTROL_REG, pData);

	rtk_port_phyReg_get(3, PHY_CONTROL_REG, &pData);
	printf("** rtk_port_phyReg_get = %x\n", pData);		
	pData |= CONTROL_REG_PORT_POWER_BIT;
	rtk_port_phyReg_set(3, PHY_CONTROL_REG, pData);

	rtk_port_phyReg_get(4, PHY_CONTROL_REG, &pData);
	printf("** rtk_port_phyReg_get = %x\n", pData);		
	pData |= CONTROL_REG_PORT_POWER_BIT;
	rtk_port_phyReg_set(4, PHY_CONTROL_REG, pData);

/*
{
int i;
for (i=0; i<8; i++)
{
	rtk_port_phy_data_t pData;
	rtk_port_phyReg_get(i, 1, &pData);
	printf("** %d rtk_port_phyReg_get = %x\n", i, pData); 	
}
}
*/

/*
{
// read EXT MAC status
// seems not works
    rtk_uint32 data;
    rtk_api_ret_t retVal;

    if((retVal = rtl8367b_getAsicReg(0x1305, &data)) != RT_ERR_OK)
    {
        printf("error = %d\n", retVal);
    }
    printf("data = %x\n", data);

	if((retVal = rtl8367b_getAsicReg(0x1311, &data)) != RT_ERR_OK)
	{
		printf("error = %d\n", retVal);
	}
	printf("data = %x\n", data);

//data 1305 = c010
//data 1311 = 1076                
}
*/

	rtl8367r_switch_inited = 1;
	return RT_ERR_OK;
}





int
rtl8367r_switch_init()
{
	// Power up all LAN ports
	rtk_port_phy_data_t pData;
	
	rtk_port_phyReg_get(1, PHY_CONTROL_REG, &pData);
	pData &= (~CONTROL_REG_PORT_POWER_BIT);
	rtk_port_phyReg_set(1, PHY_CONTROL_REG, pData);
	
	rtk_port_phyReg_get(2, PHY_CONTROL_REG, &pData);
	pData &= (~CONTROL_REG_PORT_POWER_BIT);
	rtk_port_phyReg_set(2, PHY_CONTROL_REG, pData);

	rtk_port_phyReg_get(3, PHY_CONTROL_REG, &pData);
	pData &= (~CONTROL_REG_PORT_POWER_BIT);
	rtk_port_phyReg_set(3, PHY_CONTROL_REG, pData);

	rtk_port_phyReg_get(4, PHY_CONTROL_REG, &pData);
	pData &= (~CONTROL_REG_PORT_POWER_BIT);
	rtk_port_phyReg_set(4, PHY_CONTROL_REG, pData);


	LANWANPartition_8367r();

	rtl8367r_switch_inited = 1;
	return RT_ERR_OK;
}



void run_ethtest(int port, int mode)
{
	rtl8367r_switch_init();
	if (port < 1 || port > 4)
	{
		printf("wrong port number\n");
		return;
	}
	
	if (mode == 1)
	{
		rtk_port_phyTestMode_set(port,PHY_TEST_MODE_1);
	}
	else if (mode == 4)
	{
		rtk_port_phyTestMode_set(port,PHY_TEST_MODE_4);
	}
	else if (mode == 0)
	{
		rtk_port_phyTestMode_set(port,PHY_TEST_MODE_NORMAL);
	}
	else
	{
		printf("unknown mode\n");
	}
}


/*
#define RALINK_SYS_BASE			0xB0000000
#define RALINK_SYS_SYSCFG1 (RALINK_SYS_BASE + 0x14)
#define RALINK_USB_BASE			0xB01C0000
#define RALINK_USB_PORT0_TEST (RALINK_USB_BASE + 0x54)
#define RALINK_USB_PORT1_TEST (RALINK_USB_BASE + 0x58)

void run_usbtest(int port)
{
	u_long value;
	value = *(volatile u_long *)(RALINK_SYS_SYSCFG1);
	// change to HOST mode
	*(volatile u_long *)(RALINK_SYS_SYSCFG1) = value | (1U << 10);
	// check USB
	if (port == 0)
	{
		value = *(volatile u_long *)(RALINK_USB_PORT0_TEST);
		if (value != 0x1005)
		{
			printf("USB port 0 not inserted ?");
			return;
		}
		*(volatile u_long *)(RALINK_USB_PORT0_TEST) = 0x41005;
	}
	else if (port == 1)
	{
		value = *(volatile u_long *)(RALINK_USB_PORT1_TEST);
		if (value != 0x1005)
		{
			printf("USB port 1 not inserted ?");
			return;
		}
		*(volatile u_long *)(RALINK_USB_PORT1_TEST) = 0x41005;	
	}	
	else
	{
		printf("unknown mode\n");
	}	
}
*/





