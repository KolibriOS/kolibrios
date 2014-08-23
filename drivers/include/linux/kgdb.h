#ifndef _KDB_H
#define _KDB_H

/*
 * Kernel Debugger Architecture Independent Global Headers
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (c) 2000-2007 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (C) 2000 Stephane Eranian <eranian@hpl.hp.com>
 * Copyright (C) 2009 Jason Wessel <jason.wessel@windriver.com>
 */

typedef enum {
    KDB_REPEAT_NONE = 0,    /* Do not repeat this command */
    KDB_REPEAT_NO_ARGS, /* Repeat the command without arguments */
    KDB_REPEAT_WITH_ARGS,   /* Repeat the command including its arguments */
} kdb_repeat_t;

typedef int (*kdb_func_t)(int, const char **);

#endif  /* !_KDB_H */
