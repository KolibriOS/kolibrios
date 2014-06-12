#ifndef _NETDB_H
#define _NETDB_H

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

/* Absolute file name for network data base files.  */
#define	_PATH_HEQUIV		"/etc/hosts.equiv"
#define	_PATH_HOSTS		"/etc/hosts"
#define	_PATH_NETWORKS		"/etc/networks"
#define	_PATH_NSSWITCH_CONF	"/etc/nsswitch.conf"
#define	_PATH_PROTOCOLS		"/etc/protocols"
#define	_PATH_SERVICES		"/etc/services"

/* Description of data base entry for a single service.  */
struct servent {
  char *s_name;			/* Official service name.  */
  char **s_aliases;		/* Alias list.  */
  int s_port;			/* Port number.  */
  char *s_proto;		/* Protocol to use.  */
};

extern void endservent (void) ;
extern void setservent(int stayopen) ;

extern int getservent_r(struct servent *res, char *buf, size_t buflen,
			 struct servent **res_sig) ;
extern int getservbyname_r(const char* name,const char* proto,
			   struct servent *res, char *buf, size_t buflen,
			   struct servent **res_sig) ;
extern int getservbyport_r(int port,const char* proto,
			   struct servent *res, char *buf, size_t buflen,
			   struct servent **res_sig) ;

extern struct servent *getservent(void) ;
extern struct servent *getservbyname (const char *__name,
				      const char *__proto) ;
extern struct servent *getservbyport (int __port, const char *__proto)
     ;

struct hostent {
  char *h_name;			/* Official name of host.  */
  char **h_aliases;		/* Alias list.  */
  int h_addrtype;		/* Host address type.  */
  socklen_t h_length;		/* Length of address.  */
  char **h_addr_list;		/* List of addresses from name server.  */
#define	h_addr	h_addr_list[0]	/* Address, for backward compatibility.  */
};

extern void endhostent (void) ;
extern struct hostent *gethostent (void) ;
extern struct hostent *gethostent_r (char* buf,int len) ;
extern struct hostent *gethostbyaddr (const void *__addr, socklen_t __len,
				      int __type) ;
extern struct hostent *gethostbyname (const char *__name) ;
extern struct hostent *gethostbyname2 (const char *__name, int __af) ;

/* this glibc "invention" is so ugly, I'm going to throw up any minute
 * now */
extern int gethostbyname_r(const char* NAME, struct hostent* RESULT_BUF,char* BUF,
			   size_t BUFLEN, struct hostent** RESULT,
			   int* H_ERRNOP) ;

#define HOST_NOT_FOUND 1
#define TRY_AGAIN 2
#define NO_RECOVERY 3
#define NO_ADDRESS 4
#define NO_DATA 5

extern int gethostbyaddr_r(const char* addr, size_t length, int format,
		    struct hostent* result, char *buf, size_t buflen,
		    struct hostent **RESULT, int *h_errnop) ;

int gethostbyname2_r(const char* name, int AF, struct hostent* result,
		    char *buf, size_t buflen,
		    struct hostent **RESULT, int *h_errnop) ;

struct protoent {
  char    *p_name;        /* official protocol name */
  char    **p_aliases;    /* alias list */
  int     p_proto;        /* protocol number */
};

struct protoent *getprotoent(void) ;
struct protoent *getprotobyname(const char *name) ;
struct protoent *getprotobynumber(int proto) ;
void setprotoent(int stayopen) ;
void endprotoent(void) ;

int getprotoent_r(struct protoent *res, char *buf, size_t buflen,
		  struct protoent **res_sig) ;
int getprotobyname_r(const char* name,
		     struct protoent *res, char *buf, size_t buflen,
		     struct protoent **res_sig) ;
int getprotobynumber_r(int proto,
		      struct protoent *res, char *buf, size_t buflen,
		      struct protoent **res_sig) ;


void sethostent(int stayopen) ;

/* dummy */
extern int h_errno;

struct netent {
  char    *n_name;          /* official network name */
  char    **n_aliases;      /* alias list */
  int     n_addrtype;       /* net address type */
  unsigned long int n_net;  /* network number */
};

struct netent *getnetbyaddr(unsigned long net, int type) ;
void endnetent(void) ;
void setnetent(int stayopen) ;
struct netent *getnetbyname(const char *name) ;
struct netent *getnetent(void) ;

extern const char *hstrerror (int err_num) ;
void herror(const char *s) ;

#define NI_MAXHOST 1025
#define NI_MAXSERV 32

__END_DECLS

#endif
