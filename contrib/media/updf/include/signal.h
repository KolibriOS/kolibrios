/*---------------------------------------------------------------------------*/
/* signal.h - signal header file                                             */
/*---------------------------------------------------------------------------*/
#define SIGINT          1
#define SIGILL          2
#define SIGFPE          3
#define SIGSEGV         4
#define SIGTERM         5
#define SIGBREAK        6
#define SIGABRT         7

typedef void (* PFSIG)(int);

#define SIG_DFL ((PFSIG)0)
#define SIG_IGN ((PFSIG)1)
#define SIG_ERR ((PFSIG)-1)

/*---------------------------------------------------------------------------*/
/* function prototypes                                                       */
/*---------------------------------------------------------------------------*/
int raise(int sig);
PFSIG signal(int, PFSIG);
