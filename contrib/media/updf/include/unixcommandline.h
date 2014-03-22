
#ifndef __unixcommandline_h__
#define __unixcommandline_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>

#include "yacasbase.h"
#include "commandline.h"

/** Unix command line class, using assorted termios functionality
 *  and sending ansi character sequences to the console.
 */
class CUnixCommandLine : public CCommandLine
{
public:
  CUnixCommandLine();
  ~CUnixCommandLine();
public:
  virtual LispInt GetKey();
  virtual void NewLine();
  virtual void ShowLine(LispChar * prompt,LispInt promptlen,LispInt cursor);
  virtual void Pause();
  virtual void MaxHistoryLinesSaved(LispInt aNrLines);
private:
  unsigned char term_chars[NCCS];
  struct termios orig_termio, rl_termio;
public:
  LispInt iMaxLines;
};


#endif

