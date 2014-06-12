/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <errno.h>
#include "syserr1.h"

const char *__sys_errlist[] = {
__syserr00, __syserr01, __syserr02, __syserr03, __syserr04,
__syserr05, __syserr06, __syserr07, __syserr08, __syserr09,
__syserr10, __syserr11, __syserr12, __syserr13, __syserr14,
__syserr15, __syserr16, __syserr17, __syserr18, __syserr19,
__syserr20, __syserr21, __syserr22, __syserr23, __syserr24,
__syserr25, __syserr26, __syserr27, __syserr28, __syserr29,
__syserr30, __syserr31, __syserr32, __syserr33, __syserr34,
__syserr35, __syserr36, __syserr37, __syserr38
};

int __sys_nerr = sizeof(__sys_errlist) / sizeof(__sys_errlist[0]);
