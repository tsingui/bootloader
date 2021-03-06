/*
 * AMAZON_SE internal switch ethernet driver.
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <common.h>
#include "amazon_se_reg.h"

//#if (CONFIG_COMMANDS & CFG_CMD_NET) && defined(CONFIG_NET_MULTI) \
  //  && defined(CONFIG_AMAZON_SE_SWITCH)

#include <malloc.h>
#include <net.h>
#include <asm/addrspace.h>
//#include <asm/pinstrap.h>
//#include <boot.h>
#define SET_GPIO        1       //1, for gpio, 0 for dedicated

#define EPHY_CLOCK      0       //1 External clock, 0 internal clock

#define MII_MODE 1
#define REV_MII_MODE 2
#define  RED_MII_MODE    3
#define  EPHY_MODE      4


#define TX_CHAN_NO   3
#define RX_CHAN_NO   5

#define NUM_RX_DESC	PKTBUFSRX
#define NUM_TX_DESC	8
#define MAX_PACKET_SIZE 	1536
#define TOUT_LOOP	100
#define PHY0_ADDR       1 /*fixme: set the correct value here*/

//#define  RED_MII_MUX_WITH_SPI  1  /* remark by Zhang Yu */
#define IFX_CONFIG_ETHERNET_TYPE_RMII 1

#define DMA_WRITE_REG(reg, value) *((volatile u32 *)reg) = (u32)value
#define DMA_READ_REG(reg, value)    value = (u32)*((volatile u32*)reg)

#define SW_WRITE_REG(reg, value)  *((volatile u32*)reg) = (u32)value
#define SW_READ_REG(reg, value)   value = (u32)*((volatile u32*)reg)


#ifdef ENABLE_TRACE
#define TRACE(fmt,args...) printf("%s: " fmt, __FUNCTION__ , ##args)
#else
#define TRACE(fmt,args...)
#endif
/*
extern u8 g_mac_addr[];
extern u8 g_phy_addr[];
*/
int g_ethernet_mode = 0;
u8 g_mac_addr[]= {0x00,0x03,0x19,00,00,00};
u8 g_phy_addr[]= {0x00,0x01};
typedef struct
{
	union
	{
		struct
		{
			volatile u32 OWN                 :1;
			volatile u32 C	                 :1;
			volatile u32 Sop                 :1;
			volatile u32 Eop	         :1;
			volatile u32 reserved		 :3;
			volatile u32 Byteoffset		 :2;
			volatile u32 reserve             :7;
			volatile u32 DataLen             :16;
		}field;

		volatile u32 word;
	}status;

	volatile u32 DataPtr;
} amazon_se_rx_descriptor_t;

typedef struct
{
	union
	{
		struct
		{
			volatile u32 OWN                 :1;
			volatile u32 C	                 :1;
			volatile u32 Sop                 :1;
			volatile u32 Eop	         :1;
			volatile u32 Byteoffset		 :5;
			volatile u32 reserved            :7;
			volatile u32 DataLen             :16;
		}field;

		volatile u32 word;
	}status;

	volatile u32 DataPtr;
} amazon_se_tx_descriptor_t;




static amazon_se_rx_descriptor_t rx_des_ring[NUM_RX_DESC] __attribute__ ((aligned(8)));
static amazon_se_tx_descriptor_t tx_des_ring[NUM_TX_DESC] __attribute__ ((aligned(8)));
static int tx_num, rx_num;

int amazon_se_switch_init(struct eth_device *dev, bd_t * bis);
int amazon_se_switch_send(struct eth_device *dev, volatile void *packet,int length);
int amazon_se_switch_recv(struct eth_device *dev);
void amazon_se_switch_halt(struct eth_device *dev);
static int amazon_se_sw_chip_init (int mode, int gpio);
static void amazon_se_dma_init(void);

void
configureMiiRxClk (int external, int loopback)
{
	if (external == 1) {
		TRACE("\n Configured external Rx clk ");
//		*AMAZON_SE_CGU_IFCCR = (*AMAZON_SE_CGU_IFCCR | (0x3 << 28));
		*AMAZON_SE_CGU_IFCCR = (*AMAZON_SE_CGU_IFCCR & ~(0x3 << 28));
	}
	else {
		//from CGU
		if (loopback == 0) {
			TRACE("\n Configured internal Rx clk(from CGU) ");
			// clock from CGU
	//		*AMAZON_SE_CGU_IFCCR =
	//			(*AMAZON_SE_CGU_IFCCR & ~(0x3 << 28));
			*AMAZON_SE_CGU_IFCCR =
				(*AMAZON_SE_CGU_IFCCR | (0x3 << 28));
		}
		else {
			TRACE("\n Configured  Rx clk from PADs (loop back)");
			//looped back from PADs
		//	*AMAZON_SE_CGU_IFCCR =
		//		((*AMAZON_SE_CGU_IFCCR & ~(0x1 << 28)) |
//				 (0x1 << 29));
			*AMAZON_SE_CGU_IFCCR =
				((*AMAZON_SE_CGU_IFCCR | (0x1 << 28)) &
				 ~(0x1 << 29));
		}
	}
}


void
configureMiiTxClk (int external, int loopback)
{
	if (external == 1) {
		TRACE("\n Configured external Tx clk ");
	//	*AMAZON_SE_CGU_IFCCR = (*AMAZON_SE_CGU_IFCCR | (0x3 << 26));
		*AMAZON_SE_CGU_IFCCR = (*AMAZON_SE_CGU_IFCCR & ~(0x3 << 26));
	}
	else {
		if (loopback == 0) {
			TRACE("\n Configured internal Tx clk(from CGU) ");
			//from CGU
	//		*AMAZON_SE_CGU_IFCCR =
	//			(*AMAZON_SE_CGU_IFCCR & ~(0x3 << 26));
			*AMAZON_SE_CGU_IFCCR =
				(*AMAZON_SE_CGU_IFCCR | (0x3 << 26));
		}
		else {
			TRACE("\n Configured Tx clk from PADs (loop back)");
//			*AMAZON_SE_CGU_IFCCR =
//				((*AMAZON_SE_CGU_IFCCR & ~(0x1 << 26)) |
//				 (0x1 << 27));
			*AMAZON_SE_CGU_IFCCR =
				((*AMAZON_SE_CGU_IFCCR | (0x1 << 26)) &
				 ~(0x1 << 27));
		}
	}
}


void
configureRMiiRefClk (int external, int loopback)
{
	if (external == 1) {
		TRACE("\n Configured external Ref clk ");
	//	*AMAZON_SE_CGU_IFCCR = (*AMAZON_SE_CGU_IFCCR | (0x3 << 24));
		*AMAZON_SE_CGU_IFCCR = (*AMAZON_SE_CGU_IFCCR & ~(0x3 << 24));
	}
	else {
		if (loopback == 0)
		{
			TRACE("\n Configured internal Ref clk(from CGU) ");
		//	*AMAZON_SE_CGU_IFCCR =
		//		(*AMAZON_SE_CGU_IFCCR & ~(0x3 << 24));
			*AMAZON_SE_CGU_IFCCR =
				(*AMAZON_SE_CGU_IFCCR | (0x3 << 24));
		}
		else
		{
			TRACE("\n Configured Ref clk from PADs (loop back)");
		//	*AMAZON_SE_CGU_IFCCR =
		//		((*AMAZON_SE_CGU_IFCCR & ~(0x1 << 24)) |
		//		 (0x1 << 25));
			*AMAZON_SE_CGU_IFCCR =
				((*AMAZON_SE_CGU_IFCCR | (0x1 << 24)) &
				 ~(0x1 << 25));
		}
	}
}

#if 0
void
configurePhyClk (int enable)
{
	if (enable == 1)
	{
                printf("EPHY using internal clock\n");
		*AMAZON_SE_CGU_IFCCR = (*AMAZON_SE_CGU_IFCCR | (0x1 << 4));
	        *AMAZON_SE_CGU_OSC_CON = (*AMAZON_SE_CGU_OSC_CON ) & (~0x18);

        }
	else
	{
                  
                printf("EPHY using external clock\n");   
		*AMAZON_SE_CGU_IFCCR = (*AMAZON_SE_CGU_IFCCR & ~(0x1 << 4));
		*AMAZON_SE_CGU_OSC_CON = (*AMAZON_SE_CGU_OSC_CON ) | 0x18;
	}

}
#endif

void
configurePhyClk (int enable)
{
        if (enable == 1)
        {
                //Clock from External
                *AMAZON_SE_CGU_IFCCR = (*AMAZON_SE_CGU_IFCCR & ~(0x1 << 4));
                *AMAZON_SE_CGU_OSC_CON = (*AMAZON_SE_CGU_OSC_CON ) | 0x18;
                printf("External Clock\n");
        }
        else
        {
                //Clock from CGU
                *AMAZON_SE_CGU_IFCCR = (*AMAZON_SE_CGU_IFCCR | (0x1 << 4));
                *AMAZON_SE_CGU_OSC_CON = (*AMAZON_SE_CGU_OSC_CON ) & ~(0x18);
                printf("Internal Clock\n");
        }

}
/*------Beg add by zhangzhiyong 2008-8-22 for flash port and Rev_MII mode--------*/
/*-----------------------------------------------------------------------
 */
void enable_flash_port(int mode, int gpio)
{
		
        if (gpio == 0)
                return;

        if ( mode == MII_MODE )
        {

        } else if (mode == REV_MII_MODE )
        {  
                //set 12(FL_A23,P0_DIR.12 = 1)
                *AMAZON_SE_GPIO_P0_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P0_ALTSEL0 | (0x1<<12));
                *AMAZON_SE_GPIO_P0_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P0_ALTSEL1 & (~(0x1<<12)));
#if 1
                *AMAZON_SE_GPIO_P0_DIR = (*AMAZON_SE_GPIO_P0_DIR) | (0x1<<12);
                //set 19(FL_RD_WR),20(FL_A21),set 21(FL_A20),22(FL_A19),23(FL_A18),24(FL_BC1)
                //25(FL_A15),26(FL_A16),27(FL_BC0),set 28(FL_A17),29(FL_RD),P1_DIR.3~13 = 1
                *AMAZON_SE_GPIO_P1_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL0 | (0x7ff<<3));
                *AMAZON_SE_GPIO_P1_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL1 & (~(0x7ff<<3)));
                *AMAZON_SE_GPIO_P1_DIR = (*AMAZON_SE_GPIO_P1_DIR) | (0x7ff<<3);               
#else
                *AMAZON_SE_GPIO_P1_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL0 & 0xc907 )| 0x0900 ;
                *AMAZON_SE_GPIO_P1_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL1 & 0xffff) | 0x3ff8;
                *AMAZON_SE_GPIO_P0_OD = *(AMAZON_SE_GPIO_P0_OD) | 0xffff;
                *AMAZON_SE_GPIO_P1_DIR =  ((*AMAZON_SE_GPIO_P1_DIR) & (0xff9f) ) ;
#endif
                //printf("GPIO setting is done for flash port\n");

        } else if (mode == RED_MII_MODE)
        {
                // Red MII
                //Common  GPIO 0 (TXD0), 4(TXD1) & 13(TXEN)
                *AMAZON_SE_GPIO_P0_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P0_ALTSEL0 & 0xDFFE ) | 0x0010;
                *AMAZON_SE_GPIO_P0_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P0_ALTSEL1 & 0xffff) | 0x2011;
                *AMAZON_SE_GPIO_P0_OD = *(AMAZON_SE_GPIO_P0_OD) | 0x2011;
                 //GPIO24 (MDIO) & 27 (MDC)
                *AMAZON_SE_GPIO_P1_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL0 & 0xFFFF )  | 0x0900;
                *AMAZON_SE_GPIO_P1_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL1 & 0xffff) | 0x0900;
                *AMAZON_SE_GPIO_P1_DIR = (*AMAZON_SE_GPIO_P1_DIR) |(0x0900);

#if RED_MII_MUX_WITH_SPI
                //GPIO 7(RXDV),8(RXD1),9(RXD0) & 10(Ref Clock)
                *AMAZON_SE_GPIO_P0_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P0_ALTSEL0 & 0xF87F ) ;
                *AMAZON_SE_GPIO_P0_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P0_ALTSEL1 & 0xffff) | 0x0780;
                *AMAZON_SE_GPIO_P0_OD = *(AMAZON_SE_GPIO_P0_OD) | 0x0400;

                *AMAZON_SE_GPIO_P0_DIR = (*AMAZON_SE_GPIO_P0_DIR) & 0xFC7F;

                //printf(" SPI  Inactive\n");
#else
                //GPIO 21,22,23 & 25
                *AMAZON_SE_GPIO_P1_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL0 & 0xFD1F) ;
                *AMAZON_SE_GPIO_P1_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL1 & 0xFFFF ) | 0x02E0;
                *AMAZON_SE_GPIO_P1_OD = *(AMAZON_SE_GPIO_P1_OD) | 0x0040;
                *AMAZON_SE_GPIO_P1_DIR = (*AMAZON_SE_GPIO_P1_DIR & 0xFD5F);
                //printf(" FLASH Inactive\n");

#endif
        }	
}
/*-----------------------------------------------------------------------
 */
/*------End add by zhangzhiyong 2008-8-22 for flash port and Rev_MII mode--------*/
int
configureMiiGpioChan (int mode, int gpio)
{

        if (gpio == 0)
                return 0;

        if ( mode == MII_MODE )
        {

        } else if (mode == REV_MII_MODE )
        {
                //GPIO 0 (TXD0), 4(TXD1), 12 (RXDV) & 13(TXEN)
                *AMAZON_SE_GPIO_P0_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P0_ALTSEL0 & 0xCFFF ) | 0x0010;
                *AMAZON_SE_GPIO_P0_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P0_ALTSEL1 & 0xfffe) | 0x3011;
#if 1
                *AMAZON_SE_GPIO_P0_OD = *(AMAZON_SE_GPIO_P0_OD) | 0x3011;
                *AMAZON_SE_GPIO_P0_DIR = (*AMAZON_SE_GPIO_P0_DIR) & ~(0x3011);
                *AMAZON_SE_GPIO_P0_DIR = (*AMAZON_SE_GPIO_P0_DIR) | (0x1000);
                //GPIO 29 (TXD2), 28(TXD3)
                *AMAZON_SE_GPIO_P1_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL0 & 0xCFFE ) ;
                *AMAZON_SE_GPIO_P1_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL1 & 0xffff) | 0x3000;
                *AMAZON_SE_GPIO_P1_DIR = (*AMAZON_SE_GPIO_P1_DIR) | (0x3000);

                //GPIO 19 (RXD3), 20(RXD2), 23 (RXD1), 25 (RXD0), 24 (MDIO) & 27 (MDC)
                *AMAZON_SE_GPIO_P1_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL0 & 0xFD67 )  | 0x0900;
                *AMAZON_SE_GPIO_P1_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL1 & 0xffff) | 0x0B98;
                *AMAZON_SE_GPIO_P1_OD = *(AMAZON_SE_GPIO_P1_OD) | 0x0998;
                *AMAZON_SE_GPIO_P1_DIR = (*AMAZON_SE_GPIO_P1_DIR) |(0x0998);
                *AMAZON_SE_GPIO_P1_DIR = (*AMAZON_SE_GPIO_P1_DIR) &(~0x0200);

                //GPIO 21 (CRS), 22(RX Clk) & 26 (TX Clk)
                *AMAZON_SE_GPIO_P1_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL0 & 0xFB9F ) ;
                *AMAZON_SE_GPIO_P1_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL1 & 0xffff) | 0x0460;
                *AMAZON_SE_GPIO_P1_DIR =  ((*AMAZON_SE_GPIO_P1_DIR) & ~(0x0220) ) ;
#else
                *AMAZON_SE_GPIO_P1_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL0 & 0xc907 )| 0x0900 ;
                *AMAZON_SE_GPIO_P1_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL1 & 0xffff) | 0x3ff8;
                *AMAZON_SE_GPIO_P0_OD = *(AMAZON_SE_GPIO_P0_OD) | 0xffff;
                *AMAZON_SE_GPIO_P1_DIR =  ((*AMAZON_SE_GPIO_P1_DIR) & (0xff9f) ) ;
#endif
                printf("GPIO setting is done for REV MII\n");

        } else if (mode == RED_MII_MODE)
        {
                // Red MII
                //Common  GPIO 0 (TXD0), 4(TXD1) & 13(TXEN)
                *AMAZON_SE_GPIO_P0_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P0_ALTSEL0 & 0xDFFE ) | 0x0010;
                *AMAZON_SE_GPIO_P0_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P0_ALTSEL1 & 0xffff) | 0x2011;
                *AMAZON_SE_GPIO_P0_OD = *(AMAZON_SE_GPIO_P0_OD) | 0x2011;
                 //GPIO24 (MDIO) & 27 (MDC)
                *AMAZON_SE_GPIO_P1_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL0 & 0xFFFF )  | 0x0900;
                *AMAZON_SE_GPIO_P1_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL1 & 0xffff) | 0x0900;
                *AMAZON_SE_GPIO_P1_DIR = (*AMAZON_SE_GPIO_P1_DIR) |(0x0900);

#if RED_MII_MUX_WITH_SPI
                //GPIO 7(RXDV),8(RXD1),9(RXD0) & 10(Ref Clock)
                *AMAZON_SE_GPIO_P0_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P0_ALTSEL0 & 0xF87F ) ;
                *AMAZON_SE_GPIO_P0_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P0_ALTSEL1 & 0xffff) | 0x0780;
                *AMAZON_SE_GPIO_P0_OD = *(AMAZON_SE_GPIO_P0_OD) | 0x0400;

                *AMAZON_SE_GPIO_P0_DIR = (*AMAZON_SE_GPIO_P0_DIR) & 0xFC7F;

                printf(" SPI  Inactive\n");
#else
                //GPIO 21,22,23 & 25
                *AMAZON_SE_GPIO_P1_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL0 & 0xFD1F) ;
                *AMAZON_SE_GPIO_P1_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL1 & 0xFFFF ) | 0x02E0;
                *AMAZON_SE_GPIO_P1_OD = *(AMAZON_SE_GPIO_P1_OD) | 0x0040;
                *AMAZON_SE_GPIO_P1_DIR = (*AMAZON_SE_GPIO_P1_DIR & 0xFD5F);
                printf(" FLASH Inactive\n");

#endif


        }
	return 0;
}


void enableTurboMode ( void )
{
	*AMAZON_SE_PPE32_ETOP_CFG = *AMAZON_SE_PPE32_ETOP_CFG | 0x00000004;
}

void disableTurboMode ( void )
{
	*AMAZON_SE_PPE32_ETOP_CFG = *AMAZON_SE_PPE32_ETOP_CFG & 0xfffffffb;
}

static int
amazon_se_sw_chip_init (int mode, int gpio)
{

        int i;
        //unsigned short data=0;

/* 30 May 2007 start */
        *(volatile u32*)AMAZON_SE_PPE32_BASE=0x0;
        *AMAZON_SE_RCU_RST_REQ |= 0x100;   //Reset PPE module
/* 30 May 2007 end */


        *AMAZON_SE_PMU_PWDCR &= ~(1 << 13);     /*enable PPE  from PMU */
        for (i = 0; i < 0x5000; i++);

        g_ethernet_mode = mode;

        switch (mode) {

        case MII_MODE:
                configureMiiGpioChan (mode, gpio);
                configureMiiRxClk (1, 0);       //externel = 0, loop back 0
                configureMiiTxClk (1, 0);
                *AMAZON_SE_PPE32_ETOP_CFG =
                        *AMAZON_SE_PPE32_ETOP_CFG & 0xfffffffe;
                printf("Selected MII_MODE  \n");
                break;
        case REV_MII_MODE:
                *AMAZON_SE_PPE32_ETOP_CFG =
                        (*AMAZON_SE_PPE32_ETOP_CFG ) | 0x00000001;
                configureMiiRxClk (0, 0);       //externel = 0, loop back 0
                configureMiiTxClk (0, 0);
                configureMiiGpioChan (mode, gpio);
                *AMAZON_SE_PPE32_ETOP_CFG = (*AMAZON_SE_PPE32_ETOP_CFG) & ~(0x4000);
                *AMAZON_SE_CGU_OSC_CON = (*AMAZON_SE_CGU_OSC_CON ) & ~(0x18);
                *AMAZON_SE_PPE32_ETOP_CFG =
                        (*AMAZON_SE_PPE32_ETOP_CFG ) | 0x00000002;
                *AMAZON_SE_PPE32_ETOP_CFG =
                        *AMAZON_SE_PPE32_ETOP_CFG & 0xfffffffe;
#if 1
//LiuBin,reset switch 
                //set GPIO 29 to gpio func
                *AMAZON_SE_GPIO_P1_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL0) & 0xDFFF ;
                *AMAZON_SE_GPIO_P1_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL1) & 0xDFFF;

                *AMAZON_SE_GPIO_P1_DIR = (*AMAZON_SE_GPIO_P1_DIR) | (0x2000);  //set out put
		  *AMAZON_SE_GPIO_P1_OUT = (*AMAZON_SE_GPIO_P1_OUT) & (0xDFFF);  //set low
		  
		*AMAZON_SE_RCU_RST_REQ = (*AMAZON_SE_RCU_RST_REQ) | 0x1;  //Write low HRST to reset switch
		for (i = 0; i < 0x5000; i++);
		*AMAZON_SE_RCU_RST_REQ = (*AMAZON_SE_RCU_RST_REQ) & (0xFFFE);  //Write hight HRST to stop reseting switch
		
		//set GPIO 29 func to TXD2
                *AMAZON_SE_GPIO_P1_ALTSEL0 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL0) & 0xDFFF ;
                *AMAZON_SE_GPIO_P1_ALTSEL1 =
                        (*AMAZON_SE_GPIO_P1_ALTSEL1) | 0x2000;
                *AMAZON_SE_GPIO_P1_DIR = (*AMAZON_SE_GPIO_P1_DIR) & 0xDFFF;  //set TXD2 input	 	
#endif             
/*------Beg add by zhangzhiyong 2008-8-22 for flash port and Rev_MII mode--------*/
#ifndef CONFIG_SPI
									enable_flash_port(REV_MII_MODE,1);
#endif
/*------End add by zhangzhiyong 2008-8-22 for flash port and Rev_MII mode--------*/
                printf("Selected REV_MII_MODE \n");
                break;

        case RED_MII_MODE:
                *AMAZON_SE_PPE32_ETOP_CFG =
                        (*AMAZON_SE_PPE32_ETOP_CFG ) | 0x00000001;
                configureRMiiRefClk (0, 0);     //clock from CGU
                configureMiiGpioChan (mode, gpio);
                *AMAZON_SE_PPE32_ETOP_CFG = (*AMAZON_SE_PPE32_ETOP_CFG) & ~(0x4000);
                //enable Red Mii mode and clock is from CGU
                *AMAZON_SE_PPE32_ETOP_CFG =
                        (*AMAZON_SE_PPE32_ETOP_CFG)  | 0x00001000;
                *AMAZON_SE_CGU_OSC_CON = (*AMAZON_SE_CGU_OSC_CON ) & ~(0x18);
                //enable Red Mii mode and clock is from External
//              *AMAZON_SE_PPE32_ETOP_CFG =
//                      *AMAZON_SE_PPE32_ETOP_CFG  | 0x00009000;
                *AMAZON_SE_PPE32_ETOP_CFG =
                        *AMAZON_SE_PPE32_ETOP_CFG & 0xfffffffe;
		//Leon 07092804 disable MDIO auto-sensing mode
                *AMAZON_SE_PPE32_ETOP_MDIO_CFG &= ~(1 << 1);
#if 1
//LiuBin,reset switch 
                //set GPIO 29 to gpio func
               // *AMAZON_SE_GPIO_P1_ALTSEL0 =
                        //(*AMAZON_SE_GPIO_P1_ALTSEL0) & 0xDFFF ;
                //*AMAZON_SE_GPIO_P1_ALTSEL1 =
                        //(*AMAZON_SE_GPIO_P1_ALTSEL1) & 0xDFFF;

                //*AMAZON_SE_GPIO_P1_DIR = (*AMAZON_SE_GPIO_P1_DIR) | (0x2000);  //set out put
		  //*AMAZON_SE_GPIO_P1_OUT = (*AMAZON_SE_GPIO_P1_OUT) & (0xDFFF);  //set low
		  
		*AMAZON_SE_RCU_RST_REQ = (*AMAZON_SE_RCU_RST_REQ) | 0x1;  //Write low HRST to reset switch
		for (i = 0; i < 0x5000; i++);
		*AMAZON_SE_RCU_RST_REQ = (*AMAZON_SE_RCU_RST_REQ) & (0xFFFE);  //Write hight HRST to stop reseting switch
#endif
		printf("Selected RED_MII_MODE \n");
                break;
        case EPHY_MODE:
                *AMAZON_SE_PMU_PWDCR &= ~(1 << 7);      /*enable EPHY module  from PMU */
                for (i = 0; i < 0x500; i++);
                *AMAZON_SE_PPE32_ETOP_CFG =
                        (*AMAZON_SE_PPE32_ETOP_CFG ) | 0x00000001;
#if EPHY_CLOCK
                configurePhyClk (1);   //clock from External
#else
                configurePhyClk (0);   //clock from CGU
#endif
//              *AMAZON_SE_PPE32_ETOP_CFG =  0x4141;
                *AMAZON_SE_PPE32_ETOP_CFG = (*AMAZON_SE_PPE32_ETOP_CFG) |0x4000;
                //unsigned short data;
/* 08 June 2007 start */
#if SET_CLASS_A_VALUE
//              data = ephy_read_mdio_reg (EPHY_ADDRESS,  EPHY_SET_CLASS_VALUE_REG);
                ephy_write_mdio_reg (EPHY_ADDRESS, EPHY_SET_CLASS_VALUE_REG, 0xc020);
#endif
/* 08 June 2007 End*/
                printf("Selected EPHY_MODE \n");
                break;

        default:
                printf (" Switch Mode is not selected, Error!!! \n");
                *AMAZON_SE_PPE32_ETOP_CFG = (*AMAZON_SE_PPE32_ETOP_CFG) & ~(0x4000);

        }

/* 14 May 2007 Start */
//      *AMAZON_SE_PPE32_ETOP_IG_PLEN_CTRL = 0x4005ee;  // set packetlen.
        *AMAZON_SE_PPE32_ETOP_IG_PLEN_CTRL = 0x4005dc;  // set packetlen.
/* 14 May 2007 End*/
        *ENET_MAC_CFG |= 1 << 11;       /*enable the crc */

}

int amazon_se_switch_initialize(bd_t * bis)
{
	struct eth_device *dev;



	if (!(dev = (struct eth_device *) malloc (sizeof *dev)))
	{
		printf("Failed to allocate memory\n");
		return 0;
	}
	memset(dev, 0, sizeof(*dev));
	amazon_se_dma_init();
#ifdef IFX_CONFIG_ETHERNET_TYPE_EPHY
	amazon_se_sw_chip_init(EPHY_MODE, 0);
#endif
#ifdef IFX_CONFIG_ETHERNET_TYPE_RMII
	amazon_se_sw_chip_init(RED_MII_MODE, 1);
#endif
	sprintf(dev->name, "AMAZON_SE Switch");
	dev->init = amazon_se_switch_init;
	dev->halt = amazon_se_switch_halt;
	dev->send = amazon_se_switch_send;
	dev->recv = amazon_se_switch_recv;

	eth_register(dev);


	return 1;
}

int amazon_se_switch_init(struct eth_device *dev, bd_t * bis)
{
	int i;

	tx_num=0;
	rx_num=0;

		/* Reset DMA
		 */
//	serial_puts("i \n\0");

       *AMAZON_SE_DMA_CS=RX_CHAN_NO;
       *AMAZON_SE_DMA_CCTRL=0x2;/*fix me, need to reset this channel first?*/
       *AMAZON_SE_DMA_CPOLL= 0x80000040;
       /*set descriptor base*/
       *AMAZON_SE_DMA_CDBA=(u32)rx_des_ring;
       *AMAZON_SE_DMA_CDLEN=NUM_RX_DESC;
       *AMAZON_SE_DMA_CIE = 0;
       *AMAZON_SE_DMA_CCTRL=0x30000;

       *AMAZON_SE_DMA_CS=TX_CHAN_NO;
       *AMAZON_SE_DMA_CCTRL=0x2;/*fix me, need to reset this channel first?*/
       *AMAZON_SE_DMA_CPOLL= 0x80000040;
       *AMAZON_SE_DMA_CDBA=(u32)tx_des_ring;
       *AMAZON_SE_DMA_CDLEN=NUM_TX_DESC;
       *AMAZON_SE_DMA_CIE = 0;
       *AMAZON_SE_DMA_CCTRL=0x30100;

	for(i=0;i < NUM_RX_DESC; i++)
	{
		amazon_se_rx_descriptor_t * rx_desc = KSEG1ADDR(&rx_des_ring[i]);
		rx_desc->status.word=0;
		rx_desc->status.field.OWN=1;
		rx_desc->status.field.DataLen=PKTSIZE_ALIGN;   /* 1536  */
		rx_desc->DataPtr=(u32)KSEG1ADDR(NetRxPackets[i]);
	}

	for(i=0;i < NUM_TX_DESC; i++)
	{
		amazon_se_tx_descriptor_t * tx_desc = KSEG1ADDR(&tx_des_ring[i]);
		memset(tx_desc, 0, sizeof(tx_des_ring[0]));
	}
		/* turn on DMA rx & tx channel
		 */
	 *AMAZON_SE_DMA_CS=RX_CHAN_NO;
	 *AMAZON_SE_DMA_CCTRL|=1;/*reset and turn on the channel*/
	
	 udelay(2000000);
	return 0;
}

void amazon_se_switch_halt(struct eth_device *dev)
{
        int i;
        for(i=0;i<8;i++)
	{
	   *AMAZON_SE_DMA_CS=i;
	   *AMAZON_SE_DMA_CCTRL&=~1;/*stop the dma channel*/
	}
//	udelay(1000000);
}

int amazon_se_switch_send(struct eth_device *dev, volatile void *packet,int length)
{

	int                    	i;
	int 		 	res = -1;

	amazon_se_tx_descriptor_t * tx_desc= KSEG1ADDR(&tx_des_ring[tx_num]);

	if (length <= 0)
	{
		printf ("%s: bad packet size: %d\n", dev->name, length);
		goto Done;
	}

	for(i=0; tx_desc->status.field.OWN==1; i++)
	{
		if(i>=TOUT_LOOP)
		{
			printf("NO Tx Descriptor...");
			goto Done;
		}
	}

	//serial_putc('s');

	tx_desc->status.field.Sop=1;
	tx_desc->status.field.Eop=1;
	tx_desc->status.field.C=0;
	tx_desc->DataPtr = (u32)KSEG1ADDR(packet);
	if(length<60)
		tx_desc->status.field.DataLen = 60;
	else
		tx_desc->status.field.DataLen = (u32)length;

	asm("SYNC");
	tx_desc->status.field.OWN=1;

	res=length;
	tx_num++;
        if(tx_num==NUM_TX_DESC) tx_num=0;
	*AMAZON_SE_DMA_CS=TX_CHAN_NO;

	if(!(*AMAZON_SE_DMA_CCTRL & 1))
	*AMAZON_SE_DMA_CCTRL|=1;

Done:
	return res;
}

int amazon_se_switch_recv(struct eth_device *dev)
{

	int                    length  = 0;
      //  int tmp;
	amazon_se_rx_descriptor_t * rx_desc;
       // int anchor_num=0;
	//int i;
	for (;;)
	{
	        rx_desc = KSEG1ADDR(&rx_des_ring[rx_num]);

	        if ((rx_desc->status.field.C == 0) || (rx_desc->status.field.OWN == 1))
		{
		   break;
		}


		length = rx_desc->status.field.DataLen;
		if (length)
		{
                        /*tmp=(int)KSEG1ADDR(NetRxPackets[rx_num]);
			printf("%08x\n",tmp);
                        */
			NetReceive((void*)KSEG1ADDR(NetRxPackets[rx_num]), length - 4);
		//	serial_putc('*');
		}
		else
		{
			printf("Zero length!!!\n");
		}

		rx_desc->status.field.Sop=0;
		rx_desc->status.field.Eop=0;
		rx_desc->status.field.C=0;
		rx_desc->status.field.DataLen=PKTSIZE_ALIGN;
		rx_desc->status.field.OWN=1;
		rx_num++;
		if(rx_num==NUM_RX_DESC) rx_num=0;

	}

	return length;
}




static void amazon_se_dma_init(void)
{
//        int i;
//	serial_puts("d \n\0");

        *AMAZON_SE_PMU_PWDCR &=~(1<<5);/*enable DMA from PMU*/
		/* Reset DMA
		 */
	*AMAZON_SE_DMA_CTRL|=1;
        *AMAZON_SE_DMA_IRNEN=0;/*disable all the interrupts first*/

	/* Clear Interrupt Status Register
	*/
	*AMAZON_SE_DMA_IRNCR=0xfffff;
	/*disable all the dma interrupts*/
	*AMAZON_SE_DMA_IRNEN=0;
	/*disable channel 0 and channel 1 interrupts*/

	  *AMAZON_SE_DMA_CS=RX_CHAN_NO;
	  *AMAZON_SE_DMA_CCTRL=0x2;/*fix me, need to reset this channel first?*/
          *AMAZON_SE_DMA_CPOLL= 0x80000040;
          /*set descriptor base*/
          *AMAZON_SE_DMA_CDBA=(u32)rx_des_ring;
          *AMAZON_SE_DMA_CDLEN=NUM_RX_DESC;
          *AMAZON_SE_DMA_CIE = 0;
          *AMAZON_SE_DMA_CCTRL=0x30000;

	*AMAZON_SE_DMA_CS=TX_CHAN_NO;
	*AMAZON_SE_DMA_CCTRL=0x2;/*fix me, need to reset this channel first?*/
        *AMAZON_SE_DMA_CPOLL= 0x80000040;
	*AMAZON_SE_DMA_CDBA=(u32)tx_des_ring;
        *AMAZON_SE_DMA_CDLEN=NUM_TX_DESC;
	*AMAZON_SE_DMA_CIE = 0;
	*AMAZON_SE_DMA_CCTRL=0x30100;
	/*enable the poll function and set the poll counter*/
	//*AMAZON_SE_DMA_CPOLL=AMAZON_SE_DMA_POLL_EN | (AMAZON_SE_DMA_POLL_COUNT<<4);
	/*set port properties, enable endian conversion for switch*/
	*AMAZON_SE_DMA_PS=0;
	*AMAZON_SE_DMA_PCTRL|=0xf<<8;/*enable 32 bit endian conversion*/

	return;
}





//#endif
