#ifndef _NET_IF_H
#define _NET_IF_H

#include <sys/cdefs.h>
#include <sys/socket.h>

__BEGIN_DECLS

/* Standard interface flags. */
#define IFF_UP		0x1		/* interface is up		*/
#define IFF_BROADCAST	0x2		/* broadcast address valid	*/
#define IFF_DEBUG	0x4		/* turn on debugging		*/
#define IFF_LOOPBACK	0x8		/* is a loopback net		*/
#define IFF_POINTOPOINT	0x10		/* interface is has p-p link	*/
#define IFF_NOTRAILERS	0x20		/* avoid use of trailers	*/
#define IFF_RUNNING	0x40		/* resources allocated		*/
#define IFF_NOARP	0x80		/* no ARP protocol		*/
#define IFF_PROMISC	0x100		/* receive all packets		*/
#define IFF_ALLMULTI	0x200		/* receive all multicast packets*/

#define IFF_MASTER	0x400		/* master of a load balancer 	*/
#define IFF_SLAVE	0x800		/* slave of a load balancer	*/

#define IFF_MULTICAST	0x1000		/* Supports multicast		*/

#define IFF_VOLATILE	(IFF_LOOPBACK|IFF_POINTOPOINT|IFF_BROADCAST|IFF_MASTER|IFF_SLAVE|IFF_RUNNING)

#define IFF_PORTSEL	0x2000          /* can set media type		*/
#define IFF_AUTOMEDIA	0x4000		/* auto media select active	*/
#define IFF_DYNAMIC	0x8000		/* dialup device with changing addresses*/

struct ifmap {
  unsigned long mem_start;
  unsigned long mem_end;
  unsigned short base_addr; 
  unsigned char irq;
  unsigned char dma;
  unsigned char port;
  /* 3 bytes spare */
};

struct ifreq {
#define IFHWADDRLEN	6
#define IFNAMSIZ	16
  union
  {
    char	ifrn_name[IFNAMSIZ];		/* if name, e.g. "en0" */
  } ifr_ifrn;
  union {
    struct sockaddr ifru_addr;
    struct sockaddr ifru_dstaddr;
    struct sockaddr ifru_broadaddr;
    struct sockaddr ifru_netmask;
    struct  sockaddr ifru_hwaddr;
    short ifru_flags;
    int ifru_ivalue;
    int ifru_mtu;
    struct ifmap ifru_map;
    char ifru_slave[IFNAMSIZ];	/* Just fits the size */
    char ifru_newname[IFNAMSIZ];
    char* ifru_data;
  } ifr_ifru;
};

#define ifr_name	ifr_ifrn.ifrn_name	/* interface name 	*/
#define ifr_hwaddr	ifr_ifru.ifru_hwaddr	/* MAC address 		*/
#define ifr_addr	ifr_ifru.ifru_addr	/* address		*/
#define ifr_dstaddr	ifr_ifru.ifru_dstaddr	/* other end of p-p lnk	*/
#define ifr_broadaddr	ifr_ifru.ifru_broadaddr	/* broadcast address	*/
#define ifr_netmask	ifr_ifru.ifru_netmask	/* interface net mask	*/
#define ifr_flags	ifr_ifru.ifru_flags	/* flags		*/
#define ifr_metric	ifr_ifru.ifru_ivalue	/* metric		*/
#define ifr_mtu		ifr_ifru.ifru_mtu	/* mtu			*/
#define ifr_map		ifr_ifru.ifru_map	/* device map		*/
#define ifr_slave	ifr_ifru.ifru_slave	/* slave device		*/
#define ifr_data	ifr_ifru.ifru_data	/* for use by interface	*/
#define ifr_ifindex	ifr_ifru.ifru_ivalue	/* interface index	*/
#define ifr_bandwidth	ifr_ifru.ifru_ivalue    /* link bandwidth	*/
#define ifr_qlen	ifr_ifru.ifru_ivalue	/* Queue length 	*/
#define ifr_newname	ifr_ifru.ifru_newname	/* New name		*/

struct ifconf {
  int ifc_len;			/* size of buffer	*/
  union {
    char *			ifcu_buf;
    struct	ifreq 		*ifcu_req;
  } ifc_ifcu;
};

#define ifc_buf ifc_ifcu.ifcu_buf		/* buffer address	*/
#define ifc_req ifc_ifcu.ifcu_req		/* array of structures	*/

unsigned int if_nametoindex (const char *ifname) __THROW;
char *if_indextoname (unsigned int ifindex, char *ifname) __THROW;

struct if_nameindex {
  unsigned int if_index;
  char *if_name;
};

struct if_nameindex* if_nameindex(void) __THROW;
void if_freenameindex(struct if_nameindex* ptr) __THROW;

__END_DECLS

#endif
