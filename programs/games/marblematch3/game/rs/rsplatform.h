#ifndef RS_PLATFORM_FOR_KOLIBRI_H
#define RS_PLATFORM_FOR_KOLIBRI_H

#include "../../system/sound.h"
#include "../../system/kolibri.h"
#include "rsmicrolibc.h"


#ifndef uint32_t
	#define uint32_t unsigned int
#endif


#define RS_KEY_DOWN		80
#define RS_KEY_UP		72
#define RS_KEY_LEFT		75
#define RS_KEY_RIGHT	77

#define RS_KEY_RETURN	28
#define RS_KEY_ESCAPE	1
#define RS_KEY_SPACE	57
#define RS_KEY_CONTROL_L	29

#define RS_KEY_1		2
#define RS_KEY_2		3
#define RS_KEY_3		4
#define RS_KEY_4		5
#define RS_KEY_5		6
#define RS_KEY_6		7
#define RS_KEY_7		8
#define RS_KEY_8		9
#define RS_KEY_9		10
#define RS_KEY_0		11

#define RS_KEY_P		25

#define RS_KEY_A		30
#define RS_KEY_S		31
#define RS_KEY_Z		44
#define RS_KEY_X		45


unsigned int get_time();


typedef void RSFUNC0(); 
/*typedef void RSFUNC1i(int);
typedef void RSFUNC2i(int,int);
typedef void RSFUNC1i1f(int,float); */

typedef RSFUNC0 *PRSFUNC0;
/*typedef RSFUNC1i *PRSFUNC1i;
typedef RSFUNC2i *PRSFUNC2i;
typedef RSFUNC1i1f *PRSFUNC1i1f;*/

/*
void NullFunc0();
void NullFunc1i(int i);
void NullFunc2i(int i, int j);
void NullFunc1i1f(int i, float f);

*/

typedef struct rs_app_t {

    unsigned short app_time;
    unsigned short delta_time;
    
    /*

    PRSFUNC2i OnKeyDown;
    PRSFUNC1i OnKeyUp;
    
    PRSFUNC2i OnMouseDown;
    PRSFUNC2i OnMouseUp;

    PRSFUNC0 OnAppProcess;
    
    PRSFUNC0 rsAppOnInitDisplay;
    PRSFUNC0 rsAppOnTermDisplay;
    
    */

} rs_app_t;

extern rs_app_t rs_app;

//void rsAppZero();



#endif
