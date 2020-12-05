#ifndef _PXA255_IC_H_
#define _PXA255_IC_H_

#include "mem.h"
#include "CPU.h"
#include <stdio.h> 

/*
	PXA255 interrupt controller
	
	PURRPOSE: raises IRQ, FIQ as needed

*/

#define PXA255_IC_BASE	0x40D00000UL
#define PXA255_IC_SIZE	0x00010000UL


#define PXA255_I_RTC_ALM	31
#define PXA255_I_RTC_HZ		30
#define PXA255_I_TIMR3		29
#define PXA255_I_TIMR2		28
#define PXA255_I_TIMR1		27
#define PXA255_I_TIMR0		26
#define PXA255_I_DMA		25
#define PXA255_I_SSP		24
#define PXA255_I_MMC		23
#define PXA255_I_FFUART		22
#define PXA255_I_BTUART		21
#define PXA255_I_STUART		20
#define PXA255_I_ICP		19
#define PXA255_I_I2C		18
#define PXA255_I_LCD		17
#define PXA255_I_NET_SSP	16
#define PXA255_I_AC97		14
#define PXA255_I_I2S		13
#define PXA255_I_PMU		12
#define PXA255_I_USB		11
#define PXA255_I_GPIO_all	10
#define PXA255_I_GPIO_1		9
#define PXA255_I_GPIO_0		8
#define PXA255_I_HWUART		7


typedef struct{

	ArmCpu* cpu;
	
	UInt32 ICMR;	//Mask Register
	UInt32 ICLR;	//Level Register
	UInt32 ICCR;	//Control Register
	UInt32 ICPR;	//Pending register
	
	Boolean wasIrq, wasFiq;
	
}Pxa255ic;

Boolean pxa255icInit(Pxa255ic* ic, ArmCpu* cpu, ArmMem* physMem);

void pxa255icInt(Pxa255ic* ic, UInt8 intNum, Boolean raise);		//interrupt caused by emulated hardware/ interrupt handled by guest


#endif

