
#ifndef __log_h__
#define __log_h__

#ifdef YACAS_LOGGING // DEBUG
#include <stdarg.h>
#include <stdio.h>
#endif

inline void LogPrintf(char* str,...)
{
#ifdef YACAS_LOGGING // DEBUG
  va_list arg;
  char buf[256];
  va_start (arg, str);
  vsprintf (buf, str, arg);
  va_end (arg);
  printf(buf);
#endif
}

inline void LogErrorPrintf(char* str,...)
{
#ifdef YACAS_LOGGING // DEBUG
  printf("ERROR: ");
  va_list arg;
  char buf[256];
  va_start (arg, str);
  vsprintf (buf, str, arg);
  va_end (arg);
  printf(buf);
  exit(0);
#endif
}


#endif
