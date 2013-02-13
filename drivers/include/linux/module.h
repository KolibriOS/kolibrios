#ifndef _LINUX_MODULE_H
#define _LINUX_MODULE_H
/*
 * Dynamic loading of modules into the kernel.
 *
 * Rewritten by Richard Henderson <rth@tamu.edu> Dec 1996
 * Rewritten again by Rusty Russell, 2002
 */
#include <linux/list.h>
#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/export.h>



#define MODULE_FIRMWARE(x)
#define MODULE_AUTHOR(x);
#define MODULE_DESCRIPTION(x);
#define MODULE_LICENSE(x);

#define MODULE_PARM_DESC(_parm, desc)

struct module {};

#endif /* _LINUX_MODULE_H */
