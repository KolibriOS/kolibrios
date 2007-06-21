
#include "variety.h"
#include "initfini.h"

_WCRTLINKD extern char   *LpCmdLine;    /* pointer to command line */
_WCRTLINKD extern char   *LpPgmName;    /* pointer to program name */
_WCRTLINKD int     ___Argc; /* argument count */
_WCRTLINKD char *  ___Argv[2]; /* argument vector */
_WCRTLINK void exit( int status );

_WCRTLINK extern void  (*__process_fini)( unsigned, unsigned );

extern int main( int, char ** );

#pragma aux __KolibriMain "*";
   
void __KolibriMain(void )
/***************************************/
{   ___Argc = 2;
    ___Argv[0] = LpPgmName;
    ___Argv[1] = LpCmdLine;
     
//    __process_fini = &__FiniRtns;
    __InitRtns( 255 );
//        __CommonInit();
    exit( main( ___Argc, ___Argv ) );

}





