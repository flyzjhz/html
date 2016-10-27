/*
 * Filename: multitst.h
 *
 * Description:
 */
//#define FILE_MENU_EXIT		 101
//#define HELP_MENU_ABOUT		 102
//#define HELP_MENU_ABOUT_DLL 103
//#define START_SERVER		    104

#define bcopy(a,b,c)	_fmemcpy(b,a,c)

#ifndef WSA_MULTICAST
/* 
 * The following constants are from BSD /include/netinet/in.h
 *  header file.  They DO NOT match the values defined by Steve Deering 
 *  of Stanford University <deering@pescadero.Stanford.EDU> (WinNT v3.5
 *  did use Deering's values, however, which are (2-6)  
 *
 *  The use of these multicast options is described in "IP Multicast 
 *  Extensions for 4.3BSD UNIX related systems (MULTICAST 1.2 Release)".  
 *  It describes the extensions to BSD, SunOS and Ultrix to support 
 *  multicasting, as specified by RFC-1054 (since superceded by RFC-1112)
 */
#define DEERING_OFFSET	7
/* Also Note: 
 *  You must have "multicast=yes" in [pctcp kernel] section of PCTCP.INI
 *  file to use multicast!  Beware that some drivers do not support multi-
 *  cast, and the TCP/IP kernel will put the driver in promiscuous mode,
 *  which can decrease performance dramatically.  
 
 *  Other relevant configuration settings are:
 *    - "ip_ttl=XX" in [pctcp kernel] section, where XX is default
 *       Time to Live value for the IP header (default is 64)
 *    - "pktdrv-loopback=no" (default is yes) to disable loopback.  This
 *       might be necessary since some 1.14 WinSock versions and before
 *       used the wrong value for IP_MULTICAST_LOOP (1 rather than 4)
 *
 *  Known limitations on FTP Software's WinSock Multicast Support:
 *    - you cannot have multiple sockets with same (or different) 
 *       multicast addresses and the same port (only one will get data).
 */
#define IP_MULTICAST_IF		9		/* set/get IP mulicast interface */
#define	IP_MULTICAST_TTL	10		/* set/get IP multicast timetolive */
#define IP_MULTICAST_LOOP	11		/* set/get IP multicast loopback */
#define	IP_ADD_MEMBERSHIP	12		/* add  (set) IP group membership */
#define	IP_DROP_MEMBERSHIP	13		/* drop (set) IP group membership */

/* We don't use the following since our default TTL is 64, we
 *  default to the user configuration setting for loop-back setting
 *  (in [pctcp kernel] section,
 *  and the max memberships depends on the driver capability (besides
 *  the fact ours is not on a per socket basis)
 */
#define IP_DEFAULT_MULTICAST_TTL	1
#define IP_DEFAULT_MULTICAST_LOOP	1
#define IP_MAX_MEMBERSHIPS		20

/* The structure used to add and drop multicast addresses  (the
 *  imr_interface field is ignored since we only support one interface */
typedef struct ip_mreq {
	struct in_addr imr_multiaddr;	/* multicast group to join */
	struct in_addr imr_interface;	/* interface to join on */
}IP_MREQ;
#endif
