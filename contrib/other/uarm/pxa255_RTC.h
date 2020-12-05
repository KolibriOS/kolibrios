#ifndef _PXA255_RTC_H_
#define _PXA255_RTC_H_

#include "mem.h"
#include "cpu.h"
#include "pxa255_IC.h"


/*
	PXA255 OS RTC controller
	
	PURRPOSE: it's nice to know what time it is

*/

#define PXA255_RTC_BASE		0x40900000UL
#define PXA255_RTC_SIZE		0x00001000UL


typedef struct{

	Pxa255ic* ic;
	
	UInt32 RCNR_offset;	//RTC counter offset from our local time
	UInt32 RTAR;		//RTC alarm
	UInt32 RTSR;		//RTC status
	UInt32 RTTR;		//RTC trim - we ignore this alltogether
	UInt32 lastSeenTime;	//for HZ interrupt
	
}Pxa255rtc;

Boolean pxa255rtcInit(Pxa255rtc* rtc, ArmMem* physMem, Pxa255ic* ic);
void pxa255rtcUpdate(Pxa255rtc* rtc);


#endif

