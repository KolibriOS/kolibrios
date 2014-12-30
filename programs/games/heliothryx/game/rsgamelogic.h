#ifndef RSGAMELOGIC_H_INCLUDED
#define RSGAMELOGIC_H_INCLUDED

/*

    Heliothryx
    Game by Roman Shuvalov

*/

#ifndef RS_LINUX
    #ifndef RS_WIN32
        #ifndef RS_KOS
            #error Please specify platform
        #endif
    #endif
#endif


#include "rsgame.h"


void GameProcess();


#endif // RSGAME_H_INCLUDED
