
#ifndef __stdcommandline_h__
#define __stdcommandline_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#ifndef WIN32
#   include <unistd.h>
#   include <termios.h>
#endif
#include <time.h>

#include "yacasbase.h"
#include "commandline.h"
/** Simple no-frills implementation of CCommandLine, using stdlibc-functions
 *  only, and no ansi characters. No history is supported either.
 */
class CStdCommandLine : public CCommandLine
{
public:
    CStdCommandLine();
    ~CStdCommandLine();
    virtual void ReadLine(LispChar * prompt);
public:
    virtual LispInt GetKey();
    virtual void NewLine();
    virtual void ShowLine(LispChar * prompt,LispInt promptlen,LispInt cursor);
    virtual void Pause();
};


#endif

