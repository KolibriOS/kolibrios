#ifndef _SOC_H_
#define _SOC_H_

#include "types.h"

//#define GDB_SUPPORT
//#define DYNAREC
#define MAX_WTP			32
#define MAX_BKPT		32




#ifndef GDB_SUPPORT
	#ifdef DYNAREC
		#define _JIT
	#endif
#endif



#define CHAR_CTL_C	-1L
#define CHAR_NONE	-2L
typedef int (*readcharF)(void);
typedef void (*writecharF)(int);

#define BLK_DEV_BLK_SZ	512

#define BLK_OP_SIZE	0
#define BLK_OP_READ	1
#define BLK_OP_WRITE	2

typedef int (*blockOp)(void* data, UInt32 sec, void* ptr, UInt8 op); 

struct SoC;

typedef void (*SocRamAddF)(struct SoC* soc, void* data);

typedef struct{
	
	UInt32 (*WordGet)(UInt32 wordAddr);
	void (*WordSet)(UInt32 wordAddr, UInt32 val);
	
}RamCallout;

void socRamModeAlloc(struct SoC* soc, void* ignored);
void socRamModeCallout(struct SoC* soc, void* callout);	//rally pointer to RamCallout

void socInit(struct SoC* soc, SocRamAddF raF, void* raD, readcharF rc, writecharF wc, blockOp blkF, void* blkD);
void socRun(struct SoC* soc, UInt32 gdbPort);



extern volatile UInt32 gRtc;	//needed by SoC

#include "CPU.h"
#include "MMU.h"
#include "mem.h"
#include "callout_RAM.h"
#include "RAM.h"
#include "cp15.h"
#include "math64.h"
#include "pxa255_IC.h"
#include "pxa255_TIMR.h"
#include "pxa255_RTC.h"
#include "pxa255_UART.h"
#include "pxa255_PwrClk.h"
#include "pxa255_GPIO.h"
#include "pxa255_DMA.h"
#include "pxa255_DSP.h"
#include "pxa255_LCD.h"

typedef struct SoC{

	readcharF rcF;
	writecharF wcF;

	blockOp blkF;
	void* blkD;
	
	UInt32 blkDevBuf[BLK_DEV_BLK_SZ / sizeof(UInt32)];

	union{
		ArmRam RAM;
		CalloutRam coRAM;
	}ram;
	ArmRam ROM;
	ArmCpu cpu;
	ArmMmu mmu;
	ArmMem mem;
	ArmCP15 cp15;
	Pxa255ic ic;
	Pxa255timr timr;
	Pxa255rtc rtc;
	Pxa255uart ffuart;
	Pxa255uart btuart;
	Pxa255uart stuart;
	Pxa255pwrClk pwrClk;
	Pxa255gpio gpio;
	Pxa255dma dma;
	Pxa255dsp dsp;
	Pxa255lcd lcd;
	
	UInt8 go	:1;
	UInt8 calloutMem:1;
	
	UInt32 romMem[13];		//space for embeddedBoot
	
#ifdef GDB_SUPPORT
	
	UInt8 nBkpt, nWtp;
	UInt32 bkpt[MAX_BKPT];
	UInt32 wtpA[MAX_WTP];
	UInt8 wtpS[MAX_WTP];
	
#endif

}SoC;

#endif

