#ifndef _PXA255_PWR_CLK_H_
#define _PXA255_PWR_CLK_H_

#include "mem.h"
#include "cpu.h"



typedef struct{
	
	ArmCpu* cpu;
	UInt32 CCCR, CKEN, OSCR;	//clocks manager regs
	UInt32 pwrRegs[13];		//we care so little about these, we don't even name them
	Boolean turbo;
	
}Pxa255pwrClk;


#define PXA255_CLOCK_MANAGER_BASE	0x41300000UL
#define PXA255_CLOCK_MANAGER_SIZE	0x00001000UL

#define PXA255_POWER_MANAGER_BASE	0x40F00000UL
#define PXA255_POWER_MANAGER_SIZE	0x00001000UL


Boolean pxa255pwrClkInit(Pxa255pwrClk* pc, ArmCpu* cpu, ArmMem* physMem);





#endif
