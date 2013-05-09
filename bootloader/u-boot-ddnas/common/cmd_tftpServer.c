/*
*	TFTP server     File: cmd_tftpServer.c
*/

#include <cmd_tftpServer.h>


//extern  inline int raspi_unprotect(void);

#ifdef CFG_DIRECT_FLASH_TFTP
extern flash_info_t flash_info[CFG_MAX_FLASH_BANKS];/* info for FLASH chips   */
#endif


static int	TftpServerPort;		/* The UDP port at their end		*/
static int	TftpOurPort;		/* The UDP port at our end		*/
static int	TftpTimeoutCount;
static ulong	TftpBlock;		/* packet sequence number		*/
static ulong	TftpLastBlock;		/* last packet sequence number received */
static ulong	TftpBlockWrapOffset;	/* memory offset due to wrapping	*/
static int	TftpState;

uint8_t asuslink[] = "ASUSSPACELINK";
uint8_t maclink[] = "snxxxxxxxxxxx";
#define PTR_SIZE	0x400000	// 0x390000
#define BOOTBUF_SIZE	0x30000
uint8_t ptr[PTR_SIZE], bootbuf[BOOTBUF_SIZE], MAC0[13], RDOM[7], ASUS[24], nvramcmd[60];
uint16_t RescueAckFlag = 0;
uint32_t copysize = 0;
uint32_t offset = 0;
int rc = 0;
int MAC_FLAG = 0;
static char default_filename[DEFAULT_NAME_LEN];
int env_loc = 0;

static void _tftpd_open(void);
static void TftpHandler(uchar * pkt, unsigned dest, unsigned src, unsigned len);
static void RAMtoFlash(void);
static void write_linux(void);
extern image_header_t header;
//extern int do_bootm(cmd_tbl_t *, int, int, char *[]);
//extern int do_reset(cmd_tbl_t *, int, int, char *[]);

extern IPaddr_t TempServerIP;

//extern void print_image_hdr(image_header_t *hdr);	// tmp test

ulong def_load_addr = 0xBC050000;

#if (CONFIG_COMMANDS & CFG_CMD_TFTPSERVER)
int check_image(void)
{
	if (*(unsigned long *) (ptr) == TRX_MAGIC && (*(unsigned long *) (ptr+0x40) != UBOOT_MAGIC))	
	  return 1;
	else
	  return 0;
}

static void TftpdSend(void)
{
	volatile uchar *pkt;
	volatile uchar *xp;
	int	len = 0;
	/*
	*	We will always be sending some sort of packet, so
	*	cobble together the packet headers now.
	*/
	pkt = NetTxPacket + NetEthHdrSize() + IP_HDR_SIZE;
	
	switch (TftpState) 
	{
	case STATE_RRQ:
		xp = pkt;
		*((ushort *)pkt)++ = htons(TFTP_DATA);
		*((ushort *)pkt)++ = htons(TftpBlock);/*fullfill the data part*/
		len = pkt - xp;
		break;
		
	case STATE_WRQ:
		xp = pkt;
		*((ushort *)pkt)++ = htons(TFTP_ACK);
		*((ushort *)pkt)++ = htons(TftpBlock);
		len = pkt - xp;
		break;
		
#ifdef ET_DEBUG
		printf("send option \"timeout %s\"\n", (char *)pkt);
#endif
		pkt += strlen((char *)pkt) + 1;
		len = pkt - xp;
		break;
		
	case STATE_DATA:
		xp = pkt;
		*((ushort *)pkt)++ = htons(TFTP_ACK);
		*((ushort *)pkt)++ = htons(TftpBlock);
		len = pkt - xp;
		break;
		
	case STATE_FINISHACK:
		xp = pkt;
		*((ushort *)pkt)++ = htons(TFTP_FINISHACK);
		*((ushort *)pkt)++ = htons(RescueAckFlag);
		len = pkt - xp;
		break;
		
	case STATE_TOO_LARGE:
		xp = pkt;
		*((ushort *)pkt)++ = htons(TFTP_ERROR);
		*((ushort *)pkt)++ = htons(3);
		strcpy((char *)pkt, "File too large");
		pkt += 14 /*strlen("File too large")*/ + 1;
		len = pkt - xp;
		break;
		
	case STATE_BAD_MAGIC:
		xp = pkt;
		*((ushort *)pkt)++ = htons(TFTP_ERROR);
		*((ushort *)pkt)++ = htons(2);
		strcpy((char *)pkt, "File has bad magic");
		pkt += 18 /*strlen("File has bad magic")*/ + 1;
		len = pkt - xp;
		break;
	}
	
	NetSendUDPPacket(NetServerEther, NetServerIP, TftpServerPort, TftpOurPort, len);
}

static void TftpTimeout(void)
{
	puts("T ");
	NetSetTimeout(TIMEOUT * CFG_HZ, TftpTimeout);
	TftpdSend();
}
	
	
U_BOOT_CMD(
	tftpd, 1, 1, do_tftpd,
	"tftpd\t -Start tftpd server for firmware rescue.\n",
	NULL
);
	
	
int do_tftpd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	   printf(" \n## Enter Rescue Mode ##\n");		
		setenv("autostart", "no");
		/* Wait forever for an image */
		if (NetLoop(TFTPD) < 0) 
			return 1;
	
	return 0;
}
	
	
	
void TftpdStart(void)
{
	
#if defined(CONFIG_NET_MULTI)
	printf("Using %s device\n", eth_get_name());
#endif
	//puts(" \nTFTP from server ");	print_IPaddr(NetServerIP);
	puts("\nOur IP address is:(");	
	print_IPaddr(NetOurIP);
	puts(")\nWaiting for TFTP client request...\n");
	/* Check if we need to send across this subnet */
	if (NetOurGatewayIP && NetOurSubnetMask) 
	{
		IPaddr_t OurNet 	= NetOurIP    & NetOurSubnetMask;
		IPaddr_t ServerNet 	= NetServerIP & NetOurSubnetMask;
		
		if (OurNet != ServerNet)
		{
			puts("; sending through gateway ");
			print_IPaddr(NetOurGatewayIP) ;
		}
	}

	memset(ptr,0,sizeof(ptr));
	_tftpd_open();
}
	
static void _tftpd_open()
{
  	printf("tftpd server by Lintel<lintel.huang@gmail.com>..\n");
	printf("tftpd server start...\n");	// tmp test
	NetSetTimeout(TIMEOUT * CFG_HZ * 2, TftpTimeout);
    	NetSetHandler(TftpHandler);
	
	TftpOurPort = PORT_TFTP;
	TftpTimeoutCount = 0;
	TftpState = STATE_RRQ;
	TftpBlock = 0;
	
	/* zero out server ether in case the server ip has changed */
	memset(NetServerEther, 0, 6);
}
	
	
static void TftpHandler(uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	ushort proto;
	int i;
	static int ledcount=0,ledstate=1;

	//printf("\ntftp handler 1\n");	// tmp test
	
	if (dest != TftpOurPort) 
	{
		return;
	}
	/* don't care the packets that donot send to TFTP port */
	
	if (TftpState != STATE_RRQ && src != TftpServerPort)
	{
		return;
	}
	
	if (len < 2)
	{
		return;
	}
	
	#if 0
	if (0==ledcount%64)/*the led flicker when packet received*/
		{
			ledstate+=1;
			ledstate%=2;
		}
	if(0==ledstate%2)
		LEDON(11);
	else
		LEDOFF(11);
	++ledcount;
	if (0xffffff==i)
		i=0;
	#endif

	len -= 2;
	/* warning: don't use increment (++) in ntohs() macros!! */
	proto = *((ushort *)pkt)++;

	switch (ntohs(proto))
	{
	case TFTP_RRQ:

		printf("\n Get read request from:(");
		print_IPaddr(TempServerIP);
		printf(")\n");
		NetCopyIP(&NetServerIP,&TempServerIP);

		TftpServerPort = src;
		TftpBlock = 1;
		TftpBlockWrapOffset = 0;		
		TftpState = STATE_RRQ;
		
		for (i=0; i<13; i++) 
		{
			if (*((uint8_t *)pkt)++ != asuslink[i])
				break;
		}
		if (i==13) 
		{ /* it's the firmware transmitting situation */
			/* here get the IP address from the first packet. */
			NetOurIP = (*((uint8_t *)pkt)++) & 0x000000ff;
			NetOurIP<<=8;
			NetOurIP|= (*((uint8_t *)pkt)++) & 0x000000ff;
			NetOurIP<<=8;
			NetOurIP|= (*((uint8_t *)pkt)++) & 0x000000ff;
			NetOurIP<<=8;
			NetOurIP|= (*((uint8_t *)pkt)++) & 0x000000ff;
		}
		else
		{
			for (i=0; i<13; i++)
			{
				if (*((uint8_t *)pkt)++ != maclink[i])
					break;
			}
			if(i==13)
			{
				/* here get the IP address from the first packet. */
				NetOurIP = (*((uint8_t *)pkt)++)& 0x000000ff;
				NetOurIP<<=8;
				NetOurIP|=(*((uint8_t *)pkt)++)& 0x000000ff;
				NetOurIP<<=8;
				NetOurIP|=(*((uint8_t *)pkt)++)& 0x000000ff;
				NetOurIP<<=8;
				NetOurIP|=(*((uint8_t *)pkt)++)& 0x000000ff;
			}
		}
		
		TftpdSend();//send a vacant Data packet as a ACK
		break;
		
	case TFTP_WRQ:
		TftpServerPort = src;
		TftpBlock = 0;
		TftpState = STATE_WRQ;
		TftpdSend();
		break;
		
	case TFTP_DATA:
		//printf("case TFTPDATA\n");	// tmp test
		if (len < 2)
			return;
		len -= 2;
		TftpBlock = ntohs(*(ushort *)pkt);
		/*
		* RFC1350 specifies that the first data packet will
		* have sequence number 1. If we receive a sequence
		* number of 0 this means that there was a wrap
		* around of the (16 bit) counter.
		*/
		if (TftpBlock == 0)
		{
			printf("\n\t %lu MB reveived\n\t ", TftpBlockWrapOffset>>20);
		} 
		else 
		{
			if (((TftpBlock - 1) % 10) == 0) 
			{/* print out progress bar */
				puts("#");
			} 
			else
				if ((TftpBlock % (10 * HASHES_PER_LINE)) == 0)
				{
					puts("\n");
				}
		}
		if (TftpState == STATE_WRQ)
		{		
			/* first block received */
			TftpState = STATE_DATA;
			TftpServerPort = src;
			TftpLastBlock = 0;
			TftpBlockWrapOffset = 0;
			printf("\n First block received  \n");
			//printf("Load Addr is %x\n", ptr);	// tmp test
			//ptr = 0x80100000;
			
			if (TftpBlock != 1)
			{	/* Assertion */
				printf("\nTFTP error: "
					"First block is not block 1 (%ld)\n"
					"Starting again\n\n",
					TftpBlock);
				NetStartAgain();
				break;
			}
		}
		
		if (TftpBlock == TftpLastBlock)
		{	/* Same block again; ignore it. */
			printf("\n Same block again; ignore it \n"); 
			break;
		}
		TftpLastBlock = TftpBlock;
		NetSetTimeout(TIMEOUT * CFG_HZ, TftpTimeout);
		
      		offset = (TftpBlock - 1) * TFTP_BLOCK_SIZE;
		copysize = offset + len;/* the total length of the data */

		if(copysize > PTR_SIZE)		// tmp test for upgrade fw
		{
			printf("!! OVER PTR SIZE\n");	// tmp test
		}
		
		(void)memcpy((void *)(ptr+ offset), pkt+2, len);/* store the data part to RAM */
		
		/*
		*	Acknowledge the block just received, which will prompt
		*	the Server for the next one.
		*/
		TftpdSend();
		
		if (len < TFTP_BLOCK_SIZE)
		{
		/*
		*	We received the whole thing.  Try to run it.
		*/
			puts("\ndone\n");
			TftpState = STATE_FINISHACK;
			NetState = NETLOOP_SUCCESS;
			RAMtoFlash();
		}
		break;
		
	case TFTP_ERROR:
		printf("\nTFTP error: '%s' (%d)\n",
			pkt + 2, ntohs(*(ushort *)pkt));
		puts("Starting again\n\n");
		NetStartAgain();
		break;
		
	default:
		break;
		
	}
}


static void RAMtoFlash(void)
{
	int i=0, parseflag=0, j=0;
	uchar mac_temp[7],secretcode[9];
	uint8_t SCODE[5] = {0x53, 0x43, 0x4F, 0x44, 0x45};
	char *end ,*tmp;
	
	printf("RAM to FLASH\n");	// tmp test
    /* Check for the TRX magic. If it's a TRX image, then proceed to flash it. */
//	if (*(unsigned long *) (ptr) == TRX_MAGIC)
		printf("Download of 0x%x bytes completed\n", copysize);
	if (check_image())
	{
		printf("Image is a Linux image!\n");	// tmp test
		printf("will write linux to FLASH...\n");
		write_linux();
	}
	else
	{
		printf("Download is not Firmware.\n");	// tmp test
		if (copysize>0 && copysize<=0x30000)
		// uboot
		{
			if ( (*(unsigned long *) (ptr+0x40) == UBOOT_MAGIC))
			{
				printf("File is bootloader image...\n");
				parseflag = 1;
			}
			else 
			{
				parseflag = -1;	
				printf("Warning, Bad Magic word!!\n");
				NetState=STATE_BAD_MAGIC;
				TftpdSend();
				copysize = 0;
			}
			
		}
		else if (copysize > 0x30000)
		{
			parseflag = 0;
			printf("    Download of 0x%x bytes completed\n", copysize);
			printf("Write kernel and filesystem binary to FLASH (0xbfc30000)\n");
		}
		else 
		{
			parseflag = -1;
			copysize = 0;
			printf("Downloading image time out\n");
		}	
		
		if (copysize == 0) 
			return;    /* 0 bytes, don't flash */
		
		if (parseflag != 0)
			copysize = 0x30000;
		
		printf("  BootLoader Programming...\n");
		
		if (parseflag == 1)
		{
			printf("\n Erase Uboot block !!\n From 0xBC000000 To 0xBC02FFFF\n");
			raspi_erase(0, CFG_BOOTLOADER_SIZE);
			rc = raspi_write((uchar *)ptr, 0, copysize);

		}
		
		if (!rc)
		{
			printf("rescue failed!\n");
//			flash_perror(rc);
			NetState = NETLOOP_FAIL; 
			/* Network loop state */
			TftpState = STATE_FINISHACK;
			RescueAckFlag= 0x0000;
			for (i=0; i<6; i++)
				TftpdSend();
			return;			
		}
		else
		{
			printf("done. %d bytes written\n",copysize);
			TftpState = STATE_FINISHACK;
			RescueAckFlag= 0x0001;
			for (i=0; i<6; i++)
				TftpdSend();
			if (parseflag != 0)
			{
				printf("SYSTEM RESET \n\n");
				//NetCopyIP(&NetOurIP,&((IPaddr_t)(0x0101A8C0)));
				NetOurIP=(IPaddr_t)0x0101A8C0;
				//printf("\nTempOurIP=%x",(ulong)NetOurIP);
				udelay(500);
				do_reset(NULL, 0, 0, NULL);
			}
			return;
		}
	}
}

static void write_linux(void)
{
	int  i = 0;
	int rrc = 0;
	ulong	dest = 0, count = 0;
	int e_end = 0;

	printf("Write linux!\n");	// tmp test
    	load_addr = ptr;
	
	if (!check_image())
	{
		printf("Check linux image error! SYSTEM RESET!!!\n\n");
		udelay(500);
		do_reset (NULL, 0, 0, NULL);
	}
	else
	{
		count = copysize;
		
		if (count == 0) 
		{
			puts ("Zero length ???\n");
			return;
		}
		
		e_end = CFG_KERN_ADDR + count;
		printf("first e_end is %x\n", e_end);	// tmp test


		printf("\n Erase linux kernel block !!\n");
		printf("From 0x%X length 0x%X\n", CFG_KERN_ADDR - CFG_FLASH_BASE,
				e_end);
		rrc= raspi_erase((CFG_KERN_ADDR - CFG_FLASH_BASE),(e_end - CFG_KERN_ADDR));
				
		printf ("\n Copy %d bytes to Flash... \n", count);		
		rrc = raspi_write((uchar *)ptr,(CFG_KERN_ADDR - CFG_FLASH_BASE),(e_end - CFG_KERN_ADDR));
//			raspi_write((char *)addr, dest, count);

		//flash_sect_protect(1, 0xbc450000, 0xbc7fffff);	// disable for tmp
		
		if (rrc) 
		{
			printf("done. %d bytes written\n", count);
			TftpState = STATE_FINISHACK;
			RescueAckFlag= 0x0001;
			for (i=0; i<6; i++)
				TftpdSend();
			printf("\nSYSTEM RESET!!!\n\n");
			udelay(500);
			do_reset(NULL, 0, 0, NULL);
			return;
		}
		else
		{
#if 0
			printf("rescue failed!\n");
			flash_perror(rrc);
			NetState = NETLOOP_FAIL;
			TftpState = STATE_FINISHACK;
			RescueAckFlag= 0x0000;
			for (i=0; i<6; i++)
				TftpdSend();
			return;
#endif
		      	printf("rescue failed!\n");
			printf("\nSYSTEM RESET!!!\n\n");
			udelay(500);
			do_reset(NULL, 0, 0, NULL);
			return;
		}
	}
}

#endif
