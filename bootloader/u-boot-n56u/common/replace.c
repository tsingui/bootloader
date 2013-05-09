/**********************************************************
*	File:replace.c
*	This file includes a function that is used to replace the 
*	values in RF buffer.
*
*	return rc 	=0: replace successful
*						 !=0:	fail	
							
*
**********************************************************/

#include <common.h>
#include "replace.h"

int replace(unsigned long addr, uchar *value, int len)
{
	int idx = 0;
	uchar RFbuf[0x10000];
	int rc=0;
	memset(RFbuf,0,0x10000);
	if (0==len)
	{
		printf("\nThe value to replace is vacant!");
		return -1;
	}

	for (idx =0 ;idx <0x10000;idx ++)
		RFbuf[idx]=(*(uint8_t *) (CFG_FLASH_BASE + 0x40000 + idx));

	for (idx =0 ;idx <len; idx++)
		RFbuf[0x00000 + addr + idx] = value[idx];
	
	flash_sect_protect(0, CFG_FLASH_BASE + 0x40000, CFG_FLASH_BASE + 0x4ffff);
	
	printf("\n Erase File System block !!\n From %x To %x\n", CFG_FLASH_BASE + 0x40000, CFG_FLASH_BASE + 0x4ffff);
	flash_sect_erase(CFG_FLASH_BASE + 0x40000, CFG_FLASH_BASE + 0x4ffff);
	rc = flash_write((uchar *)RFbuf, (ulong)(CFG_FLASH_BASE + 0x40000), 0x10000);

	flash_sect_protect(1, CFG_FLASH_BASE + 0x40000, CFG_FLASH_BASE + 0x4ffff);

	printf("\nrc=%d\n",rc);
 	return rc;
}

uchar blver[] = "1006";

int chkVer()
{
	int idx, len;
	uchar rfbuf[4];
	int ret = 0;

	memset(rfbuf, 0x0, 4);
	for(idx = 0; idx < 4; ++idx)
		rfbuf[idx] = (*(uint8_t *) (CFG_FLASH_BASE + 0x4018a + idx));
/*
	printf("\n## pre-dump ver here:\n");	// tmp test
	for(idx = 0; idx < 4; ++idx)
		printf("[%2x] ", rfbuf[idx]);
	printf("\n");

        printf("\n## bl ver here:\n");    // tmp test
        for(idx = 0; idx < 4; ++idx)
                printf("[%2x] ", blver[idx]);
        printf("\n");
*/
        printf("\nBootloader version: %c.%c.%c.%c\n", blver[0], blver[1], blver[2], blver[3]);

//	if((rfbuf[0] == 0x31) && (rfbuf[1] == 0x30) && (rfbuf[2] == 0x30) && (rfbuf[3] == 0x30))
	if((rfbuf[0] == blver[0]) && (rfbuf[1] == blver[1]) && (rfbuf[2] == blver[2]) && (rfbuf[3] == blver[3]))
//	if (!memcmp(rfbuf, blver, 4))
	{
//		printf("# do nothing#\n");
		return 1;
	}
	else
	{
//		printf("# rewrite version id as %s #\n", blver);
		return 0;
	}
}

int chkMAC()
{	
	int idx, len;
	uchar rfbuf[0x06];
	int ret = 0;

	memset(rfbuf, 0, 0x06);
	for(idx = 0; idx < 0x06; ++idx)
		rfbuf[idx] = (*(uint8_t *) (CFG_FLASH_BASE + 0x40000 + 4 + idx));

//	printf("CFG_FLASH_BASE: %x\n", CFG_FLASH_BASE);
//	printf("Dump MAC address from %x: [%02X:%02X:%02X:%02X:%02X:%02X]\n",
	printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
//		CFG_FLASH_BASE + 0x40000 + 4,
		(unsigned char)rfbuf[0],(unsigned char)rfbuf[1],(unsigned char)rfbuf[2],
		(unsigned char)rfbuf[3],(unsigned char)rfbuf[4],(unsigned char)rfbuf[5]);

	if((rfbuf[0] == 0xff) && (rfbuf[1] == 0xff) && (rfbuf[2] == 0xff) && (rfbuf[3] == 0xff) && (rfbuf[4] == 0xff) && (rfbuf[5] == 0xff))
	{
		printf("\ninvalid mac ff:ff:ff:ff:ff:ff\n");
		return -2;
	}

	if(rfbuf[0] & 0x01)
	{
		printf("\nerr mac with head 01\n");
		return -1;
	}
}
