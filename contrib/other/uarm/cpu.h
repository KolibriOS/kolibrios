#ifndef _CPU_H_
#define _CPU_H_


//#define ARM_V6		//define to allow v6 instructions
//#define THUMB_2			//define to allow Thumb2

#include "types.h"

struct ArmCpu;

#define ARM_SR_N		0x80000000UL
#define ARM_SR_Z		0x40000000UL
#define ARM_SR_C		0x20000000UL
#define ARM_SR_V		0x10000000UL
#define ARM_SR_Q		0x08000000UL
#ifdef ARM_V6	//V6KT2, but without T2 to be exact (we implement things like MLS, but not Thumb2 or ThumbEE)
	#define ARM_SR_J	0x01000000UL
	#define ARM_SR_E	0x00000200UL
	#define ARM_SR_A	0x00000100UL
	#define ARM_SR_GE_0	0x00010000UL
	#define ARM_SR_GE_1	0x00020000UL
	#define ARM_SR_GE_2	0x00040000UL
	#define ARM_SR_GE_3	0x00080000UL
	#define ARM_SR_GE_MASK	0x000F0000UL
	#define ARM_SR_GE_SHIFT	16
#endif
#define ARM_SR_I		0x00000080UL
#define ARM_SR_F		0x00000040UL
#define ARM_SR_T		0x00000020UL
#define ARM_SR_M		0x0000001FUL

#define ARM_SR_MODE_USR		0x00000010UL
#define ARM_SR_MODE_FIQ		0x00000011UL
#define ARM_SR_MODE_IRQ		0x00000012UL
#define ARM_SR_MODE_SVC		0x00000013UL
#define ARM_SR_MODE_ABT		0x00000017UL
#define ARM_SR_MODE_UND		0x0000001BUL
#define ARM_SR_MODE_SYS		0x0000001FUL

#define ARV_VECTOR_OFFT_RST	0x00000000UL
#define ARM_VECTOR_OFFT_UND	0x00000004UL
#define ARM_VECTOR_OFFT_SWI	0x00000008UL
#define ARM_VECTOR_OFFT_P_ABT	0x0000000CUL
#define ARM_VECTOR_OFFT_D_ABT	0x00000010UL
#define ARM_VECTOR_OFFT_UNUSED	0x00000014UL
#define ARM_VECTOR_OFFT_IRQ	0x00000018UL
#define ARM_VECTOR_OFFT_FIQ	0x0000001CUL

#define HYPERCALL_ARM		0xF7BBBBBBUL
#define HYPERCALL_THUMB		0xBBBBUL

//the following are for cpuGetRegExternal() and are generally used for debugging purposes
#define ARM_REG_NUM_CPSR	16
#define ARM_REG_NUM_SPSR	17

struct ArmCpu;

typedef Boolean	(*ArmCoprocRegXferF)	(struct ArmCpu* cpu, void* userData, Boolean two/* MCR2/MRC2 ? */, Boolean MRC, UInt8 op1, UInt8 Rx, UInt8 CRn, UInt8 CRm, UInt8 op2);
typedef Boolean	(*ArmCoprocDatProcF)	(struct ArmCpu* cpu, void* userData, Boolean two/* CDP2 ? */, UInt8 op1, UInt8 CRd, UInt8 CRn, UInt8 CRm, UInt8 op2);
typedef Boolean	(*ArmCoprocMemAccsF)	(struct ArmCpu* cpu, void* userData, Boolean two /* LDC2/STC2 ? */, Boolean N, Boolean store, UInt8 CRd, UInt32 addr, UInt8* option /* NULL if none */);
typedef Boolean (*ArmCoprocTwoRegF)	(struct ArmCpu* cpu, void* userData, Boolean MRRC, UInt8 op, UInt8 Rd, UInt8 Rn, UInt8 CRm);

typedef Boolean	(*ArmCpuMemF)		(struct ArmCpu* cpu, void* buf, UInt32 vaddr, UInt8 size, Boolean write, Boolean priviledged, UInt8* fsr);	//read/write
typedef Boolean	(*ArmCpuHypercall)	(struct ArmCpu* cpu);		//return true if handled
typedef void	(*ArmCpuEmulErr)	(struct ArmCpu* cpu, const char* err_str);

typedef void	(*ArmSetFaultAdrF)	(struct ArmCpu* cpu, UInt32 adr, UInt8 faultStatus);

#include "icache.h"


/*

	coprocessors:
				
				0    - DSP (pxa only)
				0, 1 - WMMX (pxa only)
				11   - VFP (arm standard)
				15   - system control (arm standard)
*/


typedef struct{
	
	ArmCoprocRegXferF regXfer;
	ArmCoprocDatProcF dataProcessing;
	ArmCoprocMemAccsF memAccess;
	ArmCoprocTwoRegF  twoRegF;
	void* userData;
	
}ArmCoprocessor;

typedef struct{

	UInt32 R13, R14;
	UInt32 SPSR;			//usr mode doesn't have an SPSR
}ArmBankedRegs;








typedef struct ArmCpu{

	UInt32		regs[16];		//current active regs as per current mode
	UInt32		CPSR, SPSR;

	ArmBankedRegs	bank_usr;		//usr regs when in another mode
	ArmBankedRegs	bank_svc;		//svc regs when in another mode
	ArmBankedRegs	bank_abt;		//abt regs when in another mode
	ArmBankedRegs	bank_und;		//und regs when in another mode
	ArmBankedRegs	bank_irq;		//irq regs when in another mode
	ArmBankedRegs	bank_fiq;		//fiq regs when in another mode
	UInt32		extra_regs[5];		//fiq regs when not in fiq mode, usr regs when in fiq mode. R8-12

	UInt16		waitingIrqs;
	UInt16		waitingFiqs;
	UInt16		CPAR;

	ArmCoprocessor	coproc[16];		//coprocessors

	// various other cpu config options
	UInt32		vectorBase;		//address of vector base

#ifdef ARM_V6

	Boolean		EEE;			//endianness one exception entry
	Boolean		impreciseAbtWaiting;
#endif

	ArmCpuMemF	memF;
	ArmCpuEmulErr	emulErrF;
	ArmCpuHypercall	hypercallF;
	ArmSetFaultAdrF	setFaultAdrF;
	
	icache		ic;

	void*		userData;		//shared by all callbacks
}ArmCpu;


Err cpuInit(ArmCpu* cpu, UInt32 pc, ArmCpuMemF memF, ArmCpuEmulErr emulErrF, ArmCpuHypercall hypercallF, ArmSetFaultAdrF setFaultAdrF);
Err cpuDeinit(ArmCpu* cp);
void cpuCycle(ArmCpu* cpu);
void cpuIrq(ArmCpu* cpu, Boolean fiq, Boolean raise);	//unraise when acknowledged

#ifdef ARM_V6

	void cpuSignalImpreciseAbt(ArmCpu* cpu, Boolean raise);
	
#endif

UInt32 cpuGetRegExternal(ArmCpu* cpu, UInt8 reg);
void cpuSetReg(ArmCpu* cpu, UInt8 reg, UInt32 val);

void cpuCoprocessorRegister(ArmCpu* cpu, UInt8 cpNum, ArmCoprocessor* coproc);
void cpuCoprocessorUnregister(ArmCpu* cpu, UInt8 cpNum);

void cpuSetVectorAddr(ArmCpu* cpu, UInt32 adr);

UInt16 cpuGetCPAR(ArmCpu* cpu);
void cpuSetCPAR(ArmCpu* cpu, UInt16 cpar);

void cpuIcacheInval(ArmCpu* cpu);
void cpuIcacheInvalAddr(ArmCpu* cpu, UInt32 addr);


#endif

