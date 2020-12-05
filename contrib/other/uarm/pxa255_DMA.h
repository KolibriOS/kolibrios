#ifndef _PXA255_DMA_H_
#define _PXA255_DMA_H_

#include "mem.h"
#include "cpu.h"
#include "pxa255_IC.h"

/*
	PXA255 OS DMA controller

*/

#define PXA255_DMA_BASE		0x40000000UL
#define PXA255_DMA_SIZE		0x00001000UL

typedef struct{
	
	UInt32 DAR;	//descriptor address register
	UInt32 SAR;	//source address register
	UInt32 TAR;	//target address register
	UInt32 CR;	//command register
	UInt32 CSR;	//control and status register
	
}Pxa255dmaChannel;

typedef struct{

	Pxa255ic* ic;
	ArmMem* mem;
	
	UInt16 DINT;
	Pxa255dmaChannel channels[16];
	UInt8 CMR[40];			//channel map registers	[  we store lower 8 bits only :-)  ]
	
}Pxa255dma;



Boolean pxa255dmaInit(Pxa255dma* gpio, ArmMem* physMem, Pxa255ic* ic);

#endif

