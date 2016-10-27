/*---------------------------------------------------------------------
 *
 * Program: MULTITST.EXE  Optional WinSock features test program
 *
 * filename: icmpping.c
 *
 * copyright by Bob Quinn, 1995
 *
 * Description: This module does ICMP ping on WinSock implementations
 *  that support the *optional* IPPROTO_ICMP SOCK_RAW socket type.
 *  It also has the potential for traceroute capability over (much
 *  less common) setsockopt() IP_TTL capable WinSock implementations.
 * .
 *  This software is not subject to any  export  provision  of
 *  the  United  States  Department  of  Commerce,  and may be
 *  exported to any country or planet.
 *
 *  Permission is granted to anyone to use this  software  for any  
 *  purpose  on  any computer system, and to alter it and redistribute 
 *  it freely, subject to the following  restrictions:
 *
 *  1. The author is not responsible for the consequences of
 *     use of this software, no matter how awful, even if they
 *     arise from flaws in it.
 *
 *  2. The origin of this software must not be misrepresented,
 *     either by explicit claim or by omission.  Since few users
 *     ever read sources, credits must appear in the documentation.
 *
 *  3. Altered versions must be plainly marked as such, and
 *     must not be misrepresented as being the original software.
 *     Since few users ever read sources, credits must appear in
 *     the documentation.
 *
 *  4. This notice may not be removed or altered.
 *	 
 ---------------------------------------------------------------------*/
#include <windows.h>
#include <windowsx.h>
#include <winsock.h>
#include <stdlib.h>
#include <memory.h>

#include "..\wsa_xtra.h"

#define PNGBUFSIZE 8192+ICMP_HDR_LEN+IP_HDR_LEN

/* external functions */
extern void  WSAErrMsg(LPSTR);

/* internal public functions */
SOCKET icmp_open(void);
u_short cksum (u_short FAR*, int);
int icmp_close(SOCKET);
int set_ttl (SOCKET, int);
int icmp_sendto (SOCKET, HWND, LPSOCKADDR_IN, int, int, int);
u_long icmp_recvfrom(SOCKET,LPINT,LPINT,LPSOCKADDR_IN);

/* private data */
static ICMP_HDR FAR *lpIcmpHdr;	/* pointers into our I/O buffer */
static IP_HDR 	FAR *lpIpHdr;
static char achIOBuf[PNGBUFSIZE];
static SOCKADDR_IN stFromAddr;
static DWORD lCurrentTime, lRoundTripTime;

/*-----------------------------------------------------------
 * Function icmp_open()
 *
 * Description: opens an ICMP "raw" socket,
 *  - does error check
 *  - displays error message if failed,
 *  - else returns socket handle
 */
SOCKET icmp_open(void) {
  SOCKET s;
  s = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (s == SOCKET_ERROR) {
    WSAErrMsg("socket(type=SOCK_RAW, protocol=IPROTO_ICMP)");		
    return (INVALID_SOCKET);
  }
  return (s);
} /* end icmp_open() */

/*-----------------------------------------------------------
 * Function icmp_close()
 *
 * Description: closes a socket (generic for any socket, though
 *  we happen to use it for ICMP "raw" sockets here.
 *  - does error check
 *  - displays error message if failed (which is very unlikely)
 */
int icmp_close(SOCKET s) { 
  int nRet; 
  nRet = closesocket (s);
  if (nRet == SOCKET_ERROR)	/* very unlikely */
    WSAErrMsg("closesocket()");
  return (nRet);
} /* end icmp_close() */

/*-----------------------------------------------------------
 * Function set_IP_TTL()
 *
 * Description: Attempts to set the IP Time to live value using the
 *  IP_TTL socket option (which is rarely supported).  This is necessary
 *  to implement a traceroute application/
 */
int set_ttl (SOCKET s, int nTimeToLive) {
  int nRet;
  nRet = setsockopt (s, IPPROTO_IP, IP_TTL,(LPSTR)&nTimeToLive, sizeof(int));
  if (nRet==SOCKET_ERROR) {
    WSAErrMsg("setsockopt(lewel=IPPROTO_IP, option=IP_TTL)");
  }
  return (nRet);
} /* end set_IP_TTL() */

/*-----------------------------------------------------------
 * Function: icmp_ping()
 *
 * Description: sends an ICMP Echo Request to destination address
 *  provided, then reads the ICMP Echo Reply back.  It records the
 *  round trip time.  This requires a WinSock that can provide a
 *  socket of type=SOCK_RAW and protocol=IPPROTO_ICMP.  Since "raw
 *  socket" support is optional in WinSock version 1.1, this will
 *  not work over all WinSock implementations.
 *
 */
int icmp_sendto (SOCKET s,
    HWND hwnd,
    LPSOCKADDR_IN lpstToAddr,
    int nIcmpId,
    int nIcmpSeq,
    int nEchoDataLen) {
  int nAddrLen = sizeof(SOCKADDR_IN);
  int nRet;
  u_short i;
  char c;
 
  /*--------------------- init ICMP header -----------------------*/
  lpIcmpHdr = (ICMP_HDR FAR *)achIOBuf;
  lpIcmpHdr->icmp_type  = ICMP_ECHOREQ;
  lpIcmpHdr->icmp_code  = 0;
  lpIcmpHdr->icmp_cksum = 0;
  lpIcmpHdr->icmp_id	  = nIcmpId++;
  lpIcmpHdr->icmp_seq   = nIcmpSeq++;
  /*--------------------put data into packet------------------------
   * insert the current time, so we can calculate round-trip time
   *  upon receipt of echo reply (which will echo data we sent) */
  lCurrentTime = GetCurrentTime();			
  _fmemcpy (&(achIOBuf[ICMP_HDR_LEN]),&lCurrentTime,sizeof(long));
	 		  
  /* data length includes the time (but not icmp header) */
  c=' ';   /* first char: space, right after the time */
  for (i=ICMP_HDR_LEN+sizeof(long);
       ((i < (nEchoDataLen+ICMP_HDR_LEN)) && (i < PNGBUFSIZE)); 
       i++) {
    achIOBuf[i] = c;
    c++;
    if (c > '~')	/* go up to ASCII 126, then back to 32 */
      c= ' ';
  }
  /*----------------------assign ICMP checksum ----------------------
   * ICMP checksum includes ICMP header and data, and assumes current
   *  checksum value of zero in header */
  lpIcmpHdr->icmp_cksum = cksum((u_short FAR *)lpIcmpHdr, 
	  nEchoDataLen+ICMP_HDR_LEN);
  								  
  /*--------------------- send ICMP echo request -------------------*/
  nRet = sendto (s,         /* socket */
    (LPSTR)lpIcmpHdr,       /* buffer */
    nEchoDataLen+ICMP_HDR_LEN+sizeof(long),	/* length */
    0,                      /* flags */
    (LPSOCKADDR)lpstToAddr, /* destination */
    sizeof(SOCKADDR_IN));   /* address length */

  if (nRet == SOCKET_ERROR) {
    WSAErrMsg("sendto()");
  }
	
  return (nRet);
} /* end icmp_sendto() */

/*-----------------------------------------------------------
 * Function: icmp_recvfrom()
 *
 * Description:
 *   receive icmp echo reply, parse the reply packet to get round trip
 *   time, and calculate it.
 */
u_long icmp_recvfrom(SOCKET s,
    LPINT lpnIcmpId,
    LPINT lpnIcmpSeq, 
    LPSOCKADDR_IN lpstFromAddr) {
  u_long lSendTime;	
  int nAddrLen = sizeof(struct sockaddr_in);
  int nRet, i;
		
  /*-------------------- receive ICMP echo reply ------------------*/
  stFromAddr.sin_family = AF_INET;
  stFromAddr.sin_addr.s_addr = INADDR_ANY;  /* not used on input anyway */
  stFromAddr.sin_port = 0;   /* port not used in ICMP */
  nRet = recvfrom (s,             /* socket */
      (LPSTR)achIOBuf,            /* buffer */
      PNGBUFSIZE+ICMP_HDR_LEN+sizeof(long)+IP_HDR_LEN,  /* length */
      0,                          /* flags */
      (LPSOCKADDR)lpstFromAddr,   /* source */
      &nAddrLen);                 /* addrlen*/

  if (nRet == SOCKET_ERROR) {
    WSAErrMsg("recvfrom()");
  }
  /*------------------------- parse data ---------------------------
   * remove the time from data and display with current time.
   *  NOTE: the data received and sent are asymmetric: we receive 
   *  the IP header, although we didn’t send it. This subtlety is
   *  often missed by WinSocks so we do a quick check of the data
   *  received to see if it includes the IP header (we look for 0x45
   *  value in first byte of buffer to check if IP header present).
   */
  /* figure out the offset to data */
  if (achIOBuf[0] == 0x45) {  /* IP header present? */
    i = IP_HDR_LEN + ICMP_HDR_LEN;
    lpIcmpHdr = (LPICMPHDR) &(achIOBuf[IP_HDR_LEN]);
  } else {
    i = ICMP_HDR_LEN;	
    lpIcmpHdr = (LPICMPHDR) achIOBuf;
  }
  
  /* pull out the ICMP ID and Sequence numbers */
  *lpnIcmpId  = lpIcmpHdr->icmp_id;
  *lpnIcmpSeq = lpIcmpHdr->icmp_seq;
   		
  /* remove the send time from the ICMP data */
  _fmemcpy (&lSendTime, (&achIOBuf[i]), sizeof(u_long));
   		
  return (lSendTime);
} /* end icmp_recvfrom() */

/*-----------------------------------------------------------
 * Function: cksum()
 *
 * Description:
 *  Calculate Internet checksum for data buffer and length (one’s 
 *  complement sum of 16-bit words).  Used in IP, ICMP, UDP, IGMP.
 */
u_short cksum (u_short FAR*lpBuf, int nLen) {	
  register long lSum = 0L;	/* work variables */
		
  /* note: to handle odd number of bytes, last (even) byte in 
   *  buffer have a value of 0 (we assume that it does) */
  while (nLen > 0) {
    lSum += *(lpBuf++);	/* add word value to sum */
    nLen -= 2;          /* decrement byte count by 2 */
  }
  /* put 32-bit sum into 16-bits */
  lSum = (lSum & 0xffff) + (lSum>>16);
  lSum += (lSum >> 16);

  /* return Internet checksum.  Note:integral type
   * conversion warning is expected here. It's ok. */
  return (~lSum); 
}  /* end cksum() */

