#ifndef _PXA255_LCD_H_
#define _PXA255_LCD_H_

#include "mem.h"
#include "cpu.h"
#include "pxa255_IC.h"

#ifndef EMBEDDED
	#define PXA255_LCD_SUPPORTS_PALLETES
#endif


/*
	PXA255 OS LCD controller
	
	PURRPOSE: it's nice to have a framebuffer

*/

#define PXA255_LCD_BASE		0x44000000UL
#define PXA255_LCD_SIZE		0x00001000UL




#define LCD_STATE_IDLE		0
#define LCD_STATE_DMA_0_START	1
#define LCD_STATE_DMA_0_END	2

typedef struct{

	Pxa255ic* ic;
	ArmMem* mem;
	
	//registers
	UInt32 lccr0, lccr1, lccr2, lccr3, fbr0, fbr1, liicr, trgbr, tcr;
	UInt32 fdadr0, fsadr0, fidr0, ldcmd0;
	UInt32 fdadr1, fsadr1, fidr1, ldcmd1;
	UInt16 lcsr;	//yes, 16-bit :)
	
	//for our use
	UInt16 intMask;
	
	UInt8 state		: 6;
	UInt8 intWasPending	: 1;
	UInt8 enbChanged	: 1;
	
#ifdef PXA255_LCD_SUPPORTS_PALLETES

	UInt8 palette[512];

#endif

	UInt32 frameNum;
	
}Pxa255lcd;

Boolean pxa255lcdInit(Pxa255lcd* lcd, ArmMem* physMem, Pxa255ic* ic);
void pxa255lcdFrame(Pxa255lcd* lcd);


#endif

