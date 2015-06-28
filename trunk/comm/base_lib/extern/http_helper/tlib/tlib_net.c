#ifndef _TLIB_NET_C_
#define _TLIB_NET_C_

#include "tlib_net.h"

////////////////////////////////////////////////////
struct ipovly {
	caddr_t	ih_next, ih_prev;	/* for protocol sequence q's */
	u_char	ih_x1;			/* (unused) */
	u_char	ih_pr;			/* protocol */
	short	ih_len;			/* protocol length */
	struct	in_addr ih_src;		/* source internet address */
	struct	in_addr ih_dst;		/* destination internet address */
};

struct	udpiphdr {
	struct 	ipovly ui_i;		/* overlaid ip structure */
	struct	udphdr ui_u;		/* udp header */
};
#define	ui_next		ui_i.ih_next
#define	ui_prev		ui_i.ih_prev
#define	ui_x1		ui_i.ih_x1
#define	ui_pr		ui_i.ih_pr
#define	ui_len		ui_i.ih_len
#define	ui_src		ui_i.ih_src
#define	ui_dst		ui_i.ih_dst
#define	ui_sport	ui_u.source
#define	ui_dport	ui_u.dest
#define	ui_ulen		ui_u.len
#define	ui_sum		ui_u.check

#define	UDP_TTL		30		/* time to live for UDP packets */
#define	TTL_OUT		64		/* outgoing TTL */
//////////////////////////////////////////////////////////////////////////

#define TLIB_NET_BUF_SIZE 1024*10

unsigned short TLib_Net_InCksum(unsigned short *addr, int len)
{
	int nleft = len;
	int sum = 0;
	unsigned short *w = addr;
	unsigned short answer = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}

	/* 4mop up an odd byte, if necessary */
	if (nleft == 1) 
	{
		*(unsigned char *)(&answer) = *(unsigned char *)w ;
		sum += answer;
	}

	/* 4add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return(answer);
}

int TLib_Net_GetUDPSocket(char *sErrMsg)
{
	int iRawSocket;
	int iRetCode;
	int on=1;
	
	iRawSocket = socket(AF_INET, SOCK_RAW, htons(ETH_P_IP));
	if (iRawSocket < 0)
	{
		snprintf(sErrMsg,1024, strerror(errno));
		return iRawSocket;
	}
	
	iRetCode = setsockopt(iRawSocket, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));
	if (iRetCode < 0)
	{
		snprintf(sErrMsg,1024, strerror(errno));
		close(iRawSocket);
		return iRetCode;
	}
	
	return iRawSocket;
}

int TLib_Net_AToPort(char *service, char *proto)
{
	int port;
	long int lport;
	struct servent *serv;
	char *errpos;

	/* First try to read it from /etc/services */
	serv = getservbyname(service, proto);
	if (serv != NULL)
		port = serv->s_port;
	else /* Not in services, maybe a number? */
	{
		lport = strtol(service,&errpos,0);
		if ((errpos[0] != 0) || (lport < 1) || (lport > 65535))
			return -1; /* Invalid port address */
		port = htons(lport);
	}
	return port;
}

struct in_addr *TLib_Net_AToAddr(char *address)
{
	struct hostent *host;
	static struct in_addr saddr;

	/* First try it as aaa.bbb.ccc.ddd. */
	saddr.s_addr = inet_addr(address);
	if (saddr.s_addr != -1)
	{
		return &saddr;
	}
	host = gethostbyname(address);
	if (host != NULL)
	{
		return (struct in_addr *) *host->h_addr_list;
	}
	return NULL;
}

int TLib_Net_PToN(char *sAddr, char *sPort, struct sockaddr_in *pstIAddr, char *sErrMsg)
{
	int iPort = -1;
	struct in_addr *pstAddr;

	iPort = TLib_Net_AToPort(sPort, "udp");
	if (iPort == -1)
	{
		snprintf(sErrMsg,1024, "Invalid socket type");
		return -1;
	}

	pstAddr = TLib_Net_AToAddr(sAddr);
	if (pstAddr == NULL) 
	{
		snprintf(sErrMsg,1024, "Invalid network address");
		return -1;
	}
 
	memset((char *) pstIAddr, 0, sizeof(struct sockaddr_in));
	pstIAddr->sin_family = AF_INET;
	pstIAddr->sin_port = iPort;
	pstIAddr->sin_addr.s_addr = pstAddr->s_addr;
	
	return 0;
}

int TLib_Net_WriteUDPSocketDef(int iUDPSocket, 
				struct sockaddr_in *pstSrcAddr,
				struct sockaddr_in *pstDestAddr,
				char *sBuf, int iLen, char *sErrMsg)
{
	static char buf[TLIB_NET_BUF_SIZE+1];
	struct udpiphdr	*ui;
	struct ip *ip;
	int userlen;
	char *ptr;
	
	ptr = buf + sizeof(struct udpiphdr);/* leave room for IP/UDP headers */
	memcpy(ptr, sBuf, iLen);
	
	
	userlen = iLen;

	/* 4Fill in and checksum UDP header */
	ip = (struct ip *) buf;
	ui = (struct udpiphdr *) buf;
	/* 8add 8 to userlen for pseudo-header length */
	ui->ui_len = htons((u_short) (sizeof(struct udphdr) + userlen));
	/* 8then add 28 for IP datagram length */
	userlen += sizeof(struct udpiphdr);

	ui->ui_next = 0;
	ui->ui_prev = 0;
	ui->ui_x1 = 0;
	ui->ui_pr = IPPROTO_UDP;
	ui->ui_src.s_addr = pstSrcAddr->sin_addr.s_addr;
	ui->ui_dst.s_addr = pstDestAddr->sin_addr.s_addr;
	ui->ui_sport = pstSrcAddr->sin_port;
	ui->ui_dport = pstDestAddr->sin_port;
	ui->ui_ulen = ui->ui_len;
	ui->ui_sum = 0;
	if ((ui->ui_sum = TLib_Net_InCksum((u_short *) ui, userlen)) == 0)
		ui->ui_sum = 0xffff;

	/* 4Fill in rest of IP header; */
	/* 4ip_output() calcuates & stores IP header checksum */
	ip->ip_v = IPVERSION;
	ip->ip_hl = sizeof(struct ip) >> 2;
	ip->ip_tos = 0;
	ip->ip_len = htons(userlen);	/* network byte order */
	ip->ip_id = 0;			/* let IP set this */
	ip->ip_off = 0;			/* frag offset, MF and DF flags */
	ip->ip_ttl = TTL_OUT;

	if (sendto(iUDPSocket, buf, userlen, 0, pstDestAddr, sizeof(struct sockaddr_in)) != userlen)
	{
		snprintf(sErrMsg,1024, "Fail To Send UDP");
		return -1;
	}
	
	return 0;
}

int TLib_Net_WriteUDPSocket(int iUDPSocket, 
				char *sSrcAddr, char *sSrcPort, 
				char *sDestAddr, char *sDestPort, 
				char *sBuf, int iLen, char *sErrMsg)
{
	struct sockaddr_in stSrcAddr, stDestAddr;
	int iRetCode;


	iRetCode = TLib_Net_PToN(sSrcAddr, sSrcPort, &stSrcAddr, sErrMsg);
	if (iRetCode < 0)
	{
		return iRetCode;
	}

	iRetCode = TLib_Net_PToN(sDestAddr, sDestPort, &stDestAddr, sErrMsg);
	if (iRetCode < 0)
	{
		return iRetCode;
	}
	
	iRetCode = TLib_Net_WriteUDPSocketDef(iUDPSocket, &stSrcAddr, &stDestAddr, sBuf, iLen, sErrMsg);
	
	return iRetCode;
}

int TLib_Net_CloseUDPSocket(int iUDPSocket)
{
	return close(iUDPSocket);
}

#endif

