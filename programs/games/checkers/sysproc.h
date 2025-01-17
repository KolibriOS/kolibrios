#ifndef _HEADER_SYSTEM_PROCESS_H
#define _HEADER_SYSTEM_PROCESS_H

#include <stdlib.h>
#include <time.h>

#if defined _KOLIBRI
# define DIR_SEPARATOR  ('/')
  inline long GetProcessId() {return 0;}
  inline long DuplicateProcess() {return -1;}
  inline int random(int m) {return ((unsigned long)rand()) % m;}
  inline void randomize() {srand(time(0));}
#elif defined __GNUC__
# include <unistd.h>
# define DIR_SEPARATOR	('/')
  inline long GetProcessId() {return (long)getpid();}
  inline long DuplicateProcess() {return (long)fork();}
  inline int random(int m) {return ((unsigned long)rand()) % m;}
  inline void randomize() {srand(time(0));}
#elif defined __TURBOC__
# include <process.h>
# define DIR_SEPARATOR	('\\')
  inline long GetProcessId() {return (long)getpid();}
  inline long DuplicateProcess() {return -1;}
#else
# define DIR_SEPARATOR	('\\')
  inline long GetProcessId() {return 0;}
  inline long DuplicateProcess() {return -1;}
#endif

#endif  //_HEADER_SYSTEM_PROCESS_H
