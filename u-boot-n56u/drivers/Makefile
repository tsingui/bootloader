#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

# CFLAGS += -DET_DEBUG -DDEBUG

LIB	= libdrivers.a

OBJS	= rt2880_eth.o i2c_drv.o mii_mgr.o myrtk.o

OBJS += smi.o

OBJS += rtk_api.o
OBJS += rtl8367b_asicdrv.o
OBJS += rtl8367b_asicdrv_acl.o
OBJS += rtl8367b_asicdrv_cputag.o
OBJS += rtl8367b_asicdrv_dot1x.o
OBJS += rtl8367b_asicdrv_eee.o
#OBJS += rtl8370_asicdrv_eeelldp.o
OBJS += rtl8367b_asicdrv_fc.o
OBJS += rtl8367b_asicdrv_green.o
OBJS += rtl8367b_asicdrv_igmp.o
OBJS += rtl8367b_asicdrv_inbwctrl.o
OBJS += rtl8367b_asicdrv_interrupt.o
OBJS += rtl8367b_asicdrv_hsb.o
OBJS += rtl8367b_asicdrv_led.o
OBJS += rtl8367b_asicdrv_lut.o
OBJS += rtl8367b_asicdrv_meter.o
OBJS += rtl8367b_asicdrv_mib.o
OBJS += rtl8367b_asicdrv_mirror.o
OBJS += rtl8367b_asicdrv_misc.o
#OBJS += rtl8370_asicdrv_oam.o
OBJS += rtl8367b_asicdrv_phy.o
OBJS += rtl8367b_asicdrv_port.o
OBJS += rtl8367b_asicdrv_portIsolation.o
OBJS += rtl8367b_asicdrv_qos.o
#OBJS += rtl8370_asicdrv_rldp.o
OBJS += rtl8367b_asicdrv_rma.o
#OBJS += rtl8370_asicdrv_rrcp.o
OBJS += rtl8367b_asicdrv_scheduling.o
#OBJS += rtl8370_asicdrv_specialCongest.o
OBJS += rtl8367b_asicdrv_storm.o
OBJS += rtl8367b_asicdrv_svlan.o
OBJS += rtl8367b_asicdrv_trunking.o
OBJS += rtl8367b_asicdrv_unknownMulticast.o
OBJS += rtl8367b_asicdrv_vlan.o
OBJS += rtl8367b_asicdrv_eav.o
OBJS += mysmi.o


ifeq ($(CFG_ENV_IS), IN_FLASH)
OBJS	+= spi_drv.o 
endif

ifeq ($(CFG_ENV_IS), IN_SPI)
OBJS	+= spi_flash.o 
 ifeq ($(RT3883_MP), y)
 OBJS	+= spi_drv.o 
 endif
 ifeq ($(RT3352_MP), y)
 OBJS	+= spi_drv.o 
 endif
endif

ifeq ($(CFG_ENV_IS), IN_NAND)
OBJS	+= nand_flash.o 
 ifeq ($(RT3883_MP), y)
 OBJS	+= spi_drv.o 
 endif
endif

ifeq ($(RALINK_OHCI), ON)
OBJS	+= ohci-hcd.o
endif

ifeq ($(RALINK_EHCI), ON)
OBJS	+= ehci-hcd.o ehci-ra.o
endif

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) crv $@ $(OBJS)

#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
