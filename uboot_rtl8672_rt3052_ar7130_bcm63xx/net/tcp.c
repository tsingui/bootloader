/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*   Copyright (C) 2001-2003 by Texas Instruments, Inc.  All rights reserved.  */
/*   Copyright (C) 2001-2003 Telogy Networks, Inc.							   */
/*                                                                             */
/*   NOTE: THIS VERSION OF CODE IS MAINTAINED BY TELOGY NETWORKS AND NOT TI!   */
/*                                                                             */
/*     IMPORTANT - READ CAREFULLY BEFORE PROCEEDING TO USE SOFTWARE.           */
/*                                                                             */
/*  This document is displayed for you to read prior to using the software     */
/*  and documentation.  By using the software and documentation, or opening    */
/*  the sealed packet containing the software, or proceeding to download the   */
/*  software from a Bulletin Board System(BBS) or a WEB Server, you agree to   */
/*  abide by the following Texas Instruments License Agreement. If you choose  */
/*  not to agree with these provisions, promptly discontinue use of the        */
/*  software and documentation and return the material to the place you        */
/*  obtained it.                                                               */
/*                                                                             */
/*                               *** NOTE ***                                  */
/*                                                                             */
/*  The licensed materials contain MIPS Technologies, Inc. confidential        */
/*  information which is protected by the appropriate MIPS Technologies, Inc.  */
/*  license agreement.  It is your responsibility to comply with these         */
/*  licenses.                                                                  */
/*                                                                             */
/*                   Texas Instruments License Agreement                       */
/*                                                                             */
/*  1. License - Texas Instruments (hereinafter "TI"), grants you a license    */
/*  to use the software program and documentation in this package ("Licensed   */
/*  Materials") for Texas Instruments broadband products.                      */
/*                                                                             */
/*  2. Restrictions - You may not reverse-assemble or reverse-compile the      */
/*  Licensed Materials provided in object code or executable format.  You may  */
/*  not sublicense, transfer, assign, rent, or lease the Licensed Materials    */
/*  or this Agreement without written permission from TI.                      */
/*                                                                             */
/*  3. Copyright - The Licensed Materials are copyrighted. Accordingly, you    */
/*  may either make one copy of the Licensed Materials for backup and/or       */
/*  archival purposes or copy the Licensed Materials to another medium and     */
/*  keep the original Licensed Materials for backup and/or archival purposes.  */
/*                                                                             */
/*  4. Runtime and Applications Software - You may create modified or          */
/*  derivative programs of software identified as Runtime Libraries or         */
/*  Applications Software, which, in source code form, remain subject to this  */
/*  Agreement, but object code versions of such derivative programs are not    */
/*  subject to this Agreement.                                                 */
/*                                                                             */
/*  5. Warranty - TI warrants the media to be free from defects in material    */
/*  and workmanship and that the software will substantially conform to the    */
/*  related documentation for a period of ninety (90) days after the date of   */
/*  your purchase. TI does not warrant that the Licensed Materials will be     */
/*  free from error or will meet your specific requirements.                   */
/*                                                                             */
/*  6. Remedies - If you find defects in the media or that the software does   */
/*  not conform to the enclosed documentation, you may return the Licensed     */
/*  Materials along with the purchase receipt, postage prepaid, to the         */
/*  following address within the warranty period and receive a refund.         */
/*                                                                             */
/*  TEXAS INSTRUMENTS                                                          */
/*  Application Specific Products, MS 8650                                     */
/*  c/o ADAM2 Application Manager                                              */
/*  12500 TI Boulevard                                                         */
/*  Dallas, TX 75243  - U.S.A.                                                 */
/*                                                                             */
/*  7. Limitations - TI makes no warranty or condition, either expressed or    */
/*  implied, including, but not limited to, any implied warranties of          */
/*  merchantability and fitness for a particular purpose, regarding the        */
/*  licensed materials.                                                        */
/*                                                                             */
/*  Neither TI nor any applicable licensor will be liable for any indirect,    */
/*  incidental or consequential damages, including but not limited to loss of  */
/*  profits.                                                                   */
/*                                                                             */
/*  8. Term - The license is effective until terminated.   You may terminate   */
/*  it at any other time by destroying the program together with all copies,   */
/*  modifications and merged portions in any form. It also will terminate if   */
/*  you fail to comply with any term or condition of this Agreement.           */
/*                                                                             */
/*  9. Export Control - The re-export of United States origin software and     */
/*  documentation is subject to the U.S. Export Administration Regulations or  */
/*  your equivalent local regulations. Compliance with such regulations is     */
/*  your responsibility.                                                       */
/*                                                                             */
/*                         *** IMPORTANT NOTICE ***                            */
/*                                                                             */
/*  Texas Instruments (TI) reserves the right to make changes to or to         */
/*  discontinue any semiconductor product or service identified in this        */
/*  publication without notice. TI advises its customers to obtain the latest  */
/*  version of the relevant information to verify, before placing orders,      */
/*  that the information being relied upon is current.                         */
/*                                                                             */
/*  TI warrants performance of its semiconductor products and related          */
/*  software to current specifications in accordance with TI's standard        */
/*  warranty. Testing and other quality control techniques are utilized to     */
/*  the extent TI deems necessary to support this warranty. Unless mandated    */
/*  by government requirements, specific testing of all parameters of each     */
/*  device is not necessarily performed.                                       */
/*                                                                             */
/*  Please be aware that Texas Instruments products are not intended for use   */
/*  in life-support appliances, devices, or systems. Use of a TI product in    */
/*  such applications without the written approval of the appropriate TI       */
/*  officer is prohibited. Certain applications using semiconductor devices    */
/*  may involve potential risks of injury, property damage, or loss of life.   */
/*  In order to minimize these risks, adequate design and operating            */
/*  safeguards should be provided by the customer to minimize inherent or      */
/*  procedural hazards. Inclusion of TI products in such applications is       */
/*  understood to be fully at the risk of the customer using TI devices or     */
/*  systems.                                                                   */
/*                                                                             */
/*  TI assumes no liability for TI applications assistance, customer product   */
/*  design, software performance, or infringement of patents or services       */
/*  described herein. Nor does TI warrant or represent that license, either    */
/*  expressed or implied, is granted under any patent right, copyright, mask   */
/*  work right, or other intellectual property right of TI covering or         */
/*  relating to any combination, machine, or process in which such             */
/*  semiconductor products or services might be or are used.                   */
/*                                                                             */
/*  All company and/or product names are trademarks and/or registered          */
/*  trademarks of their respective manaufacturers.                             */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/* tinytcp.c - Tiny Implementation of the Transmission Control Protocol        */
/*                                                                             */
/* Written March 28, 1986 by Geoffrey Cooper, IMAGEN Corporation.              */
/*                                                                             */
/* This code is a small implementation of the TCP and IP protocols, suitable   */
/* for burning into ROM.  The implementation is bare-bones and represents      */
/* two days' coding efforts.  A timer and an ethernet board are assumed.  The  */
/* implementation is based on busy-waiting, but the tcp_handler procedure      */
/* could easily be integrated into an interrupt driven scheme.                 */
/*                                                                             */
/* IP routing is accomplished on active opens by broadcasting the tcp SYN      */
/* packet when ARP mapping fails.  If anyone answers, the ethernet address     */
/* used is saved for future use.  This also allows IP routing on incoming      */
/* connections.								   								   */
/*                                                                             */ 
/* The TCP does not implement urgent pointers (easy to add), and discards      */
/* segments that are received out of order.  It ignores the received window    */
/* and always offers a fixed window size on input (i.e., it is not flow        */
/* controlled).								   								   */
/*                                                                             */
/* Special care is taken to access the ethernet buffers only in word		   */
/* mode.  This is to support boards that only allow word accesses.			   */
/*                                                                             */
/* Copyright (C) 1986, IMAGEN Corporation								       */
/*  "This code may be duplicated in whole or in part provided that [1] there   */
/*   is no commercial gain involved in the duplication, and [2] that this      */
/*   copyright notice is preserved on all copies.  Any other duplication       */
/*   requires written notice of the author."								   */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : tcp.c
 文件描述 : TCP协议栈

 说明:本协议栈从TI移植而来,并对其做了精简,为TBS web升级提供TCP协议支持
 函数列表 :


 修订记录 :
          1 创建 : 轩光磊
            日期 : 2008-7-22
            描述 :
=========================================================================*/

#include <common.h>
#include <flash_layout_private.h>
#include <types.h>
#include <net.h>
#include <tcp.h>
#include <crc.h>

#ifdef CMD_TCP

DECLARE_GLOBAL_DATA_PTR;

static	unsigned char indexdata[]="HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\
\r\n\
<html>\
<head><title>Update Firmware</title></head>\
<body>\
<form enctype=multipart/form-data method=post>\
<b>Update Software</b><br>\
<br>\
<b>Step 1:</b> Obtain an updated software image file from your ISP.<br>\
<br>\
<b>Step 2:</b> Enter the path to the image file location in the box below or \
click the &quot;Browse&quot; button to locate the image file.<br>\
<br>\
<b>Step 3:</b> Click the 'Update Software'button once to upload the new image file.<br>\
<br>\
NOTE: The update process takes about 3 minutes to complete, and your DSL Router \
will reboot.<br>\
<br>\
<tr>\
<td>Software File Name:&nbsp;\
</td>\
<td><input type='file' name='filename' size='15'></td>\
</tr>\
<p align='center'><input type=submit value='Update Software'></p>\
</form>\
</body>\
</html>";



static	unsigned char updateerror_crc[]="HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\
\r\n\
<html>\
<head><title>Update Firmware</title></head>\
<body>\
Checksum validation failed! You should translate image again!\
<form enctype=multipart/form-data method=post>\
<tr>\
<td>Software File Name:&nbsp;\
</td>\
<td><input type='file' name='filename' size='15'></td>\
</tr>\
<p align='center'><input type=submit value='Update Software'></p>\
</form>\
</body> \
</html>";

static	unsigned char updateerror_big[]="HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\
\r\n\
<html>\
<head><title>Update Firmware</title></head>\
<body>\
Update failure! Because your update image is too long ! Please select a small one.\
<form enctype=multipart/form-data method=post>\
<tr>\
<td>Software File Name:&nbsp;\
</td>\
<td><input type='file' name='filename' size='15'></td>\
</tr>\
<p align='center'><input type=submit value='Update Software'></p>\
</form>\
</body> \
</html>";

static	unsigned char updateerror_img_type[]="HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\
\r\n\
<html>\
<head><title>Update Firmware</title></head>\
<body>\
Update failure !Because your system is %s ,but you translate a %s image!\
<form enctype=multipart/form-data method=post>\
<tr>\
<td>Software File Name:&nbsp;\
</td>\
<td><input type='file' name='filename' size='15'></td>\
</tr>\
<p align='center'><input type=submit value='Update Software'></p>\
</form>\
</body> \
</html>";

static	unsigned char updateerror_product[]="HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\
\r\n\
<html>\
<head><title>Update Firmware</title></head>\
<body>\
Update failure ! Because your system is %s ,but you translate a %s image!\
<form enctype=multipart/form-data method=post>\
<tr>\
<td>Software File Name:&nbsp;\
</td>\
<td><input type='file' name='filename' size='15'></td>\
</tr>\
<p align='center'><input type=submit value='Update Software'></p>\
</form>\
</body> \
</html>";

static	unsigned char updateok[]="HTTP/1.0 200 OK\r\n\
Content-type: text/html\r\n\
\r\n\
<html>\
<head><title>Update Firmware</title></head>\
<body>\
<b>DSL Router Software Upgrade</b><br>\
New software is being programmed to the flash memory. The DSL Router will reboot upon completion. This process will take about 3 minutes.\
</body> \
</html>";


tcp_Socket		*tcp_allsocs;
//tcp_Socket		ftp_ctl;
tcp_Socket		http_data;

unsigned long		length = 0;
char				first_http_package = 0;
char				boundary_data[255];
unsigned int		boundary_len = 0;
unsigned long		content_length;
unsigned long		content_length_receive;
char				toolong = 0;			/* 0:normal         1:too long */
char				find_boundary = 0;			/* 0:not find         1:find */
char				find_boundary_change = 0;	/* 0:not change	1:change */

char				image_ok ;			/* 0:valid   1:OK*/

/* IP identification numbers */
static word tcp_id;

/* Timer definitions */
#define tcp_RETRANSMITTIME 1000     /* interval at which retransmitter is called */
#define tcp_LONGTIMEOUT 31000       /* timeout for opens */
#define tcp_TIMEOUT 10000           /* timeout during a connection */

#define Move(s,d,l)  (sys_memcpy(d,s,l))


#if defined(FTP)
unsigned ftp_pkt_counter;
#endif

/*
 * Passive open: listen for a connection on a particular port
 */
void tcp_Listen(tcp_Socket *s, word port, procref datahandler, int timeout)
  {
#ifdef FTP_SERVER_SUPPORT
  if ((s->state != 0) && (s->state != tcp_StateCLOSED)) {
	s->flags = tcp_FlagACK;
    tcp_Send(s);
    s->state = tcp_StateCLOSED;
    s->dataHandler(s, 0, 0,SABORT);
    tcp_Unthread(s);
  }
#endif

  s->ip_type = 6;
  s->state = tcp_StateLISTEN;
  if ( timeout == 0 ) 
    s->timeout = 0x7ffffff; /* forever... */
  else s->timeout = timeout;
  s->myport = port;
  s->hisport = 0;
  s->seqnum = 0;
  s->dataSize = 0;
  s->flags = 0;
  s->unhappy = 0;
  s->dataHandler = datahandler;
  s->next = tcp_allsocs;
  tcp_allsocs = s;
  }

/*
 * Send a FIN on a particular port -- only works if it is open
 */
void tcp_Close(tcp_Socket *s)
  {
#if FTP_SERVER_DEBUG  
  sys_printf("Inside tcp_Close\n");
#endif
  if ( s->state == tcp_StateESTAB || s->state == tcp_StateSYNREC ) 
    {
    s->flags = tcp_FlagACK | tcp_FlagFIN;
    s->state = tcp_StateFINWT1;
    s->unhappy = TRUE;
	tcp_Send(s); /*TODO*/
    }
  }

/*
 * Abort a tcp connection
 */
void tcp_Abort(tcp_Socket *s)
  {
  if ( s->state != tcp_StateLISTEN && s->state != tcp_StateCLOSED ) 
    {
    s->flags = tcp_FlagRST | tcp_FlagACK;
    tcp_Send(s);
    }
  s->unhappy = 0;
  s->dataSize = 0;
  s->state = tcp_StateCLOSED;
  s->dataHandler(s, 0, -1,SABORT);
  tcp_Unthread(s);
  }



/*
 * Unthread a socket from the socket list, if it's there 
 */
void tcp_Unthread(tcp_Socket *ds)
{
  tcp_Socket *s, **sp;

  sp = &tcp_allsocs;
  for (;;) 
    {
    s = *sp;
    if ( s == ds ) 
      {
      *sp = s->next;
      break;
      }
    if ( s == NULL ) 
      break;
    sp = &s->next;
    }
}


/*
 * Write data to a connection.
 * Returns number of bytes written, == 0 when connection is not in
 * established state.
 */
int tcp_Write(tcp_Socket *s, byte *dp, int len)
  {
  int x;
#if FTP_SERVER_DEBUG  
  	sys_printf("*********** Inside tcp_Write ...\n");
	sys_printf("*********** len: %d\n", len);
	sys_printf("*********** s->dataSize: %d\n", s->dataSize);
#endif	
  if ( s->state != tcp_StateESTAB ) {
    len = 0;
  }
  if ( len > (x = (tcp_MaxData - s->dataSize)) ) 
    len = x;
#if FTP_SERVER_DEBUG  
	sys_printf("*********** len: %d, x: %d, tcp_MaxData: %d\n", len, x, tcp_MaxData);  
	sys_printf("*********** s->dataSize: %d, tcp_MaxData: %d\n", s->dataSize, tcp_MaxData);  
#endif	
  if ( len > 0 ) 
    {
    Move(dp, &s->data[s->dataSize], len);
    s->dataSize += len;
#if FTP_SERVER_DEBUG  
	sys_printf("*********** len: %d, s->dataSize: %d\n", len, s->dataSize);
#endif	
    /*tcp_Flush(s); */
    }
  return ( len );
  }

/*
 * Send pending data
 */
void tcp_Flush(tcp_Socket *s)
  {
#if FTP_SERVER_DEBUG  
  	sys_printf("Inside tcp_Flush ...\n");		  
#endif	
  if ( s->dataSize > 0 ) 
    {
    s->flags |= tcp_FlagPUSH;
    tcp_Send(s);
    }
  }

/*
 * Handler for incoming packets.
 */

void tcp_Handler(in_Header *ip)
{
	tcp_Header *tp;
	tcp_PseudoHeader ph;
	int len;
	int diff;
	tcp_Socket *s;
	word flags;

	len = in_GetHdrlenBytes(ip);

	tp = (tcp_Header *)((byte *)ip + len);
	len = wfix(ip->length) - len;

	/* demux to active sockets */
	for ( s = tcp_allsocs; s; s = s->next )
	{
		if ( s->hisport != 0 && 
			wfix(tp->dstPort) == s->myport && 
			wfix(tp->srcPort) == s->hisport && 
			lfix(ip->source) == s->hisaddr ) 
			break;

	}

	if ( s == NULL ) 
	{
		/* demux to passive sockets */
		for ( s = tcp_allsocs; s; s = s->next )
		{
			if ( s->hisport == 0 && wfix(tp->dstPort) == s->myport ) 
			break;
		}
	}
	
	if ( s == NULL ) 
	{	
		return;
	}

	/* save his ethernet address */
	MoveW(&((((eth_Header *)ip) - 1)->source[0]), &s->hisethaddr[0], sizeof(eth_HwAddress));

	ph.src = ip->source;
	ph.dst = ip->destination;
	ph.mbz = 0;
	ph.protocol = 6;
	ph.length = wfix(len);
	ph.checksum = wfix(checksum(tp, len));

	if ( checksum(&ph, sizeof ph) != 0xffff )
	{	
		return;
	}

	flags = wfix(tp->flags);
	if ( flags & tcp_FlagRST ) 
	{
		s->state = tcp_StateCLOSED;
		s->dataHandler(s, 0, -1,SABORT);
		tcp_Unthread(s);	

		return;
	}

    switch ( s->state ) 
      {
    case tcp_StateLISTEN:
        if ( flags & tcp_FlagSYN ) 
          {
#if 1
          
         s->acknum = lfix4(tp->seqnum) + 1;

#else
          s->acknum = lfix(tp->seqnum) + 1;

#endif
          s->hisport = wfix(tp->srcPort);
          s->hisaddr = lfix(ip->source);
          s->flags = tcp_FlagSYN | tcp_FlagACK;

          tcp_Send(s);
          s->state = tcp_StateSYNREC;
          s->unhappy = TRUE;
          s->timeout = tcp_TIMEOUT;
          }
        break;

    case tcp_StateSYNSENT:
        if ( flags & tcp_FlagACK )
          {
#if 1
          if (lfix4(tp->acknum) != (s->seqnum + 1))

#else
          if (lfix(tp->acknum) != (s->seqnum + 1))

#endif
            {
            s->flags = tcp_FlagRST;
            tcp_Send(s);
            s->flags = tcp_FlagSYN;
            }
          }

        if ( flags & tcp_FlagSYN ) 
            {
            s->acknum++;
            s->flags = tcp_FlagACK;
            s->timeout = tcp_TIMEOUT;
#if 1
            if ( (flags & tcp_FlagACK) && lfix4(tp->acknum) == (s->seqnum + 1) ) 

#else
            if ( (flags & tcp_FlagACK) && lfix(tp->acknum) == (s->seqnum + 1) ) 

#endif
                {
                s->state = tcp_StateESTAB;
                s->seqnum++;
#if 1
                s->acknum = lfix4(tp->seqnum) + 1;

#else
                s->acknum = lfix(tp->seqnum) + 1;

#endif
                s->unhappy = FALSE;
                tcp_Send(s);    /*DRB Added*/
                s->dataHandler(s, 0, 0,SOPEN);
                } 
               else 
                {
                s->state = tcp_StateSYNREC;
                }
            }
        break;

    case tcp_StateSYNREC:
        if ( flags & tcp_FlagSYN ) 
          {
          s->flags = tcp_FlagSYN | tcp_FlagACK;
          tcp_Send(s);
          s->timeout = tcp_TIMEOUT;
          }
#if 1
        if ( (flags & tcp_FlagACK) && lfix4(tp->acknum) == (s->seqnum + 1) ) 

#else
        if ( (flags & tcp_FlagACK) && lfix(tp->acknum) == (s->seqnum + 1) ) 

#endif
          {
          s->flags = tcp_FlagACK;
          s->seqnum++;
       
          s->unhappy = FALSE;
          s->state = tcp_StateESTAB;
          s->timeout = tcp_TIMEOUT;
          s->dataHandler(s, 0, 0,SOPEN);
          }
        break;

    case tcp_StateESTAB:
        if ( (flags & tcp_FlagACK) == 0 ) return;
        /* process ack value in packet */
#if 1
        diff = lfix4(tp->acknum) - s->seqnum;

#else
        diff = lfix(tp->acknum) - s->seqnum;

#endif
        if ( diff > 0 ) {
            Move(&s->data[diff], &s->data[0], diff);
            s->dataSize -= diff;
            s->seqnum += diff;
        }
        s->flags = tcp_FlagACK;
        tcp_ProcessData(s, tp, len);
        break;

    case tcp_StateFINWT1:
        if ( (flags & tcp_FlagACK) == 0 ) return;

#if 1
        diff = lfix4(tp->acknum) - s->seqnum - 1;

#else
        diff = lfix(tp->acknum) - s->seqnum - 1;

#endif
        s->flags = tcp_FlagACK | tcp_FlagFIN;
        if ( diff == 0 ) {
            s->state = tcp_StateFINWT2;
            s->flags = tcp_FlagACK;
	     s->seqnum++;	
        }
#ifdef FTP_SERVER_SUPPORT
		else {
			Move(&s->data[diff], &s->data[0], diff);
            s->dataSize -= diff;
            s->seqnum += diff;
		}
#endif
        tcp_ProcessData(s, tp, len);
        break;

    case tcp_StateFINWT2:
        s->flags = tcp_FlagACK;
        tcp_ProcessData(s, tp, len);

		
            s->state = tcp_StateCLOSED;
      	     s->unhappy = FALSE;
            s->dataSize = 0;
            s->dataHandler(s, 0, 0,SCLOSE);
            tcp_Unthread(s);
			tcp_Listen(&http_data, 80, GotData, 0);
		
        break;

    case tcp_StateCLOSING:
#if 1
        if ( lfix4(tp->acknum) == (s->seqnum + 1) ) {

#else
        if ( lfix(tp->acknum) == (s->seqnum + 1) ) {

#endif
            s->state = tcp_StateTIMEWT;
            s->timeout = tcp_TIMEOUT;
        }
        break;

    case tcp_StateLASTACK:
#if 1
        if ( lfix4(tp->acknum) == (s->seqnum + 1) ) {

#else
        if ( lfix(tp->acknum) == (s->seqnum + 1) ) {

#endif
            s->state = tcp_StateCLOSED;
            s->unhappy = FALSE;
            s->dataSize = 0;
            s->dataHandler(s, 0, 0,SCLOSE);
            tcp_Unthread(s);
        } else {
            s->flags = tcp_FlagACK | tcp_FlagFIN;
            tcp_Send(s);
            s->timeout = tcp_TIMEOUT;
        }
        break;

    case tcp_StateTIMEWT:
        s->flags = tcp_FlagACK;
        tcp_Send(s);
#ifdef FTP_SERVER_SUPPORT
        s->state = tcp_StateCLOSED;
        s->dataHandler(s, 0, 0,SCLOSE);
        tcp_Unthread(s);
#endif
    }
}

/*
 * Process the data in an incoming packet.
 * Called from all states where incoming data can be received: established,
 * fin-wait-1, fin-wait-2
 */
void tcp_ProcessData(tcp_Socket *s, tcp_Header *tp, int len)
{
	int			diff;
	int			x;
	word		flags;
	byte			*dp;

	/* Check if this packet was not received before */
#if 1
	if( lfix4(tp->seqnum) >= s->acknum )
#else
	if( lfix(tp->seqnum) >= s->acknum )
#endif
	{
		flags = wfix(tp->flags);
	
#if 1
		diff = s->acknum - lfix4(tp->seqnum);

#else
		diff = s->acknum - lfix(tp->seqnum);

#endif
		if ( flags & tcp_FlagSYN ) 
	 	{
	  		diff--;
	  	}
		x = tcp_GetDataOffset(tp) << 2;
		dp = (byte *)tp + x;
		len -= x;
		if ( diff >= 0 ) 
		{
			dp += diff;
			len -= diff;
			s->acknum += len;
			s->dataHandler(s, dp, len,SDATA);
			if ( flags & tcp_FlagFIN ) 
			{
				s->acknum++;
				switch(s->state) 
				{
					case tcp_StateESTAB:
						/* note: skip state CLOSEWT by automatically closing conn */
						x = tcp_StateLASTACK;
						s->flags |= tcp_FlagFIN;
						s->unhappy = TRUE;
						break;
					case tcp_StateFINWT1:
						x = tcp_StateCLOSING;
						break;
					case tcp_StateFINWT2:
						x = tcp_StateTIMEWT;
						break;
				}
				s->state = x;
          		}
  			tcp_Send(s);
		}
	}
	else
	{ /* lost ACK for the recieved packet. re-send */
        	tcp_Send(s);
	}
	s->timeout = tcp_TIMEOUT;
}

/*
 * Format and send an outgoing segment
 */
void tcp_Send(tcp_Socket *s)
{
    tcp_PseudoHeader ph;
    struct _pkt {
        in_Header in;
        tcp_Header tcp;
        longword maxsegopt;
    } *pkt;
    byte *dp;
#if FTP_SERVER_DEBUG  
	sys_printf("Inside tcp_Send, dataSize: %d\n", s->dataSize);
#endif	
    pkt = (struct _pkt *)sed_FormatPacket(&s->hisethaddr[0], 0x800);
    dp = (byte *)&pkt->maxsegopt;

    pkt->in.length = wfix(sizeof(in_Header) + sizeof(tcp_Header) + s->dataSize);

    /* tcp header */
    pkt->tcp.srcPort = wfix(s->myport);
    pkt->tcp.dstPort = wfix(s->hisport);
    pkt->tcp.seqnum = lfix(s->seqnum);
    pkt->tcp.acknum = lfix(s->acknum);
    pkt->tcp.window = wfix(1024);
    pkt->tcp.flags = wfix(s->flags | 0x5000);
    pkt->tcp.checksum = 0;
    pkt->tcp.urgentPointer = 0;
    if ( s->flags & tcp_FlagSYN ) 
      {
      pkt->tcp.flags = wfix(0x1000+wfix(pkt->tcp.flags));
      pkt->in.length = wfix(4+wfix(pkt->in.length));
      pkt->maxsegopt = lfix(0x02040578); /* 1400 bytes */
      dp += 4;
      }
    MoveW(s->data, dp, s->dataSize);

    /* internet header */
    pkt->in.vht = wfix(0x4500);   /* version 4, hdrlen 5, tos 0 */
    pkt->in.identification = wfix(tcp_id++);
    pkt->in.frag = 0;
    pkt->in.ttlProtocol = wfix((250<<8) + 6);
    pkt->in.checksum = 0;
    pkt->in.source = lfix(NetOurIP);
    pkt->in.destination = lfix(s->hisaddr);
    pkt->in.checksum = wfix(~checksum(&pkt->in, sizeof(in_Header)));

    /* compute tcp checksum */
    ph.src = pkt->in.source;
    ph.dst = pkt->in.destination;
    ph.mbz = 0;
    ph.protocol = 6;
    ph.length = wfix(wfix(pkt->in.length) - sizeof(in_Header));
    ph.checksum = wfix(checksum(&pkt->tcp, wfix(ph.length)));
    pkt->tcp.checksum = wfix(~checksum(&ph, sizeof ph));

    sed_Send(wfix(pkt->in.length));
}

/*
 * Do a one's complement checksum
 */
int checksum(void *vdp, int length)
{
    int len;
    longword sum;
    word *dp=vdp;

    len = length >> 1;
    sum = 0;
    while ( len-- > 0 ) 
      {
    sum += wfix(*dp);
      dp++;
      }
    if ( length & 1 ) sum += (wfix(*dp) & 0xFF00);
    sum = (sum & 0xFFFF) + ((sum >> 16) & 0xFFFF);
    sum = (sum & 0xFFFF) + ((sum >> 16) & 0xFFFF);
    return ( sum );
}


int atoi(const char *s)
{
    int idx = 0;
    int sign = 1, val = 0;

    if (s == 0)
        return 0;
    while (s[idx] != 0)
    {
        if ((s[idx] == ' ') || (s[idx] == '\t'))
            idx++;
        else
            break;
    }
    if (s[idx] == 0)
        return 0;
    else if (s[idx] == '-')
    {
        sign = -1;
        idx++;
    }
    while ((s[idx] >= '0') && (s[idx] <= '9'))
    {
        val = val * 10 + (s[idx] - '0');
        idx++;
    }
    return (sign*val);
}

int check_image( tcp_Socket *s,unsigned int len)
{
	update_hdr_t		*update_hdr;
	sys_config_t		*sys_data;
	char				web_string[512];
	unsigned long		checksum ;
	unsigned int		img_file_checksum;

	image_ok = 0;
	sys_data = ( sys_config_t *  ) ( CFG_SDRAM_BASE + SYSCFG_OFFSET );
	update_hdr = ( update_hdr_t * ) load_addr;

	img_file_checksum = *( unsigned int * )( load_addr + len - 4 );

	tbs_calc_sum_addr( ( unsigned char * ) load_addr , &checksum , len - 4 );

	if( img_file_checksum!= checksum )
	{
		tcp_Write(s, updateerror_crc, strlen(updateerror_crc));
		return ERROR_CRC;
	}

	if( strcmp( update_hdr->img_type, IMAGE_TYPES ) != 0 )
	{
		sprintf(web_string, updateerror_img_type, IMAGE_TYPES , update_hdr->img_type);
		tcp_Write(s, web_string, strlen(web_string));

		return ERROR_IMG_TYPE;
	}

	if( strcmp( PRODUCT, update_hdr->product ) != 0 )
	{
		sprintf(web_string, updateerror_product, PRODUCT , update_hdr->product );
		tcp_Write(s, web_string, strlen(web_string));
		return ERROR_PRODUCT;
	}

#ifdef CONFIG_DOUBLE_BACKUP
	if( update_hdr->image_len > ( info->size  - sys_data->layout.zone_offset[ ZONE_KERNEL_SECOND] ) ) 
	{		
		tcp_Write(s, updateerror_big, strlen(updateerror_big));
		
		return ERROR_BIG;
	}
#else

	if( update_hdr->image_len > ( info->size  - sys_data->layout.zone_offset[ZONE_KERNEL_FIRST] ) ) 
	{		
		tcp_Write(s, updateerror_big, strlen(updateerror_big));
		
		return ERROR_BIG;
	}
#endif

	tcp_Write(s, updateok, strlen(updateok));
	image_ok = 1;

	return ERROR_OK;
}
void GotData(tcp_Socket *s, byte *dp, int len, int state)
{
	int			i = 0;
	int			j = 0;
	char			*http_ram = dp;
	unsigned int	http_ram_tail = len;
	sys_config_t	*syscfg;

	if( state == 2 )
	{		
		if( ( http_ram[0] == 'G' ) && ( http_ram[1] == 'E' ) &&  ( http_ram[2] == 'T' ) )
		{
			tcp_Write(s, indexdata, strlen(indexdata));
		}
		else if ((http_ram[0] == 'P'&&http_ram[1] == 'O'&&http_ram[2] == 'S'&&http_ram[3] == 'T')) 
		{	
			first_http_package = 1;
			length = 0;
			boundary_len = 0;
			find_boundary = 0;
						
			for(i=0;i<http_ram_tail;i++) 
			{
				if (http_ram[i]=='b'&&http_ram[i+1]=='o'&&http_ram[i+2]=='u'&&http_ram[i+3]=='n'&&
					http_ram[i+4]=='d'&&http_ram[i+5]=='a'&&http_ram[i+6]=='r'&&http_ram[i+7]=='y'
					&&http_ram[i+8]=='=') 
				{
					i += 9;
					j = 0;
					while(i<http_ram_tail) 
					{
						if (http_ram[i]=='\r'&&http_ram[i+1]=='\n')
						{
							break;
						}
						boundary_data[j++] = http_ram[i];
						i++;
						boundary_len++;
					}
				}
			}

			// find out the content-length
			for(i=0;i<http_ram_tail;i++) 
			{
				while(1) 
				{
					if (http_ram[i]=='\r'&&http_ram[i+1]=='\n') 
					{
						i+=2;
						break;
					}
					i++;
				}
				
				if (http_ram[i]=='C'&&http_ram[i+1]=='o'&&http_ram[i+2]=='n'&&http_ram[i+3]=='t'&&
					http_ram[i+4]=='e'&&http_ram[i+5]=='n'&&http_ram[i+6]=='t'&&http_ram[i+7]=='-'&&
					http_ram[i+8]=='L'&&http_ram[i+9]=='e'&&http_ram[i+10]=='n'&&http_ram[i+11]=='g') 
				{
					i += 15;
					content_length = atoi(&http_ram[i]);
					break;
				}
			}

			if( content_length > info->size )
			{
				toolong = 1;
			}

			while(i<http_ram_tail) 
			{
				if (http_ram[i]=='\r'&&http_ram[i+1]=='\n'&&http_ram[i+2]=='\r'&&http_ram[i+3]=='\n') 
				{
					i+=4;
					break;
				}
				i++;
			}
			for( j = i ; j < http_ram_tail ; j++ )
			{
				if( strncmp(&http_ram[j], boundary_data, boundary_len) == 0 )
				{
					find_boundary = 1;
					break;
				}
				else
				{
					find_boundary = 0;					
				}
			}
			
			content_length_receive = http_ram_tail - i;
			while( i < http_ram_tail ) 
			{
				if (http_ram[i]=='\r'&&http_ram[i+1]=='\n'&&http_ram[i+2]=='\r'&&http_ram[i+3]=='\n') 
				{
					i+=4;
					break;
				}
				i++;
			}

			if( content_length_receive ==  content_length )
			{
				length = http_ram_tail - i;
				memcpy(load_addr , http_ram + i ,  length);

				length -= boundary_len + 8;			/* (\r\n)*2 + 4*(-) =8*/

				first_http_package = 0;
				check_image( s,length);					
				return;

			}			
			length = http_ram_tail - i;
			memcpy(load_addr, http_ram + i , http_ram_tail - i );			
		}
		else if( first_http_package == 1 )
		{
			if( find_boundary == 0 )
			{
				for( j = 0 ; j < http_ram_tail ; j++ )
				{
					if( strncmp(&http_ram[j], boundary_data, boundary_len) == 0 )
					{
						find_boundary = 1;
						find_boundary_change = 1;
						content_length_receive = http_ram_tail - j + 2;
						i = 0;
						while( i < http_ram_tail ) 
						{
							if (http_ram[i]=='\r'&&http_ram[i+1]=='\n'&&http_ram[i+2]=='\r'&&http_ram[i+3]=='\n') 
							{
								i+=4;
								break;
							}
							i++;
						}
						break;
					}
					else
					{
						find_boundary = 0;					
					}
				}
			}
			else
			{
				content_length_receive += http_ram_tail;
			}

			if( ( toolong == 0 ) && ( content_length_receive <=  content_length ) )
			{
				if( find_boundary_change == 1 )
				{
					find_boundary_change = 0;
					memcpy( load_addr + length , http_ram + i ,  http_ram_tail - i );
					length += http_ram_tail - i;
				}
				else
				{
					memcpy(load_addr + length, http_ram,  http_ram_tail);
					length += http_ram_tail;
				}
				
				if( content_length_receive ==  content_length )
				{
						length -= boundary_len + 8;			/* (\r\n)*2 + 4*(-) =8*/
						first_http_package = 0;
						check_image( s,length);					
						return;
				}
			}
			else if( ( toolong == 1 )  && ( content_length_receive >= content_length ) )
			{
				toolong = 0;
				first_http_package = 0;
				content_length_receive = 0;
				tcp_Write(s, updateerror_big, strlen(updateerror_big));
				return ;
			}
		}		
		else  if( s->state == tcp_StateESTAB || s->state == tcp_StateSYNREC ) 
		{	
			s->flags = tcp_FlagACK | tcp_FlagFIN;
			s->state = tcp_StateFINWT1;
			s->unhappy = TRUE;
		}
	}

	if( ( state == 3 ) && ( image_ok == 1 ) )
	{
		syscfg =(sys_config_t*)(CFG_SDRAM_BASE + SYSCFG_OFFSET);
		flash_update_img( ( unsigned char * ) load_addr,  syscfg , length);
	}
}

/*
 * Initialize the tcp implementation
 */

extern int MoreUpgrade ;
int tcp_Init()
 {
	bd_t *bd = gd->bd;

	tcp_allsocs = NULL;
	tcp_id = 0;
	image_ok = 0;
	
	NetCopyIP(&NetOurIP, &bd->bi_ip_addr);
	memcpy (NetOurEther, bd->bi_enetaddr, 6);
	memcpy(sed_lclEthAddr, NetOurEther, 6);

	eth_halt();
	if (eth_init(bd) < 0) 
	{
		eth_halt();
		return(-1);
	}	
	return 0;
 }


void tbs_tcp()
{
	tcp_Init();
	tcp_Listen(&http_data, 80, GotData, 0);
}

#if 0

/*
 * Actively open a TCP connection to a particular destination.
 */
void tcp_Open(tcp_Socket *s, word lport, in_HwAddress ina, word port, procref datahandler)
  {
  extern eth_HwAddress sed_ethBcastAddr;

#ifdef FTP_SERVER_SUPPORT  
  if ((s->state != 0) && (s->state != tcp_StateCLOSED)) {
	s->flags = tcp_FlagACK;
    tcp_Send(s);
    s->state = tcp_StateCLOSED;
    s->dataHandler(s, 0, 0,SABORT);
    tcp_Unthread(s);
  }
#endif
  
  s->ip_type = 6;

  s->state = tcp_StateSYNSENT;
  s->timeout = tcp_LONGTIMEOUT;
  if ( lport == 0 ) lport = clock_ValueRough();
  s->myport = lport;
  if ( ! sar_MapIn2Eth(ina, (eth_HwAddress *)&s->hisethaddr[0]) ) 
    {
    sys_printf("ARP fail. Resorting to broadcast.\n", ina);
    Move(&sed_ethBcastAddr[0], &s->hisethaddr[0], sizeof(eth_HwAddress));
    }
  s->hisaddr = ina;
  s->hisport = port;
  s->seqnum = 0;
  s->dataSize = 0;
  s->flags = tcp_FlagSYN;
  s->unhappy = TRUE;
  s->dataHandler = datahandler;
  s->next = tcp_allsocs;
  tcp_allsocs = s;
  tcp_Send(s);
  }


/*
 * Retransmitter - called periodically to perform tcp retransmissions
 */
void tcp_Retransmitter()
  {
  tcp_Socket *s;
  BOOL x;

  for ( s = tcp_allsocs; s; s = s->next ) 
    {
    x = FALSE;
    if ( s->dataSize > 0 || s->unhappy ) 
      {
      tcp_Send(s);
      x = TRUE;
      }
    if ( x || s->state != tcp_StateESTAB )
      s->timeout -= tcp_RETRANSMITTIME;
    if ( s->timeout <= 0 ) 
      {
      if ( s->state == tcp_StateTIMEWT ) 
        {
        s->state = tcp_StateCLOSED;
        s->dataHandler(s, 0, 0,SABORT);
        tcp_Unthread(s);
        } 
       else 
        {
        tcp_Abort(s);
        }
      }
    }
  }



/*
 * busy-wait loop for tcp.  Also calls an "application proc"
 */
int tcp(procref application)
{
    in_Header *ip;
    longword timeout, start;
    int x;


    timeout = clock_ValueRough() + tcp_RETRANSMITTIME;
    while ( tcp_allsocs ) 
        {
        start = clock_ValueRough();
        ip = (in_Header *)sed_IsPacket();
        if ( ip == NULL ) 
            {
            if ( clock_ValueRough() > timeout ) 
                {
                tcp_Retransmitter();
                timeout = clock_ValueRough() + tcp_RETRANSMITTIME;
                }
#if defined(FTP) && defined (AVALANCHE)
            ftp_pkt_counter++;
            if((ftp_pkt_counter % 50000) == 0) {
                application();
            }
#else
            application();
#endif

            continue;
            }

        if ( sed_CheckPacket(ip, 0x806) == 1 ) {
            /* do arp */
            sar_CheckPacket((arp_Header *)ip);

        } else if ( sed_CheckPacket(ip, 0x800) == 1 ) 
            {
          /* do IP */
            if ( ip->destination == lfix(NetOurIP) &&
                 in_GetProtocol(ip) == 6 &&
                 checksum(ip, in_GetHdrlenBytes(ip)) == 0xFFFF ) 
              {
              tcp_Handler(ip);
              }
            }

        x = clock_ValueRough() - start;
        timeout -= x;
        }

    return ( 1 );
}


/*
 * Move bytes from hither to yon
 */


void Move( void *vsrc, void *vdest, int numbytes )
  {
  byte *src=vsrc;
  byte *dest=vdest;

    if ( numbytes <= 0 ) return;
    if ( src < dest ) {
        src += numbytes;
        dest += numbytes;
        do {
            *--dest = *--src;
        } while ( --numbytes > 0 );
    } else
        do {
             *dest++ = *src++;
        } while ( --numbytes > 0 );
  }

#endif


#endif

