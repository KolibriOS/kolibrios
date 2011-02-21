#ifndef __MENUET_MASH_H
#define __MENUET_MASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include<menuet/os.h>

extern void __mash__puts(char * str);
extern void __mash__gets(char * str,int len);

extern char mash_args[];

#ifdef __cplusplus
}
#endif

#endif
