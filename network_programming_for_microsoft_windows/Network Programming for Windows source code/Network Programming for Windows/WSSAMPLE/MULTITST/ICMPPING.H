/*
 * Filename: ICMPPING.H
 *
 * Description: essentials for WinSock Ping application
 *  (which has potential for traceroute capability over
 *  setsockopt() IP_TTL capable WinSock implementations)
 */
#define IP_TTL	 4	/* level=IPPROTO_IP option, Time To Live */
#define ICMP_ECHOREPLY	0	/* ICMP type: echo reply */
#define ICMP_ECHOREQ	8	/* ICMP type: echo request */
typedef struct icmp_hdr {	/* ICMP as per RFC 792 */
	u_char	icmp_type;		/* type of message */
	u_char	icmp_code;		/* type sub code */
	u_short icmp_cksum;		/* ones complement cksum */
	u_short	icmp_id;		/* identifier */
	u_short	icmp_seq;		/* sequence number */
	char	icmp_data[1];	/* data */
} ICMP_HDR, *PICMPHDR, FAR *LPICMPHDR;
#define ICMP_HDR_LEN	sizeof(ICMP_HDR)
#define	IPVERSION	4
typedef struct ip_hdr {	/* IP version 4 as per RFC 791 */
	u_char	ip_hl;		/* header length */
	u_char	ip_v;		/* version */
	u_char	ip_tos;		/* type of service */
	short	ip_len;		/* total length */
	u_short	ip_id;		/* identification */
	short	ip_off;		/* fragment offset field */
	u_char	ip_ttl;		/* time to live */
	u_char	ip_p;		/* protocol */
	u_short	ip_cksum;		/* checksum */
	struct	in_addr ip_src;	/* source address */
	struct	in_addr ip_dst;	/* destination address */
} IP_HDR, *PIP_HDR, *LPIP_HDR;
#define IP_HDR_LEN sizeof(IP_HDR)
#define MAX_TTL	255

#define PNGBUFSIZE 8192+ICMP_HDR_LEN+IP_HDR_LEN
