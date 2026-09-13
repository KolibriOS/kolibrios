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
#include <linux/cache.h>
#include <linux/init.h>

#include <linux/kobject.h>
#include <linux/moduleparam.h>
#include <linux/export.h>
#include <linux/printk.h>


#define MODULE_FIRMWARE(x)
#define MODULE_AUTHOR(x);
#define MODULE_DESCRIPTION(x);
#define MODULE_LICENSE(x);

#define MODULE_PARM_DESC(_parm, desc)

struct module {};

/*
 * Nothing is a module here - everything is linked into the one image - so the
 * reference count can never drop and a request for another module can never be
 * satisfied.  Implemented in the driver's kos_* glue.
 */
extern int request_module(const char *name, ...);
extern int try_module_get(struct module *module);
extern void module_put(struct module *module);


#endif /* _LINUX_MODULE_H */
