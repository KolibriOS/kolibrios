#ifndef _LINUX_MODULE_PARAMS_H
#define _LINUX_MODULE_PARAMS_H
/* (C) Copyright 2001, 2002 Rusty Russell IBM Corporation */
#include <linux/kernel.h>

#define MODULE_PARM_DESC(_parm, desc)
#define module_param_named(name, value, type, perm)
#define module_param_named_unsafe(name, value, type, perm)

#endif
