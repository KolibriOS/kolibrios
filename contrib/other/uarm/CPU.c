/*
	TODO:
	
		-verify DSP adds, subtracts, multiplies
		-MCR/MRC to R15 is special - handle it

	TOTOv6:
		-Endianness support (incl for pagetable lookups)

*/


#include "CPU.h"
#include "math64.h"



#define ARM_MODE_2_REG	0x0F
#define ARM_MODE_2_WORD	0x10
#define ARM_MODE_2_LOAD	0x20
#define ARM_MODE_2_T	0x40
#define ARM_MODE_2_INV	0x80

#define ARM_MODE_3_REG	0x0F	//flag for actual reg number used
#define ARM_MODE_3_TYPE	0x30	//flag for the below 4 types
#define ARM_MODE_3_H	0x00
#define ARM_MODE_3_SH	0x10
#define ARM_MODE_3_SB	0x20
#define ARM_MODE_3_D	0x30
#define ARM_MODE_3_LOAD	0x40
#define ARM_MODE_3_INV	0x80

#define ARM_MODE_4_REG	0x0F
#define ARM_MODE_4_INC	0x10	//incr or decr
#define ARM_MODE_4_BFR	0x20	//before or after
#define ARM_MODE_4_WBK	0x40	//writeback?
#define ARM_MODE_4_S	0x80	//S bit set?

#define ARM_MODE_5_REG		0x0F
#define ARM_MODE_5_ADD_BEFORE	0x10	//add value before?
#define ARM_MODE_5_ADD_AFTER	0x20	//add value after?
#define ARM_MODE_5_IS_OPTION	0x40	//is value option (as opposed to offset)
#define ARM_MODE_5_RR		0x80	//MCRR or MRCC instrs


#ifdef ARM_V6

	#define ARM_CPSR_UND_AND	(~(ARM_SR_M | ARM_SR_E))
	#define ARM_CPSR_UND_ORR	(ARM_SR_I | ARM_SR_MODE_UND | (cpu->EEE ? ARM_SR_E : 0))

	#define ARM_CPSR_SWI_AND	(~(ARM_SR_M | ARM_SR_E))
	#define ARM_CPSR_SWI_ORR	(ARM_SR_I | ARM_SR_MODE_SVC | (cpu->EEE ? ARM_SR_E : 0))

	#define ARM_CPSR_PAB_AND	(~(ARM_SR_M | ARM_SR_E))
	#define ARM_CPSR_PAB_ORR	(ARM_SR_I | ARM_SR_A | ARM_SR_MODE_ABT | (cpu->EEE ? ARM_SR_E : 0))

	#define ARM_CPSR_DAB_AND	(~(ARM_SR_M | ARM_SR_E))
	#define ARM_CPSR_DAB_ORR	(ARM_SR_I | ARM_SR_A | ARM_SR_MODE_ABT | (cpu->EEE ? ARM_SR_E : 0))

	#define ARM_CPSR_IRQ_AND	(~(ARM_SR_M | ARM_SR_E))
	#define ARM_CPSR_IRQ_ORR	(ARM_SR_I | ARM_SR_A | ARM_SR_MODE_IRQ | (cpu->EEE ? ARM_SR_E : 0))

	#define ARM_CPSR_FIQ_AND	(~(ARM_SR_M | ARM_SR_E))
	#define ARM_CPSR_FIQ_ORR	(ARM_SR_I | ARM_SR_A | ARM_SR_F | ARM_SR_MODE_IRQ | (cpu->EEE ? ARM_SR_E : 0))

	
#else
	
	#define ARM_CPSR_UND_AND	(~ARM_SR_M)
	#define ARM_CPSR_UND_ORR	(ARM_SR_I | ARM_SR_MODE_UND)

	#define ARM_CPSR_SWI_AND	(~ARM_SR_M)
	#define ARM_CPSR_SWI_ORR	(ARM_SR_I | ARM_SR_MODE_SVC)

	#define ARM_CPSR_PAB_AND	(~ARM_SR_M)
	#define ARM_CPSR_PAB_ORR	(ARM_SR_I | ARM_SR_MODE_ABT)

	#define ARM_CPSR_DAB_AND	(~ARM_SR_M)
	#define ARM_CPSR_DAB_ORR	(ARM_SR_I | ARM_SR_MODE_ABT)

	#define ARM_CPSR_IRQ_AND	(~ARM_SR_M)
	#define ARM_CPSR_IRQ_ORR	(ARM_SR_I | ARM_SR_MODE_IRQ)

	#define ARM_CPSR_FIQ_AND	(~ARM_SR_M)
	#define ARM_CPSR_FIQ_ORR	(ARM_SR_I | ARM_SR_F | ARM_SR_MODE_FIQ)

	
#endif



static _INLINE_ UInt32 cpuPrvROR(UInt32 val, UInt8 ror){

	if(ror) val = (val >> (UInt32)ror) | (val << (UInt32)(32 - ror));
	
	return val;
}

static _INLINE_ void cpuPrvSetPC(ArmCpu* cpu, UInt32 pc){
	cpu->regs[15] = pc &~ 1UL;
	cpu->CPSR &=~ ARM_SR_T;
	if(pc & 1) cpu->CPSR |= ARM_SR_T;
	else if(pc & 2) cpu->emulErrF(cpu, "Attempt to branch to non-word-aligned ARM address");
}

static _INLINE_ UInt32 cpuPrvGetReg(ArmCpu* cpu, UInt8 reg, Boolean wasT, Boolean specialPC){

	UInt32 ret;

	ret = cpu->regs[reg];
	if(reg == 15) ret += (wasT) ? 2 : 4;
	if(wasT && specialPC) ret &=~ 3UL;

	return ret;
}

static _INLINE_ void cpuPrvSetReg(ArmCpu* cpu, UInt8 reg, UInt32 val){

	if(reg == 15){
		cpuPrvSetPC(cpu, val);
	}
	else cpu->regs[reg] = val;
}

UInt32 cpuGetRegExternal(ArmCpu* cpu, UInt8 reg){

	if(reg < 16){	// real reg
	
		return (reg == 15) ? (cpu->regs[15] + ((cpu->CPSR & ARM_SR_T) ? 1 : 0)) : cpu->regs[reg];
	}
	else if(reg == ARM_REG_NUM_CPSR){
	
		return cpu->CPSR;
	}
	else if(reg == ARM_REG_NUM_SPSR){
	
		return cpu->SPSR;	
	}
	
	return 0;
}

void cpuSetReg(ArmCpu* cpu, UInt8 reg, UInt32 val){

	cpuPrvSetReg(cpu, reg, val);
}

#define cpuSetReg	_DO_NOT_USE_cpuSetReg_IN_CPU_C_

static ArmBankedRegs* cpuPrvModeToBankedRegsPtr(ArmCpu* cpu, UInt8 mode){

	switch(mode){
		case ARM_SR_MODE_USR:
		case ARM_SR_MODE_SYS:
			return &cpu->bank_usr;
		
		case ARM_SR_MODE_FIQ:
			return &cpu->bank_fiq;
		
		case ARM_SR_MODE_IRQ:
			return &cpu->bank_irq;
		
		case ARM_SR_MODE_SVC:
			return &cpu->bank_svc;
		
		case ARM_SR_MODE_ABT:
			return &cpu->bank_abt;
			
		case ARM_SR_MODE_UND:
			return &cpu->bank_und;
		
		default:
			cpu->emulErrF(cpu, "Invalid mode passed to cpuPrvModeToBankedRegsPtr()");
			return NULL;
	}
}

static void cpuPrvSwitchToMode(ArmCpu* cpu, UInt8 newMode){

	ArmBankedRegs *saveTo, *getFrom;
	UInt32 tmp;
	UInt8 i, curMode;
	
	curMode = cpu->CPSR & ARM_SR_M;
	if(curMode == newMode) return;
	
	if(curMode == ARM_SR_MODE_FIQ || newMode == ARM_SR_MODE_FIQ){	//bank/unbank the fiq regs
		
		for(i = 0; i < 5; i++){
			tmp = cpu->extra_regs[i];
			cpu->extra_regs[i] = cpu->regs[i + 8];
			cpu->regs[i + 8] = tmp;
		}
	}
	
	saveTo = cpuPrvModeToBankedRegsPtr(cpu, curMode);
	getFrom = cpuPrvModeToBankedRegsPtr(cpu, newMode);
	
	if(saveTo == getFrom) return;	//we're done if no regs to switch [this happens if we switch user<->system]
	
	saveTo->R13 = cpu->regs[13];
	saveTo->R14 = cpu->regs[14];
	saveTo->SPSR = cpu->SPSR;
	
	cpu->regs[13] = getFrom->R13;
	cpu->regs[14] = getFrom->R14;
	cpu->SPSR = getFrom->SPSR;
	
	cpu->CPSR = (cpu->CPSR &~ ARM_SR_M) | newMode;
}

static void cpuPrvException(ArmCpu* cpu, UInt32 vector_pc, UInt32 lr, UInt32 newCPSR){

	UInt32 cpsr = cpu->CPSR;
	
	cpuPrvSwitchToMode(cpu, newCPSR & ARM_SR_M);
	cpu->CPSR = newCPSR;
	cpu->SPSR = cpsr;
	cpu->regs[14] = lr;
	cpu->regs[15] = vector_pc;
}

static void cpuPrvHandleMemErr(ArmCpu* cpu, UInt32 addr, _UNUSED_ UInt8 sz, _UNUSED_ Boolean write, Boolean instrFetch, UInt8 fsr){

	if(cpu->setFaultAdrF) cpu->setFaultAdrF(cpu, addr, fsr);

	if(instrFetch){
		
		//handle prefetch abort
		cpuPrvException(cpu, cpu->vectorBase + ARM_VECTOR_OFFT_P_ABT, cpu->regs[15] + 4, ARM_CPSR_PAB_ORR | (cpu->CPSR & ARM_CPSR_PAB_AND));
	}
	else{
		//handle data abort
		cpuPrvException(cpu, cpu->vectorBase + ARM_VECTOR_OFFT_D_ABT, cpu->regs[15] + ((cpu->CPSR & ARM_SR_T) ? 6 : 4) /* instr + 8*/, ARM_CPSR_DAB_ORR | (cpu->CPSR & ARM_CPSR_DAB_AND));
	}
}

static _INLINE_ UInt32 cpuPrvArmAdrMode_1(ArmCpu* cpu, UInt32 instr, Boolean* carryOutP, Boolean wasT, Boolean specialPC){
	
	UInt32 ret;
	UInt8 v, a;
	Boolean co = ((cpu->CPSR & ARM_SR_C) != 0);	//be default carry out = C flag

	if(instr & 0x02000000UL){				//immed

		v = (instr >> 7) & 0x1E;
		ret = cpuPrvROR(instr & 0xFF, v);
		if (v) co = ((ret & 0x80000000UL) != 0);
	}
	else{
		v = (instr >> 5) & 3;			//get shift type
		ret = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);	//get Rm
	
		if(instr & 0x00000010UL){			//reg with reg shift
	
			a = cpuPrvGetReg(cpu, (instr >> 8) & 0x0F, wasT, specialPC);	//get the relevant part of Rs, we only care for lower 8 bits (note we use uint8 for this)
			
			if(a != 0){	//else all is already good
			
				switch(v){			//perform shifts
					
					case 0:			//LSL
					
						if(a < 32){
							co = (ret >> (32 - a)) & 1;
							ret = ret << a;
						}
						else if(a == 32){
							co = ret & 1;
							ret = 0;
						}
						else{	// >32
							co = 0;
							ret = 0;
						}
						break;
					
					case 1:			//LSR
					
						if(a < 32){
							co = (ret >> (a - 1)) & 1;
							ret = ret >> a;
						}
						else if(a == 32){
							co = ret >> 31;
							ret = 0;
						}
						else{	// >32
							co = 0;
							ret = 0;
						}
						break;
						
					case 2:			//ASR
					
						if(a < 32){
							co = (ret >> (a - 1)) & 1;
							ret = ((Int32)ret >> a);
						}
						else{	// >=32
							if(ret & 0x80000000UL){
								co = 1;
								ret = 0xFFFFFFFFUL;
							}
							else{
								co = 0;
								ret = 0;
							}
						}
						break;
						
					case 3:			//ROR
					
						if(a == 0){
							
							//nothing...
						}
						else{
							a &= 0x1F;
							if(a == 0){
								co = ret >> 31;
							}
							else{
								co = (ret >> (a - 1)) & 1;
								ret = cpuPrvROR(ret, a);
							}
						}
						break;
				}
			}
		}
		else{					//reg with immed shift
	
			a = (instr >> 7) & 0x1F;	//get imm
			
			switch(v){
				
				case 0:			//LSL
				
					if(a == 0){
						//nothing
					}
					else{
						co = (ret >> (32 - a)) & 1;
						ret = ret << a;
					}
					break;
				
				case 1:			//LSR
				
					if(a == 0){
						co = ret >> 31;
						ret = 0;
					}
					else{
						co = (ret >> (a - 1)) & 1;
						ret = ret >> a;
					}
					break;
				
				case 2:			//ASR

					if(a == 0){
						if(ret & 0x80000000UL){
							co = 1;
							ret = 0xFFFFFFFFUL;
						}
						else{
							co = 0;
							ret = 0;
						}
					}
					else{
						co = (ret >> (a - 1)) & 1;
						if(ret & 0x80000000UL){
							ret = (ret >> a) | (0xFFFFFFFFUL << (32 - a));
						}
						else{
							ret = ret >> a;
						}
					}
					break;
				
				case 3:			//ROR or RRX
				
					if(a == 0){	//RRX
						a = co;
						co = ret & 1;
						ret = ret >> 1;
						if(a) ret |= 0x80000000UL;
					}
					else{
						co = (ret >> (a - 1)) & 1;
						ret = cpuPrvROR(ret, a);
					}
					break;
			}
		}
	}

	*carryOutP = co;
	return ret;
}

/*
idea:

addbefore is what to add to add to base reg before addressing, addafter is what to add after. we ALWAYS do writeback, but if not requested by instr, it will be zero

for [Rx, 5]   baseReg = x addbefore = 5 addafter = -5
for [Rx, 5]!  baseReg = x addBefore = 0 addafter = 0
for [Rx], 5   baseReg = x addBefore = 0 addAfter = 5

t = T bit (LDR vs LDRT)

baseReg is returned in return val along with flags:


ARM_MODE_2_REG	is mask for reg
ARM_MODE_2_WORD	is flag for word access
ARM_MODE_2_LOAD	is flag for load
ARM_MODE_2_INV	is flag for invalid instructions
ARM_MODE_2_T	is flag for T


*/
static _INLINE_ UInt8 cpuPrvArmAdrMode_2(ArmCpu* cpu, UInt32 instr, UInt32* addBeforeP, UInt32* addWritebackP, Boolean wasT, Boolean specialPC){

	UInt32 val;
	UInt8 reg, shift;

	reg = (instr >> 16) & 0x0F;

	if(!(instr & 0x02000000UL)){	//immediate
		val = instr & 0xFFFUL;
	}
	else{			//[scaled] register
	
		if(instr & 0x00000010UL) reg |= ARM_MODE_2_INV;	//invalid instrucitons need to be reported
		
		val = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);
		shift = (instr >> 7) & 0x1F;
		switch((instr >> 5) & 3){
			
			case 0:		//LSL
				val <<= shift;
				break;
			
			case 1:		//LSR
				val = shift ? (val >> shift) : 0;
				break;
			
			case 2:		//ASR
				val = shift ? (UInt32)((((Int32)val) >> shift)) : ((val & 0x80000000UL) ? 0xFFFFFFFFUL : 0x00000000UL);
				break;
			
			case 3:		//ROR/RRX
				if(shift){
					val = cpuPrvROR(val, shift);
				}
				else{	//RRX
					val = val >> 1;
					if(cpu->CPSR & ARM_SR_C) val |= 0x80000000UL;
				}
		}
			
	}

	if(!(instr & 0x00400000UL)) reg |= ARM_MODE_2_WORD;
	if(instr & 0x00100000UL) reg |= ARM_MODE_2_LOAD;
	if(!(instr & 0x00800000UL)) val = -val;
	if(!(instr & 0x01000000UL)){
		*addBeforeP = 0;
		*addWritebackP = val;
		
		if(instr & 0x00200000UL) reg |= ARM_MODE_2_T;
	}
	else if(instr & 0x00200000UL){
		*addBeforeP = val;
		*addWritebackP = val;
	}
	else{
		*addBeforeP = val;
		*addWritebackP = 0;
	}
	
	return reg;
}


/*
same comments as for addr mode 2 apply

#define ARM_MODE_3_REG	0x0F	//flag for actual reg number used
#define ARM_MODE_3_TYPE	0x30	//flag for the below 4 types
#define ARM_MODE_3_H	0x00
#define ARM_MODE_3_SH	0x10
#define ARM_MODE_3_SB	0x20
#define ARM_MODE_3_D	0x30
#define ARM_MODE_3_LOAD	0x40
#define ARM_MODE_3_INV	0x80
*/

static _INLINE_ UInt8 cpuPrvArmAdrMode_3(ArmCpu* cpu, UInt32 instr, UInt32* addBeforeP, UInt32* addWritebackP, Boolean wasT, Boolean specialPC){

	UInt32 val;
	UInt8 reg;
	Boolean S, H, L;

	reg = (instr >> 16) & 0x0F;
	
	if(instr & 0x00400000UL){	//immediate
		val = ((instr >> 4) & 0xF0) | (instr & 0x0F);
	}
	else{
		if(instr & 0x00000F00UL) reg |= ARM_MODE_3_INV;	 //bits 8-11 must be 1 always
		val = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);
	}
	
	
	L = (instr & 0x00100000UL) != 0;
	H = (instr & 0x00000020UL) != 0;
	S = (instr & 0x00000040UL) != 0;
	
	if(S && H){
		reg |= ARM_MODE_3_SH;
	}
	else if(S){
		reg |= ARM_MODE_3_SB;
	}
	else if(H){
		reg |= ARM_MODE_3_H;
	}
	else{
		reg |= ARM_MODE_3_INV;	//S == 0 && H == 0 is invalid mode 3 operation
	}
	
	if((instr & 0x00000090UL) != 0x00000090UL) reg |= ARM_MODE_3_INV;	 //bits 4 and 7 must be 1 always
	
	if(S && !L){		//LDRD/STRD is encoded thusly
		
		reg = (reg &~ ARM_MODE_3_TYPE) | ARM_MODE_3_D;
		L = !H;
	}
	if(L) reg |= ARM_MODE_3_LOAD;
	if(!(instr & 0x00800000UL)) val = -val;
	if(!(instr & 0x01000000UL)){
		*addBeforeP = 0;
		*addWritebackP = val;
		
		if(instr & 0x00200000UL) reg |= ARM_MODE_3_INV;	//W must be 0 in this case, else unpredictable (in this case - invalid instr)
	}
	else if(instr & 0x00200000UL){
		*addBeforeP = val;
		*addWritebackP = val;
	}
	else{
		*addBeforeP = val;
		*addWritebackP = 0;
	}
	
	return reg;
}

/*
	#define ARM_MODE_4_REG	0x0F
	#define ARM_MODE_4_INC	0x10	//incr or decr
	#define ARM_MODE_4_BFR	0x20	//after or before
	#define ARM_MODE_4_WBK	0x40	//writeback?
	#define ARM_MODE_4_S	0x80	//S bit set?
*/

static UInt8 cpuPrvArmAdrMode_4(_UNUSED_ ArmCpu* cpu, UInt32 instr, UInt16* regs){

	UInt8 reg;
	
	
	*regs = instr;
	
	reg = (instr >> 16) & 0x0F;
	if(instr & 0x00400000UL) reg |= ARM_MODE_4_S;
	if(instr & 0x00200000UL) reg |= ARM_MODE_4_WBK;
	if(instr & 0x00800000UL) reg |= ARM_MODE_4_INC;
	if(instr & 0x01000000UL) reg |= ARM_MODE_4_BFR;
	
	return reg;
}

/*
#define ARM_MODE_5_REG		0x0F
#define ARM_MODE_5_ADD_BEFORE	0x10	//add value before?
#define ARM_MODE_5_ADD_AFTER	0x20	//add value after?
#define ARM_MODE_5_IS_OPTION	0x40	//is value option (as opposed to offset)
#define ARM_MODE_5_RR		0x80

*/
static _INLINE_ UInt8 cpuPrvArmAdrMode_5(_UNUSED_ ArmCpu* cpu, UInt32 instr, UInt32* valP){

	UInt32 val;
	UInt8 reg;
	
	
	val = instr & 0xFF;
	reg = (instr >> 16) & 0x0F;
	
	if(!(instr & 0x01000000UL)){	//unindexed or postindexed
		
		if(instr & 0x00200000UL){	//postindexed
		
			reg |= 	ARM_MODE_5_ADD_AFTER;
		}
		else{			//unindexed
			
			if(!(instr & 0x00800000UL)) reg |= ARM_MODE_5_RR;	//U must be 1 for unindexed, else it is MCRR/MRCC
			reg |= ARM_MODE_5_IS_OPTION;
		}
	}
	else{			//offset or preindexed
		
		reg |= ARM_MODE_5_ADD_BEFORE;
		
		if(instr & 0x00200000UL){	//preindexed
			
			reg |= ARM_MODE_5_ADD_AFTER;
		}
	}
	
	if(!(reg & ARM_MODE_5_IS_OPTION)){
		
		val = val << 2;
		if(!(instr & 0x00800000UL)) val = -val;	
	}
	
	*valP = val;
	return reg;
}

static _INLINE_ void cpuPrvSetPSR(ArmCpu* cpu, UInt8 mask, Boolean privileged, Boolean R, UInt32 val){
							
	if(R){	//setting SPSR in sys or usr mode is no harm since they arent used, so just do it without any checks
		
		cpu->SPSR = val;
	}
	else{
		
		UInt32 newCPSR = cpu->CPSR;
		
		if(privileged){
			if(mask & 1){
				
				cpuPrvSwitchToMode(cpu, val & ARM_SR_M);
				newCPSR = (newCPSR & 0xFFFFFF00UL) | (val & 0x000000FFUL);
			}
			if(mask & 2) newCPSR = (newCPSR & 0xFFFF00FFUL) | (val & 0x0000FF00UL);
			if(mask & 4) newCPSR = (newCPSR & 0xFF00FFFFUL) | (val & 0x00FF0000UL);
		}
		if(mask & 8) newCPSR = (newCPSR & 0x00FFFFFFUL) | (val & 0xFF000000UL);
		
		cpu->CPSR = newCPSR;
	}
}

static _INLINE_ Boolean cpuPrvSignedAdditionOverflows(UInt32 a, UInt32 b, UInt32 sum){
	
	return ((a ^ b ^ 0x80000000UL) & (a ^ sum)) >> 31;
}

static _INLINE_ Boolean cpuPrvSignedSubtractionOverflows(UInt32 a, UInt32 b, UInt32 diff){	//diff = a - b
	
	return ((a ^ b) & (a ^ diff)) >> 31;
}

static _INLINE_ UInt32 cpuPrvMedia_signedSaturate32(UInt32 sign){
	
	return (sign & 0x80000000UL) ? 0xFFFFFFFFUL : 0;
}

#ifdef ARM_V6
	
	static _INLINE_ UInt32 cpuPrvMedia_sxtb(UInt8 v){
		
		UInt32 r = v;
		if(v & 0x80) r |= 0xFFFFFF00UL;
		
		return r;
	}
	
	static _INLINE_ UInt32 cpuPrvMedia_uxtb(UInt8 v){
		
		UInt32 r = v;
		
		return r;
	}
	
	static _INLINE_ UInt32 cpuPrvMedia_sxth(UInt16 v){
		
		UInt32 r = v;
		if(v & 0x8000UL) r |= 0xFFFF0000UL;
		
		return r;
	}
	
	static _INLINE_ UInt32 cpuPrvMedia_uxth(UInt16 v){
		
		UInt32 r = v;
		
		return r;
	}
	
	static _INLINE_ UInt32 cpuPrvMedia_sxt16(UInt32 v){
		
		if(v & 0x00800000UL){
			v |= 0xFF000000UL;
		}
		else{
			v &=~ 0xFF000000UL;
		}
		
		if(v & 0x00000080UL){
			v |= 0x0000FF00UL;
		}
		else{
			v &=~ 0x0000FF00UL;
		}
		return v;
	}
	
	static _INLINE_ UInt32 cpuPrvMedia_uxt16(UInt32 v){
		
		v &=~ 0xFF00FF00UL;
		
		return v;
	}
	
	static _INLINE_ UInt32 cpuPrvMedia_dualAdd(UInt32 a, UInt32 b){		//3 ands, 2 ads, 1 sub
		
		UInt32 sum = a + b;
		sum -= (((a & 0xFFFFUL) + (b & 0xFFFFUL)) & 0xFFFF0000UL);		//think about it...it should work...
		
		return sum;
	}
	
	static _INLINE_ UInt32 cpuPrvMedia_SignedSat(UInt32 val, UInt8 from_bits, UInt8 to_bits){	//returns value. [orred with 0x80000000UL to indicate sat was done]
		
		UInt32 check_mask, top_in;
		
		if(from_bits <= to_bits) return val;	//no saturation needed
		
		top_in = (1UL << (from_bits - 1));
		check_mask = (1UL << to_bits) - (1UL << from_bits);
		
		if(val & top_in){	//input was negative
			
			if((val & check_mask) != check_mask){	//saturate
				
				val = top_in | 0x80000000UL;
			}
		}
		else{
			
			if(val & check_mask){			//saturate
				
				val = (top_in - 1) | 0x80000000UL;
			}
		}
		
		
		return val;
	}
	
	static _INLINE_ Boolean cpuPrvMediaInstrs(ArmCpu* cpu, UInt32 instr, Boolean wasT, Boolean specialPC){
		
		UInt64 v64;
		UInt32 v32a, v32b, v32c, v32d;
		Boolean vB = false;
		UInt8 v8a, v8b;
		UInt16 v16;
		
		switch((instr >> 20) & 0x1F){
			
			case 0:		//parrallel addition/subtraction signed
			case 1:
			case 2:
			case 3:
			
				//TODO
				break;
			
			case 4:		//parrallel addition/subtraction unsigned
			case 5:
			case 6:
			case 7:
				
				//TODO
				break;
			
			case 8:		//packing/unpacking/saturation/reversal
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			
				v8a = (instr >> 16) & 0x0F;
				v8b = (instr >> 5) & 7;
				switch((instr >> 20) & 7){
					
					case 0:	switch(v8b){
						
							case 0:		//PKH
							case 2:
							case 4:
							case 6:
								
								v32a = cpuPrvGetReg(cpu, (instr >> 16) & 0x0F, wasT, specialPC);		//Rn
								v32b = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);			//Rm
								v8a = (instr >> 7) & 0x1F;
								if(instr & 0x00000040UL){	//ASR
									
									if(!v8a) v8a = 32;
									v32b = (((Int32)v32b) >> v8a);
									
									v32c = (v32b & 0xFFFFUL) | (v32a & 0xFFFF0000UL);
								}
								else{			//LSL
									
									v32b <<= v8a;
									v32c = (v32a & 0xFFFFUL) | (v32b & 0xFFFF0000UL);
								}
								cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, v32c);
								break;
							
							case 3:		//SXTAB16 SXTB16
								
								vB = true;		//sign extend
								goto xt16;
							
							case 5:		//SEL
								
								v32a = cpuPrvGetReg(cpu, (instr >> 16) & 0x0F, wasT, specialPC);		//Rn
								v32b = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);			//Rm
								
								v32c = 0;
								if(cpu->CPSR & ARM_SR_GE_0) v32c |= 0x000000FFUL;
								if(cpu->CPSR & ARM_SR_GE_1) v32c |= 0x0000FF00UL;
								if(cpu->CPSR & ARM_SR_GE_2) v32c |= 0x00FF0000UL;
								if(cpu->CPSR & ARM_SR_GE_2) v32c |= 0xFF000000UL;
								v32c = (v32a & v32c) | (v32b & ~v32c);
								
								cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, v32c);
								break;
							
							default:
								
								return false;
						}
						break;
					
					case 2:	switch(v8b){
							
							case 1:		//SSAT16
								
								//TODO;
								break;
							
							case 3:		//SXTAB SXTB
								
								vB = true;	//sign extend
								goto xtb;
							
							case 0:
							case 2:
							case 4:
							case 6:
								
								goto ssat;
								break;
							
							default:
								
								return false;
						}
						break;
					
					case 3: switch(v8b){
							
							case 1:		//REV
								
								//TODO;
								break;
							
							case 3:		//SXTAH SXTH
								
								vB = true;	//sign extend
								goto xth;
							
							case 5:		//REV16
								
								//TOOD;
								break;
							
							case 0:		//SSAT
							case 2:
							case 4:
							case 6:
				ssat:
								//TODO
								break;
							
							default:
								
								return false;
						}
						break;
					
					case 4:				//UXTAB16 UXTB16
						if(v8b != 3) return false;
						
						vB = false;		//not sign extend
				xt16:
						v32a = (v8a == 15) ? 0 : cpuPrvGetReg(cpu, v8a, wasT, specialPC);	//Rn
						v32b = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);		//Rm
						v32b = cpuPrvROR(v32b, (instr >> 7) & 0x18);
						v32b = vB ? cpuPrvMedia_sxt16(v32b) : cpuPrvMedia_uxt16(v32b);
						
						v32c = cpuPrvMedia_dualAdd(v32a, v32b);
						cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, v32a);
						break;
					
					case 6: switch(v8b){
							
							case 1:		//USAT16
								
								//TODO;
								break;
							
							case 3:		//UXTAB UXTB
								vB = false;	//not sign extend
				xtb:
								v32a = (v8a == 15) ? 0 : cpuPrvGetReg(cpu, v8a, wasT, specialPC);	//Rn
								v32b = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC)			//Rm
								v32b = cpuPrvROR(v32b, (instr >> 7) & 0x18);
								v32b = vB ? cpuPrvMedia_sxtb(v32b) : cpuPrvMedia_uxtb(v32b);
								
								v32a += v32b;
								cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, v32a);
								break;
							
							case 0:
							case 2:
							case 4:
							case 6:
								
								goto usat;
							
							default:
								
								return false;
						}
						break;
					
					case 7: switch(v8b){
							
							case 1:		//RBIT
								
								v32a = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);
								v32b = 0;
								v32c = 0x80000000UL;
								v32d = 0x00000001UL;
								
								//faster ways exist, but this is smaller code
								for(v8a = 0; v8a < 32; v8a++, v32c >>= 1, v32d <<= 1) if(v32a & v32c) v32b |= v32d;
								
								cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, v32b);
								break;
							
							case 3:		//UXTAH UXTH
								
								vB = false;	//not sign extend
				xth:
								v32a = (v8a == 15) ? 0 : cpuPrvGetReg(cpu, v8a, wasT, specialPC);	//Rn
								v32b = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC)			//Rm
								v32b = cpuPrvROR(v32b, (instr >> 7) & 0x18);
								v32b = vB ? cpuPrvMedia_sxth(v32b) : cpuPrvMedia_uxth(v32b);
								
								v32a += v32b;
								cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, v32a);
								break;
							
							case 5:		//REVSH
								
								//TODO;
								break;
							
							case 0:		//USAT
							case 2:
							case 4:
							case 6:
				usat:
								//TODO;
								break;
							
							default:
								
								return false;
						}
						break;
					
					default:
						return false;
				}
				break;
			
			case 16:	//signed multiplies
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
			case 22:
			case 23:
				
				v8a = (instr >> 12) & 0x0F;
				if((instr & 0x00700000UL) == 0x00000000){			//SMLAD, SMUAD, SMLSD, SMUSD
					
					v32a = cpuPrvGetReg(cpu, (instr >> 8) & 0x0F, wasT, specialPC);	//Rm
					v32b = cpuPrvGetReg(cpu, (instr >> 0) & 0x0F, wasT, specialPC);	//Rn
					
					if(instr & 0x00000020UL) v32a = cpuPrvROR(v32a, 16);
					v32c = (Int32)((Int16)(v32a & 0xFFFFUL)) * (Int32)((Int16)(v32b & 0xFFFFUL));
					v32a = (Int32)((Int16)(v32a >> 16)) * (Int32)((Int16)(v32b >> 16));
					v32b = (v8a == 15) ? 0 : cpuPrvGetReg(cpu, v8a, wasT, specialPC);
					
					if((instr & 0x000000C0UL) == 0x00000000){		//SMLAD, SMUAD
						
						//now add them and mark Q flag is we overflow
						v32d = v32a + v32c;
						vB = cpuPrvSignedAdditionOverflows(v32a, v32c, v32d);
					}
					else if((instr & 0x000000C0UL) == 0x00000040UL){	//SMLSD, SMUSD
						
						v32d = v32a - v32c;
						vB = cpuPrvSignedSubtractionOverflows(v32a, v32c, v32d);
					}
					else return false;
					
					v32a = v32d + v32b;
					vB = vB || cpuPrvSignedAdditionOverflows(v32d, v32b, v32a);
					cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, v32a);
				}
				else if((instr & 0x00700000UL) == 0x00400000UL){	//SMLALD, SMLSLD
					
					v64 = u64_from_halves(cpuPrvGetReg(cpu, (instr >> 16) & 0x0F, wasT, specialPC), cpuPrvGetReg(cpu, (instr >> 12) & 0x0F, wasT, specialPC));
					v32a = cpuPrvGetReg(cpu, (instr >> 8) & 0x0F, wasT, specialPC);	//Rm
					v32b = cpuPrvGetReg(cpu, (instr >> 0) & 0x0F, wasT, specialPC);	//Rn
					
					if(instr & 0x00000020UL) v32a = cpuPrvROR(v32a, 16);
					v32c = (Int32)((Int16)(v32a & 0xFFFFUL)) * (Int32)((Int16)(v32b & 0xFFFFUL));
					v32a = (Int32)((Int16)(v32a >> 16)) * (Int32)((Int16)(v32b >> 16));
					
					if((instr & 0x000000C0UL) == 0x00000000){		//SMLALD
						
						v64 = u64_add(v64, u64_add(u64_xtnd32(u64_32_to_64(v32a)), u64_xtnd32(u64_32_to_64(v32c))));
					}
					else if((instr & 0x000000C0UL) == 0x00000040){	//SMLSLD
						
						v64 = u64_add(v64, u64_sub(u64_xtnd32(u64_32_to_64(v32a)), u64_xtnd32(u64_32_to_64(v32c))));
					}
					else return false;
					
					cpuPrvSetReg(cpu, (instr >> 16) & 0x0F, u64_get_hi(v64));
					cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, u64_64_to_32(v64));
				}
				else if((instr & 0x00700000UL) == 0x00500000UL){		//SMMLA, SMMUL, SMMLS
					
					v32a = cpuPrvGetReg(cpu, (instr >> 8) & 0x0F, wasT, specialPC);	//Rm
					v32b = cpuPrvGetReg(cpu, (instr >> 0) & 0x0F, wasT, specialPC);	//Rn
					v32c = cpuPrvGetReg(cpu, (instr >> 12) & 0x0F, wasT, specialPC);	//Ra
					
					v64 = u64_smul3232(v32a, v32b);
					
					if((instr & 0x000000C0UL) == 0x00000000){		//SMMLA, SMMUL
						
						if(v8a == 15){	//SMMUL
							
							//nothing to do here
						}
						else{		//SMMLA
							
							v64 = u64_add(v64, u64_shl(u64_32_to_64(v32c), 32));
						}
					}
					else if((instr & 0x000000C0UL) == 0x000000C0){	//SMMLS
						
						v64 = u64_sub(v64, u64_shl(u64_32_to_64(v32c), 32));
					}
					else return false;
					
					if(instr & 0x00000010UL) v64 = u64_add(v64, u64_32_to_64(0x80000000UL));	//round if requested
					
					cpuPrvSetReg(cpu, (instr >> 16) & 0x0F, u64_get_hi(v64));
				}
				else return false;
				
				if(vB){	//overflow
					
					cpu->CPSR |= ARM_SR_Q;	
				}
				break;
			
			case 24:					//USAD8 USADA8
				
				if(instr & 0x000000E0UL) return false;
				
				v32a = ((instr & 0x0000F000UL) == 0x0000F000UL) ? 0 : cpuPrvGetReg(cpu, (instr >> 12) & 0x0F, wasT, specialPC);	//Ra
				v32b = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);							//Rn
				v32c = cpuPrvGetReg(cpu, (instr >> 8) & 0x0F, wasT, specialPC);							//Rm
				
				for(v8a = 0; v8a < 4; v8a++){
					v16 = (v32b & 0xFF) - (v32c & 0xFF);
					if(v16 & 0x8000U) v16 = -v16;
					v32a += v16;
					v32b >>= 8;
					v32c >>= 8;
				}
				cpuPrvSetReg(cpu, (instr >> 16) & 0x0F, v32a);
				break;
			
			case 26:
			case 27:
				
				vB = true;	//sign extend
				goto bitfield_extract;
			
			case 30:
			case 31:
				
				vB = false;	//do not sign extend
		bitfield_extract:
				
				if((instr & 0x00000060UL) == 0x00000040){	//unsigned bitfield extract
					
					v32a = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);
					v8a = ((instr >> 16) & 0x1F) + 1;
					v8b = (instr >> 7) & 0x1F;
					v32a >>= v8b;
					v32c = 0xFFFFFFFFUL << v8a;
					v32a &=~ v32c;
					if(vB){	//sign extend
						if(v32a & (1UL << (v8a - 1))) v32a |= v32c;
					}
					cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, v32a);
				}
				else return false;
				break;
			
			case 28:
			case 29:
				if(instr & 0x00000060UL){
					
					return false;
				}
				else{		//BFC BFI
				
					v8a = 31 - ((instr >> 16) & 0x1F);
					v8b = (instr >> 7) & 0x1F;
					v32a = 0xFFFFFFFFUL;
					v32a >>= v8b;
					v32a <<= v8b + v8a;
					v32a >>= v8a;
					
					v32b = cpuPrvGetReg(cpu, (instr >> 12) & 0x0F, wasT, specialPC);
					v32b &= ~v32a;
					
					if((instr & 0x0000000FUL) == 0x0000000F){	//BFC
						
						//bits already clear
					}
					else{					//BFI
						
						v32c = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);
						v32c >>= v8b;
						v32c <<= v8b + v8a;
						v32c >>= v8a;
						v32b |= v32c;
					}
					cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, v32b);
				}
				break;
			
			default:
				
				return false;
		}
		return true;
	}
#endif

static Err cpuPrvExecInstr(ArmCpu* cpu, UInt32 instr, UInt32 instrPC/* lower bit always clear */, Boolean wasT , Boolean privileged, Boolean specialPC/* for thumb*/){
	
	Boolean specialInstr = false, usesUsrRegs, execute = false, L, ok;
	UInt8 fsr;
	
	usesUsrRegs = ((cpu->CPSR & ARM_SR_M) == ARM_SR_MODE_USR) || ((cpu->CPSR & ARM_SR_M) == ARM_SR_MODE_SYS);

	//check condition code
	{
		switch(instr >> 29UL){

			case 0:		//EQ / NE
				execute = (cpu->CPSR & ARM_SR_Z) != 0;
				break;

			case 1:		//CS / CC
				execute = (cpu->CPSR & ARM_SR_C) != 0;
				break;

			case 2:		//MI/PL
				execute = (cpu->CPSR & ARM_SR_N) != 0;
				break;

			case 3:		//VS/VC
				execute = (cpu->CPSR & ARM_SR_V) != 0;
				break;

			case 4:		//HI/LS
				execute = (cpu->CPSR & ARM_SR_C) && !(cpu->CPSR & ARM_SR_Z);
				break;

			case 5:		//GE/LT
				execute = !(cpu->CPSR & ARM_SR_N) == !(cpu->CPSR & ARM_SR_V);	//check for N == V
				break;

			case 6:		//GT/LE
				execute = !(cpu->CPSR & ARM_SR_N) == !(cpu->CPSR & ARM_SR_V);	//check for N == V
				execute = execute && !(cpu->CPSR & ARM_SR_Z);			//enforce Z==0
				break;

			case 7:
				specialInstr = (instr & 0x10000000UL) != 0;
				execute = true;
				break;
		}
		if((instr & 0x10000000UL) && !specialInstr) execute = !execute;	//invert for inverted conditions
	}

	//execute, if needed
	if(execute){
		UInt64 v64;
		register UInt32 adr, tmp, v32;
		UInt32 m32, x32;	//non-register 32-bit val
		UInt16 v16;
		UInt8 va8, vb8 = 0, vc8;

		switch((instr >> 24) & 0x0F){

			case 0:
			case 1:		//data processing immediate shift, register shift and misc instrs and mults
				if(specialInstr) goto invalid_instr;
				
				if((instr & 0x00000090UL) == 0x00000090){		//multiplies, extra load/stores (table 3.2)

					if((instr & 0x00000060UL) == 0x00000000){	//swp[b], mult(acc), mult(acc) long
						
						if(instr & 0x01000000UL){		//SWB/SWPB
							
							switch((instr >> 20) & 0x0F){
								
								case 0:		//SWP
									
									adr = cpuPrvGetReg(cpu, (instr >> 16) & 0x0F, wasT, specialPC);
									ok = cpu->memF(cpu, &m32, adr, 4, false, privileged, &fsr);
									if(!ok){
										cpuPrvHandleMemErr(cpu, adr, 4, false, false, fsr);
										goto instr_done;
									}
									tmp = m32;
									m32 = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);
									ok = cpu->memF(cpu, &m32, adr, 4, true, privileged, &fsr);
									if(!ok){
										cpuPrvHandleMemErr(cpu, adr, 4, true, false, fsr);
										goto instr_done;
									}
									cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, tmp);
									break;
								
								case 4:		//SWPB
									
									adr = cpuPrvGetReg(cpu, (instr >> 16) & 0x0F, wasT, specialPC);
									ok = cpu->memF(cpu, &vc8, adr, 1, false, privileged, &fsr);
									if(!ok){
										cpuPrvHandleMemErr(cpu, adr, 1, false, false, fsr);
										goto instr_done;
									}
									va8 = vc8;
									vc8 = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);
									ok = cpu->memF(cpu, &vc8, adr, 1, true, privileged, &fsr);
									if(!ok){
										cpuPrvHandleMemErr(cpu, adr, 1, true, false, fsr);
										goto instr_done;
									}
									cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, va8);
									break;
								
				#ifdef ARM_V6
				
								/* about exclusives: these are for SMP. we do not have SMP, so for now they always succeed */
				
								case 8:		//STREX
								
									if((instr & 0x00000F00UL) != 0x00000F00UL) goto invalid_instr;
									
									adr = cpuPrvGetReg(cpu, (instr >> 16) & 0x0F, wasT, specialPC);
									tmp = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);
									ok = cpu->memF(cpu, &tmp, adr, 4, true, privileged, &fsr);
									if(!ok){
										cpuPrvHandleMemErr(cpu, adr, 4, true, false, fsr);
										goto instr_done;
									}
									cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, 0);	//0 -> success
									break;
								
								
								case 9:		//LDREX
								
									if((instr & 0x00000F0FUL) != 0x00000F0FUL) goto invalid_instr;
									adr = cpuPrvGetReg(cpu, (instr >> 16) & 0x0F, wasT, specialPC);
									ok = cpu->memF(cpu, &tmp, adr, 4, false, privileged, &fsr);
									if(!ok){
										cpuPrvHandleMemErr(cpu, adr, 4, false, false, fsr);
										goto instr_done;
									}
									cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, tmp);
									break;
								
				/*	these are a v7/v6K  thing - we do not bother				
								case 10:	//STREXD
									
									//TODO
								
								
								case 11:	//LDREXD
								
									//TODO
								
								
								case 12:	//STREXB
									
									//TODO
								
								
								case 13:	//LDREXB
									
									//TODO
								
								
								case 14:	//STREXH
									
									//TODO
								
								
								case 15:	//LDREXH
									
									//TODO
					*/
				#endif
								default:
									goto invalid_instr;
							}
							
						}
						else switch((instr >> 20) & 0x0F){				//multiplies
						
							case 0:			//MUL
							case 1:
								
								tmp = 0;
								if(instr & 0x0000F000UL) goto invalid_instr;
								goto mul32;
							
							
							case 2:			//MLA
							case 3:
							
								tmp = cpuPrvGetReg(cpu, (instr >> 12 ) & 0x0F, wasT, specialPC);
					mul32:
								tmp += cpuPrvGetReg(cpu, (instr >> 8) & 0x0F, wasT, specialPC) * cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);
								cpuPrvSetReg(cpu, (instr >> 16) & 0x0F, tmp);
								if(instr & 0x00100000UL){	//S
									
									adr = cpu->CPSR &~ (ARM_SR_Z | ARM_SR_N);
									if(!tmp) adr |= ARM_SR_Z;
									if(tmp & 0x80000000UL) adr |= ARM_SR_N;
									cpu->CPSR = adr;
								}
								goto instr_done;
							
				#ifdef ARM_V6
							case 4:			//UMAAL
								
								v64 = u64_32_to_64((instr >> 12) & 0x0F);
								v64 = u64_add32(v64, (instr >> 16) & 0x0F);
								goto mul64;
							
							
							case 6:			//MLS
								
								tmp = cpuPrvGetReg(cpu, (instr >> 8) & 0x0F, wasT, specialPC);
								tmp *= cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);
								tmp = cpuPrvGetReg(cpu, (instr >> 12) & 0x0F, wasT, specialPC) - tmp;
								cpuPrvSetReg(cpu, (instr >> 16) & 0x0F, tmp);
								break;
									
				#endif
							case 8:			//UMULL
							case 9:
							case 12:		//SMULL
							case 13:
								
								v64 = u64_zero();
								goto mul64;
								
							case 10:		//UMLAL
							case 11:
							case 14:		//SMLAL
							case 15:
								
								v64 = u64_from_halves(cpuPrvGetReg(cpu, (instr >> 16) & 0x0F, wasT, specialPC), cpuPrvGetReg(cpu, (instr >> 12) & 0x0F, wasT, specialPC));
								
					mul64:				
								adr = cpuPrvGetReg(cpu, (instr >> 8) & 0x0F, wasT, specialPC);
								tmp = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);
								
								v64 = u64_add(v64, (instr & 0x00400000UL) ? u64_smul3232(adr, tmp) : u64_umul3232(adr, tmp));
								
								v32 = u64_get_hi(v64);
								cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, u64_64_to_32(v64));
								cpuPrvSetReg(cpu, (instr >> 16) & 0x0F, v32);
								
								if(instr & 0x00100000UL){	//S
									
									adr = cpu->CPSR &~ (ARM_SR_Z | ARM_SR_N);
									if(u64_isZero(v64)) adr |= ARM_SR_Z;
									if(v32 & 0x80000000UL) adr |= ARM_SR_N;
									cpu->CPSR = adr;
								}
								break;
							
							default:
								goto invalid_instr;
						}
					}
					else{	//load/store signed/unsigned byte/halfword/two_words
					
						UInt32 store[2] = {0,0};
						UInt8* store8 = (UInt8*)store;
						UInt16* store16 = (UInt16*)store;
						
						va8 = cpuPrvArmAdrMode_3(cpu, instr, &m32, &x32, wasT, specialPC);
						tmp = m32;
						v32 = x32;
						if(va8 & ARM_MODE_3_INV ) goto invalid_instr;
						adr = cpuPrvGetReg(cpu, va8 & ARM_MODE_3_REG, wasT, specialPC);
						
						switch(va8 & ARM_MODE_3_TYPE){
							case ARM_MODE_3_H:
							case ARM_MODE_3_SH:
								vb8 = 2;
								break;
							
							case ARM_MODE_3_SB:
								vb8 = 1;
								break;
							
							case ARM_MODE_3_D:
								vb8 = 8;
								break;
						}
						if(va8 & ARM_MODE_3_LOAD){
							
							ok = cpu->memF(cpu, store, adr + tmp, vb8, false, privileged, &fsr);
							if(!ok){
								cpuPrvHandleMemErr(cpu, adr + tmp, vb8, false, false, fsr);
								goto instr_done;
							}
							if(vb8 == 1){
								tmp = *store8;
								if(tmp & 0x80) tmp |= 0xFFFFFF00UL;	//sign-extend	
								cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, tmp);
							}
							else if(vb8 == 2){
								tmp = *store16;
								if(((va8 & ARM_MODE_3_TYPE) == ARM_MODE_3_SH) && (tmp & 0x8000UL)) tmp |= 0xFFFF0000UL;
								cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, tmp);
							}
							else{
								cpuPrvSetReg(cpu, ((instr >> 12) & 0x0F) + 0, store[0]);
								cpuPrvSetReg(cpu, ((instr >> 12) & 0x0F) + 1, store[1]);
							}
							if(v32) cpuPrvSetReg(cpu, va8 & ARM_MODE_3_REG, v32 + adr);
						}
						else{
							if(v32){
								v32 += adr;
								va8 |= ARM_MODE_3_INV;	//re-use flag to mean writeback
							}
							if(vb8 == 1){
								*store8 = cpuPrvGetReg(cpu, (instr >> 12) & 0x0F, wasT, specialPC);
							}
							else if(vb8 == 2){
								*store16 = cpuPrvGetReg(cpu, (instr >> 12) & 0x0F, wasT, specialPC);
							}
							else{
								store[0] = cpuPrvGetReg(cpu, ((instr >> 12) & 0x0F) + 0, wasT, specialPC);
								store[1] = cpuPrvGetReg(cpu, ((instr >> 12) & 0x0F) + 1, wasT, specialPC);
							}
							adr += tmp;
							ok = cpu->memF(cpu, store, adr, vb8, true, privileged, &fsr);
							if(!ok){
								cpuPrvHandleMemErr(cpu, adr, vb8, true, false, fsr);
								goto instr_done;
							}
							if(va8 & ARM_MODE_3_INV) cpuPrvSetReg(cpu, va8 & ARM_MODE_3_REG, v32);
						}
					}
					goto instr_done;
				}
				else if((instr & 0x01900000UL) == 0x01000000UL){	//misc instrs (table 3.3)
						
					tmp = (instr >> 4) & 0x0F;
					
					switch(tmp){
						
						case 0:		//move reg to PSR or move PSR to reg
						
							if((instr & 0x00BF0FFFUL) == 0x000F0000UL){	//move PSR to reg
								
								cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, (instr & 0x00400000UL) ? cpu->SPSR : cpu->CPSR);	//access in user and sys mode is undefined. for us that means returning garbage that is currently in "cpu->SPSR"
							}
							else if((instr & 0x00B0FFF0UL) == 0x0020F000UL){	//move reg to PSR
								
								cpuPrvSetPSR(cpu, (instr >> 16) & 0x0F, privileged, (instr & 0x00400000UL) !=0, cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC));
							}
							else goto invalid_instr;
							goto instr_done;
					
						case 1:					//BLX/BX/BXJ or CLZ
						case 3:
						
							if(instr & 0x00400000UL){		//CLZ
								
								if((instr & 0x0FFF0FF0UL) != 0x016F0F10UL) goto invalid_instr;
								tmp = cpuPrvGetReg(cpu, instr &0xF, wasT, specialPC);
								adr = 0x80000000UL;
								for(va8 = 0; va8 < 32; va8++, adr >>= 1) if(tmp & adr) break;
								cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, va8);
							}
							else{				//BL / BLX / BXJ
								
								if((instr & 0x0FFFFF00UL) != 0x012FFF00UL) goto invalid_instr;
								
								if((instr & 0x00000030UL) == 0x00000030UL) cpuPrvSetReg(cpu, 14, instrPC + (wasT ? 3 : 4));	//save return value for BLX
								cpuPrvSetPC(cpu, cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC));
							}
							goto instr_done;
						
						case 5:					//enhanced DSP adds/subtracts
						
							if(instr & 0x00000F00UL) goto invalid_instr;
							tmp = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);		//Rm
							adr = cpuPrvGetReg(cpu, (instr >> 16) & 0x0F, wasT, specialPC);	//Rn
							vb8 = 0;					//used as boolead for if saturation happened
							switch((instr >> 21) & 3){		//what op?
								
								case 0:			//QADD
								
									v32 = tmp + adr;
									vb8 = cpuPrvSignedAdditionOverflows(adr, tmp, v32);
									if(vb8) v32 = cpuPrvMedia_signedSaturate32(adr);
									break;
								
								case 1:			//QSUB
									
									v32 = tmp - adr;
									vb8 = cpuPrvSignedAdditionOverflows(tmp, adr, v32);
									if(vb8) v32 = cpuPrvMedia_signedSaturate32(tmp);
									break;
								case 2:			//QDADD
									
									v32 = adr << 1;
									vb8 = cpuPrvSignedAdditionOverflows(adr, tmp, v32);
									if(vb8) v32 = cpuPrvMedia_signedSaturate32(adr);
									adr = v32;
									v32 = tmp + adr;
									if(cpuPrvSignedAdditionOverflows(adr, tmp, v32)){
										vb8 = 1;
										v32 = cpuPrvMedia_signedSaturate32(adr);
									}
									break;
								
								case 3:			//QDSUB
									
									v32 = adr << 1;
									vb8 = cpuPrvSignedAdditionOverflows(adr, tmp, v32);
									if(vb8) v32 = cpuPrvMedia_signedSaturate32(adr);
									adr = v32;
									v32 = tmp + adr;
									if(cpuPrvSignedAdditionOverflows(adr, tmp, v32)){
										vb8 = 1;
										v32 = cpuPrvMedia_signedSaturate32(adr);
									}
									break;
								
								default:
									v32 = 0;	//make compiler happy;
									break;
								
							}
							cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, v32);	//save result
							if(vb8) cpu->CPSR |= ARM_SR_Q;
							goto instr_done;
							
						case 7:					//soft breakpoint
					
							cpuPrvException(cpu, cpu->vectorBase + ARM_VECTOR_OFFT_P_ABT, instrPC + 4, ARM_CPSR_PAB_ORR | (cpu->CPSR & ARM_CPSR_PAB_ORR));
							goto instr_done;
						
						case 8:					//enhanced DSP multiplies
						case 9:
						case 10:
						case 11:
						case 12:
						case 13:
						case 14:
						case 15:
							if((instr & 0x00000090UL) != 0x00000080UL) goto invalid_instr;
							tmp = cpuPrvGetReg(cpu, instr & 0x0F, wasT, specialPC);		//Rm
							adr = cpuPrvGetReg(cpu, (instr >> 8) & 0x0F, wasT, specialPC);	//Rs
							vb8 = 0;					//used as boolead for if saturation happened
							va8 = 0;					//used as temporary boolean flag
							switch((instr >> 21) & 3){		//what op?
								case 0:			//SMLAxy
									if(instr & 0x20) tmp >>= 16;
									else tmp &= 0xFFFFUL;
									if(tmp & 0x8000UL) tmp |= 0xFFFF0000UL;
									
									if(instr & 0x40) adr >>= 16;
									else adr &= 0xFFFFUL;
									if(adr & 0x8000UL) adr |= 0xFFFF0000UL;
									
									tmp *= adr;
									adr = cpuPrvGetReg(cpu, (instr >> 12) & 0x0F, wasT, specialPC);	//Rn
									v32 = tmp + adr;
									vb8 = cpuPrvSignedAdditionOverflows(adr, tmp, v32);
									break;
								
								case 1:			//SMLAWy/SMULWy
								
									if(instr & 0x40) adr >>= 16;
									else adr &= 0xFFFFUL;
									if(adr & 0x8000UL) adr |= 0xFFFF0000UL;
									
									adr = u64_64_to_32(u64_shr(u64_smul3232(tmp, adr), 16));	//do the multiplication, WAS: adr = (((UInt64)tmp) * ((UInt64)adr)) >> 16;
									
									if(instr & 0x20){	//SMULWy
										
										v32 = adr;
										if(instr & 0x0000F000UL) goto invalid_instr;
									}
									else{			//SMLAWy
										
										tmp = cpuPrvGetReg(cpu, (instr >> 12) & 0x0F, wasT, specialPC);	//Rn
										v32 = adr + tmp;
										vb8 = cpuPrvSignedAdditionOverflows(adr, tmp, v32);
									}
									break;
									
								case 2:			//SMLALxy
								
									if(instr & 0x20) tmp >>= 16;
									else tmp &= 0xFFFFUL;
									if(tmp & 0x8000UL) tmp |= 0xFFFF0000UL;
									
									if(instr & 0x40) adr >>= 16;
									else adr &= 0xFFFFUL;
									if(adr & 0x8000UL) adr |= 0xFFFF0000UL;
									
									adr *= tmp;
									if(adr & 0x80000000UL) va8 |= 1;	//neg
									tmp = cpuPrvGetReg(cpu, (instr >> 12) & 0x0F, wasT, specialPC);	//RdLo
									if((tmp + adr) < tmp) va8 |= 2; //carry
									cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, tmp);
									v32 = cpuPrvGetReg(cpu, (instr >> 16) & 0x0F, wasT, specialPC);	//RdHi
									if(va8 & 2) v32++;
									if(va8 & 1) v32--;
									break;
									
								case 3:			//SMULxy
								
									if(instr & 0x0000F000UL) goto invalid_instr;
									
									if(instr & 0x20) tmp >>= 16;
									else tmp &= 0xFFFFUL;
									if(tmp & 0x8000UL) tmp |= 0xFFFF0000UL;
									
									if(instr & 0x40) adr >>= 16;
									else adr &= 0xFFFFUL;
									if(adr & 0x8000UL) adr |= 0xFFFF0000UL;
									
									v32 = tmp * adr;
									break;
								
								default:		//make compiler happy
									
									v32 = 0;
									break;
							}
							cpuPrvSetReg(cpu, (instr >> 16) & 0x0F, v32);
							if(vb8) cpu->CPSR |= ARM_SR_Q;
							goto instr_done;
							
						default:
							goto invalid_instr;
					}
				}
				
				goto data_processing;
				break;
				
			case 2:
			case 3:		//data process immediate val, move imm to SR

				if(specialInstr) goto invalid_instr;
				
				if((instr & 0x01900000UL) == 0x01000000UL){	//all NON-data-processing instrs in this space are here
					
					if(instr & 0x00200000UL){		//MSR imm and hints
						
						if((instr & 0x00400000UL) || (instr & 0x000F0000UL)){	//move imm to PSR
							
							cpuPrvSetPSR(cpu, (instr >> 16) & 0x0F, privileged, (instr & 0x00400000UL) != 0, cpuPrvROR(instr & 0xFF, ((instr >> 8) & 0x0F) * 2));
						}
						else{
		#ifdef ARM_V6	
							if((instr & 0x000000F0UL) == 0x000000F0){		//debug hint
								
								err_str("DEBUG hint 0x");
								err_hex(instr & 0x0F);
								err_str(" at 0x");
								err_hex(instrPC);
								err_str("\r\n");
							}
							else switch(instr){
								
								case 0:		//NOP
									break;
								
								case 1:		//YIELD;
									break;
								
								case 2:		//WFE
									break;
								
								case 3:		//WFI
									break;
								
								case 4:		//SEV
									break;
							}
							goto instr_done;	//all hints are treated as valid and do nothing...
		#endif
							goto invalid_instr;
						}
					}
					else if(instr & 0x00400000UL){	//MOVT (high halfword 16-bit immediate load)
						
						adr = cpuPrvGetReg(cpu, (instr >> 12) & 0x0F, wasT, specialPC) & 0xFFFFUL;
						adr |= ((instr & 0xFFFUL) << 16) | ((instr & 0xF0000UL) << 12);
						cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, adr);
					}
					else{				//MOVW (16-bit immediate load)
						
						cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, (instr & 0xFFFUL) | ((instr >> 4) & 0xF000UL));
					}
					goto instr_done;
				}
				
data_processing:							//data processing
				{
					Boolean carryOut, carryIn, V, S, store = true;
					S = (instr & 0x00100000UL) != 0;
					V = (cpu->CPSR & ARM_SR_V) != 0;
					carryIn = (cpu->CPSR & ARM_SR_C) != 0;
					tmp = cpuPrvArmAdrMode_1(cpu, instr, &carryOut, wasT, specialPC);
					va8 = (instr >> 16) & 0x0F;
					
					switch((instr >> 21) & 0x0F){
						case 0:			//AND
							tmp = cpuPrvGetReg(cpu, va8, wasT, specialPC) & tmp;
							break;
						
						case 1:			//EOR
						
							tmp = cpuPrvGetReg(cpu, va8, wasT, specialPC) ^ tmp;
							break;
						
						case 2:			//SUB
						
							adr = cpuPrvGetReg(cpu, va8, wasT, specialPC);
							v32 = adr - tmp;
							if(S) V = cpuPrvSignedSubtractionOverflows(adr, tmp, v32);
							if(S) carryOut = adr >= tmp;
							tmp = v32;
							break;
						
						case 3:			//RSB
						
							adr = cpuPrvGetReg(cpu, va8, wasT, specialPC);
							v32 = tmp - adr;
							if(S) V = cpuPrvSignedSubtractionOverflows(tmp, adr, v32);
							if(S) carryOut = tmp >= adr;
							tmp = v32;
							break;
						
						case 4:			//ADD
							
							adr = cpuPrvGetReg(cpu, va8, wasT, specialPC);
							v32 = adr + tmp;
							if(S) V = cpuPrvSignedAdditionOverflows(adr, tmp, v32);
							if(S) carryOut = v32 < adr;
							tmp = v32;
							break;
						
						case 5:			//ADC
						
							adr = cpuPrvGetReg(cpu, va8, wasT, specialPC);
							if(carryIn){
								v32 = adr + tmp + 1;
								if(S) carryOut = v32 <= adr;
							}
							else{
								v32 = adr + tmp;
								if(S) carryOut = v32 < adr;
							}
							if(S) V = cpuPrvSignedAdditionOverflows(adr, tmp, v32);
							tmp = v32;
							break;
						
						case 6:			//SBC
						
							adr = cpuPrvGetReg(cpu, va8, wasT, specialPC);
							if(carryIn){
								
								v32 = adr - tmp;
								if(S) carryOut = adr >= tmp;
							}
							else{
								v32 = adr - tmp - 1;
								if(S) carryOut = adr > tmp;
							}
							if(S) V = cpuPrvSignedSubtractionOverflows(adr, tmp, v32);
							tmp = v32;
							break;
						
						case 7:			//RSC
						
							adr = cpuPrvGetReg(cpu, va8, wasT, specialPC);
							if(carryIn){
								
								v32 = tmp - adr;
								if(S) carryOut = tmp >= adr;
							}
							else{
								v32 = tmp - adr - 1;
								if(S) carryOut = tmp > adr;
							}
							if(S) V = cpuPrvSignedSubtractionOverflows(tmp, adr, v32);
							tmp = v32;
							break;
						
						case 8:			//TST
							if(!S) goto invalid_instr;
							store = false;
							tmp = cpuPrvGetReg(cpu, va8, wasT, specialPC) & tmp;
							break;
						
						case 9:			//TEQ
						
							if(!S) goto invalid_instr;
							store = false;
							tmp = cpuPrvGetReg(cpu, va8, wasT, specialPC) ^ tmp;
							break;
						
						case 10:		//CMP
						
							if(!S) goto invalid_instr;
							store = false;
							adr = cpuPrvGetReg(cpu, va8, wasT, specialPC);
							V = cpuPrvSignedSubtractionOverflows(adr, tmp, adr - tmp);	//((adr ^ tmp) & (adr ^ (adr - tmp))) >> 31;
							carryOut = adr >= tmp;
							tmp = adr - tmp;
							break;
						
						case 11:		//CMN
						
							if(!S) goto invalid_instr;
							store = false;
							adr = cpuPrvGetReg(cpu, va8, wasT, specialPC);
							v32 = adr + tmp;
							V = cpuPrvSignedAdditionOverflows(adr, tmp, v32);
							carryOut = v32 < adr;
							tmp = v32;
							break;
						
						case 12:		//ORR
						
							tmp = cpuPrvGetReg(cpu, va8, wasT, specialPC) | tmp;
							break;
						
						case 13:		//MOV
						
							//tmp already equals tmp
							break;
						
						case 14:		//BIC
						
							tmp = cpuPrvGetReg(cpu, va8, wasT, specialPC) & ~tmp;
							break;
						
						case 15:		//MVN
						
							tmp = ~tmp;
							break;
					}
					vb8 = (instr >> 12) & 0x0F;
					if(S){	//update flags or restore CPSR
						
						if(!usesUsrRegs && vb8 == 15 && store){
							
							UInt32 sr;
							
							sr = cpu->SPSR;
							cpuPrvSwitchToMode(cpu, sr & ARM_SR_M);
							cpu->CPSR = sr;
							cpu->regs[15] = tmp;	//do it right here - if we let it use cpuPrvSetReg, it will check lower bit...
							store = false;
						}
						else{
							adr = cpu->CPSR &~ (ARM_SR_Z | ARM_SR_N | ARM_SR_C | ARM_SR_V);
							if(!tmp) adr |= ARM_SR_Z;
							if(tmp & 0x80000000UL) adr |= ARM_SR_N;
							if(carryOut) adr |= ARM_SR_C;
							if(V) adr |= ARM_SR_V;
							cpu->CPSR = adr;
						}
					}
					if(store){
						if(vb8 == 15){
							cpuPrvSetReg(cpu, vb8, tmp &~ 1UL);
							cpu->CPSR &=~ ARM_SR_T;
							if(tmp & 1) cpu->CPSR |= ARM_SR_T;
						}
						else{
							cpu->regs[vb8] = tmp;	//not pc - no need for func call cpuPrvSetReg(cpu, vb8, tmp);
						}
					}
					goto instr_done;
				}
				break;

			case 4:
			case 5:		//load/stor imm offset
			
				goto load_store_mode_2;
				break;
				
			case 6:
			case 7:		//load/store reg offset

				if(instr & 0x00000010UL){		//media and undefined instrs
		
		#ifdef ARM_V6
					if(cpuPrvMediaInstrs(cpu, instr, wasT, specialPC)) goto instr_done;	
		#endif		
					goto invalid_instr;
				}

load_store_mode_2:
				if(specialInstr){	//handle PLD
					
					if((instr & 0x0D70F000UL) == 0x0550F000UL) goto instr_done;	//PLD
					goto invalid_instr;
				}
				
				va8 = cpuPrvArmAdrMode_2(cpu, instr, &m32, &x32, wasT, specialPC);
				tmp = m32;
				v32 = x32;
				if(va8 & ARM_MODE_2_INV) goto invalid_instr;
				if(va8 & ARM_MODE_2_T) privileged = false;
				vb8 = (va8 & ARM_MODE_2_WORD) ? 4 : 1;	//get operation size
				
				adr = cpuPrvGetReg(cpu, va8 & ARM_MODE_2_REG, wasT, specialPC);
				
				if(va8 & ARM_MODE_2_LOAD){
					
					ok = cpu->memF(cpu, &m32, adr + tmp, vb8, false, privileged, &fsr);
					if(!ok){
						cpuPrvHandleMemErr(cpu, adr + tmp, vb8, false, false, fsr);
						goto instr_done;
					}
					if(vb8 == 1) m32 = *(UInt8*)&m32;	//endian-free way to make it a valid 8-bit value, if need be
					tmp = m32;
					cpuPrvSetReg(cpu, (instr >> 12) & 0x0F, tmp);
					if(v32) cpuPrvSetReg(cpu, va8 & ARM_MODE_2_REG, v32 + adr);
				}
				else{
					if(v32){
						v32 += adr;
						va8 |= ARM_MODE_2_INV;	//re-use flag to mean writeack
					}
					
					adr += tmp;
					if(vb8 == 1){
						m32 = 0;
						*(UInt8*)&m32 = cpuPrvGetReg(cpu, (instr >> 12) & 0x0F, wasT, specialPC);
					}
					else{
						m32 = cpuPrvGetReg(cpu, (instr >> 12) & 0x0F, wasT, specialPC);
					}
					ok = cpu->memF(cpu, &m32, adr, vb8, true, privileged, &fsr);
					if(!ok){
						cpuPrvHandleMemErr(cpu, adr, vb8, true, false, fsr);
						goto instr_done;
					}
					if(va8 & ARM_MODE_2_INV) cpuPrvSetReg(cpu, va8 & ARM_MODE_2_REG, v32);
				}
				
				goto instr_done;

			case 8:
			case 9:		//load/store multiple

				if(specialInstr) goto invalid_instr;

				va8 = cpuPrvArmAdrMode_4(cpu, instr, &v16);
				if((va8 & ARM_MODE_4_S) && usesUsrRegs) goto invalid_instr;	//no S mode please in modes with no baked regs //or SPSR
				L = (instr & 0x00100000UL) != 0;
				tmp = adr = cpuPrvGetReg(cpu, va8 & ARM_MODE_4_REG, wasT, specialPC);
				
				specialInstr = L && (va8 & ARM_MODE_4_S) && (v16 & 0x8000UL) && !usesUsrRegs;	//specialInstr = "copyCPSR"
				
				for(vc8 = 0; vc8 < 16; vc8++){

					vb8 = (va8 & ARM_MODE_4_INC) ? vc8 : 15 - vc8;

					if(v16 & (1UL << vb8)){
						
						UInt32* reg = cpu->regs + vb8;
						
						if(L){
							if(va8 & ARM_MODE_4_S){		//handle LDM(2) and LDM(3)
							
								if(v16 & 0x8000UL){	//handle LDM(3)
									
									/* nothing to do here, we did all we need above in line beginning with "specialInstr=" */
								}
								else if(!usesUsrRegs){	//handle LDM(2)
									
									if(vb8 >= 8 && vb8 <= 12 && (cpu->CPSR & ARM_SR_M) == ARM_SR_MODE_FIQ){	//handle fiq/usr banked regs
										
										reg = cpu->extra_regs + vb8 - 8;
									}
									else if(vb8 == 13){
										
										reg = &cpu->bank_usr.R13;
									}
									else if(vb8 == 14){
										
										reg = &cpu->bank_usr.R14;
									}
								}
							}
							else if(vb8 == 15){		//handle LDM(1)'s use of PC
								
								/* nothing to do here - all is handled below */
							}
						}
						else if(va8 & ARM_MODE_4_S){		//handle STM(2)'s access to user regs
							
							if(!usesUsrRegs){
									
								if(vb8 >= 8 && vb8 <= 12 && (cpu->CPSR & ARM_SR_M) == ARM_SR_MODE_FIQ){	//handle fiq/usr banked regs
									
									reg = cpu->extra_regs + vb8 - 8;
								}
								else if(vb8 == 13){
									
									reg = &cpu->bank_usr.R13;
								}
								else if(vb8 == 14){
									
									reg = &cpu->bank_usr.R14;
								}
							}
						}
						if(va8 & ARM_MODE_4_BFR) adr += (va8 & ARM_MODE_4_INC) ? 4L : -4L;
						ok = cpu->memF(cpu, reg, adr, 4, !L, privileged, &fsr);
						if(!ok){
							cpuPrvHandleMemErr(cpu, adr, 4, !L, false, fsr);
							if(v16 & (1UL << (va8 & ARM_MODE_4_REG))) cpuPrvSetReg(cpu, va8 & ARM_MODE_4_REG, tmp);
							goto instr_done;
						}
						if(!(va8 & ARM_MODE_4_BFR)) adr += (va8 & ARM_MODE_4_INC) ? 4L : -4L;
					}
				}
				if(va8 & ARM_MODE_4_WBK){
					cpuPrvSetReg(cpu, va8 & ARM_MODE_4_REG, adr);
				}
				
				if(specialInstr){		//process LDM(3) SPSR->CPSR copy
					v32 = cpu->SPSR;
					cpuPrvSwitchToMode(cpu, v32 & ARM_SR_M);
					cpu->CPSR = v32;	
				}
				else if((v16 & 0x8000U) && !(va8 & ARM_MODE_4_S)){	//we just loaded PC
					if(cpu->regs[15] & 1){
						cpu->regs[15] &=~ 1UL;
						cpu->CPSR |= ARM_SR_T;	
					}
					else{
						cpu->CPSR &=~ ARM_SR_T;
					}
				}
				
				goto instr_done;

			case 10:
			case 11:	//B/BL/BLX(if cond=0b1111)

				tmp = instr & 0x00FFFFFFUL;			//get offset
				if(tmp & 0x00800000UL) tmp |= 0xFF000000UL;	//sign extend
				tmp = tmp << (wasT ? 1 : 2);			//shift left 2(ARM) or 1(thumb)
				tmp += instrPC + (wasT ? 4 : 8); 		//add where PC would point in an ARM 
				if(specialInstr){				//handle BLX
					if(instr & 0x01000000UL) tmp += 2;
					cpu->regs[14] = instrPC + (wasT ? 2 : 4);
					if(!(cpu->CPSR & ARM_SR_T)) tmp |= 1UL;	//set T flag if needed
				}
				else{						//not BLX -> differentiate between BL and B
					if(instr & 0x01000000UL) cpu->regs[14] = instrPC + (wasT ? 2 : 4);
					if(cpu->CPSR & ARM_SR_T) tmp |= 1UL;	//keep T flag as needed
				}
				cpuPrvSetPC(cpu, tmp);
				goto instr_done;

			case 12:
			case 13:	//coprocessor load/store and double register transfers

				va8 = cpuPrvArmAdrMode_5(cpu, instr, &m32);
				v32 = m32;
				vb8 = (instr >> 8) & 0x0F;
				
				if(vb8 >= 14){						//cp14 and cp15 are for priviledged users only
					if(!privileged) goto invalid_instr;
				}
				else if(!(cpu->CPAR & (1UL << vb8))) goto invalid_instr;	//others are access-controlled by CPAR
				
				if(va8 & ARM_MODE_5_RR){		//handle MCRR, MRCC
					
					if(!cpu->coproc[vb8].twoRegF) goto invalid_instr;
					if(!cpu->coproc[vb8].twoRegF(cpu, cpu->coproc[vb8].userData, (instr & 0x00100000UL) != 0, (instr >> 4) & 0x0F,(instr >> 12) & 0x0F, (instr >> 16) & 0x0F, instr & 0x0F)) goto invalid_instr;
				}
				else{
					vc8 = v32;
					tmp = adr = cpuPrvGetReg(cpu, va8 & ARM_MODE_5_REG, wasT, specialPC);
					tmp += v32;
					
					if(!cpu->coproc[vb8].memAccess) goto invalid_instr;
					if(!cpu->coproc[vb8].memAccess(cpu, cpu->coproc[vb8].userData, specialInstr, (instr & 0x00400000UL) !=0, !(instr & 0x00100000UL), (instr >> 12) & 0x0F, (va8 & ARM_MODE_5_ADD_BEFORE) ? tmp : adr, (va8 & ARM_MODE_5_IS_OPTION) ? &vc8 : NULL)) goto invalid_instr;
					if(va8 & ARM_MODE_5_ADD_AFTER) cpuPrvSetReg(cpu, va8 & ARM_MODE_5_REG, tmp);
				}
				goto instr_done;

			case 14:	//coprocessor data processing and register transfers

				vb8 = (instr >> 8) & 0x0F;
				
				if(vb8 >= 14){						//cp14 and cp15 are for priviledged users only
					if(!privileged) goto invalid_instr;
				}
				else if(!(cpu->CPAR & (1UL << vb8))) goto invalid_instr;	//others are access-controlled by CPAR
				
				if(instr & 0x00000010UL){		//MCR/MRC
					
					if(!cpu->coproc[vb8].regXfer) goto invalid_instr;
					if(!cpu->coproc[vb8].regXfer(cpu, cpu->coproc[vb8].userData, specialInstr, (instr & 0x00100000UL) != 0, (instr >> 21) & 0x07, (instr >> 12) & 0x0F, (instr >> 16) & 0x0F, instr & 0x0F, (instr >> 5) & 0x07)) goto invalid_instr;
				}
				else{				//CDP
					
					if(!cpu->coproc[vb8].dataProcessing) goto invalid_instr;
					if(!cpu->coproc[vb8].dataProcessing(cpu, cpu->coproc[vb8].userData, specialInstr, (instr >> 20) & 0x0F, (instr >> 12) & 0x0F, (instr >> 16) & 0x0F, instr & 0x0F, (instr >> 5) & 0x07)) goto invalid_instr;
				}
				goto instr_done;

			case 15:	//SWI

				if(specialInstr) goto invalid_instr;

				cpuPrvException(cpu, cpu->vectorBase + ARM_VECTOR_OFFT_SWI, instrPC + (wasT ? 2 : 4), ARM_CPSR_SWI_ORR | (cpu->CPSR & ARM_CPSR_SWI_AND));
				goto instr_done;
		}

	invalid_instr:

		if(instr == HYPERCALL_ARM && privileged){
			if(cpu->hypercallF && cpu->hypercallF(cpu)) goto instr_done;
		}

		err_str("Invalid instr 0x");
		err_hex(instr);
		err_str(" seen at 0x");
		err_hex(instrPC);
		err_str(". CPSR=0x");
		err_hex(cpu->CPSR);
		err_str("\r\n");
		cpuPrvException(cpu, cpu->vectorBase + ARM_VECTOR_OFFT_UND, instrPC + (wasT ? 2 : 4), ARM_CPSR_UND_ORR | (cpu->CPSR & ARM_CPSR_UND_AND));

	instr_done:;
	}

	return errNone;
}

static Err cpuPrvCycleArm(ArmCpu* cpu){
	
	Boolean privileged, ok;
	UInt32 instr, pc;
	UInt8 fsr;

	privileged = (cpu->CPSR & ARM_SR_M) != ARM_SR_MODE_USR;
	//fetch instruction
	{
		ok = icacheFetch(&cpu->ic, pc = cpu->regs[15], 4, privileged, &fsr, &instr);
		if(!ok){
			cpuPrvHandleMemErr(cpu, cpu->regs[15], 4, false, true, fsr);
			return errNone;						//exit here so that debugger can see us execute first instr of execption handler
		}
		cpu->regs[15] += 4;
	}
	
	return cpuPrvExecInstr(cpu, instr, pc, false, privileged, false);
}


static Err cpuPrvCycleThumb(ArmCpu* cpu){
	
	Boolean privileged, vB, specialPC = false, ok;
	UInt32 instr = 0xE0000000UL /*most likely thing*/, pc;
	UInt16 instrT, v16;
	UInt8 v8, fsr;

	privileged = (cpu->CPSR & ARM_SR_M) != ARM_SR_MODE_USR;
	
	pc = cpu->regs[15];
	ok = icacheFetch(&cpu->ic, pc, 2, privileged, &fsr, &instrT);
	if(!ok){
		cpuPrvHandleMemErr(cpu, pc, 2, false, true, fsr);
		return errNone;						//exit here so that debugger can see us execute first instr of execption handler
	}
	cpu->regs[15] += 2;
	
	switch(instrT >> 12){
		
		case 0:		// LSL(1) LSR(1) ASR(1) ADD(1) SUB(1) ADD(3) SUB(3)
		case 1:
			if((instrT & 0x1800) != 0x1800){	// LSL(1) LSR(1) ASR(1)
				
				instr |= 0x01B00000UL | ((instrT & 0x7) << 12) | ((instrT >> 3) & 7) | ((instrT >> 6) & 0x60) | ((instrT << 1) & 0xF80);
			}
			else{
				vB = (instrT & 0x0200) != 0;	// SUB or ADD ?
				instr |= ((vB ? 5UL : 9UL) << 20) | (((UInt32)(instrT & 0x38)) << 13) | ((instrT & 0x07) << 12) | ((instrT >> 6) & 0x07);
				
				if(instrT & 0x0400){		// ADD(1) SUB(1)
					
					instr |= 0x02000000UL;
				}
				else{				// ADD(3) SUB(3)
				
					// nothing to do here
				}
			}
			break;
		
		case 2:		// MOV(1) CMP(1) ADD(2) SUB(2)
		case 3:
			instr |= instrT & 0x00FF;
			switch((instrT >> 11) & 3){
				case 0:				// MOV(1)
					instr |= 0x03B00000UL | ((instrT & 0x0700) << 4);
					break;
				
				case 1:				// CMP(1)
					instr |= 0x03500000UL | (((UInt32)(instrT & 0x0700)) << 8);
					break;
				
				case 2:				// ADD(2)
					instr |= 0x02900000UL | ((instrT & 0x0700) << 4) | (((UInt32)(instrT & 0x0700)) << 8);
					break;
				
				case 3:				// SUB(2)
					instr |= 0x02500000UL | ((instrT & 0x0700) << 4) | (((UInt32)(instrT & 0x0700)) << 8);
					break;
			}
			break;
		
		case 4:		// LDR(3) ADD(4) CMP(3) MOV(3) BX MVN CMP(2) CMN TST ADC SBC NEG MUL LSL(2) LSR(2) ASR(2) ROR AND EOR ORR BIC
		
			if(instrT & 0x0800){			// LDR(3)
				
				instr |= 0x059F0000UL | ((instrT & 0xFF) << 2) | ((instrT & 0x700) << 4);
				specialPC = true;
			}
			else if(instrT & 0x0400){		// ADD(4) CMP(3) MOV(3) BX
				
				UInt8 vD;
				
				vD = (instrT & 7) | ((instrT >> 4) & 0x08);
				v8 = (instrT >> 3) & 0xF;
				
				switch((instrT >> 8) & 3){
					
					case 0:			// ADD(4)
						
						instr |= 0x00800000UL | (((UInt32)vD) << 16) | (((UInt32)vD) << 12) | v8;
						break;
					
					case 1:			// CMP(3)
						
						instr |= 0x01500000UL | (((UInt32)vD) << 16) | v8;
						break;
					
					case 2:			// MOV(3)
						
						instr |= 0x01A00000UL | (((UInt32)vD) << 12) | v8;
						break;
					
					case 3:			// BX
						
						if(instrT == 0x4778){	//special handing for thumb's "BX PC" as aparently docs are wrong on it
							
							cpuPrvSetPC(cpu, (cpu->regs[15] + 2) &~ 3UL);
							goto instr_done;
						}
						
						instr |= 0x012FFF10UL | ((instrT >> 3) & 0x0F);
						break;
				}
			}
			else{					// AND EOR LSL(2) LSR(2) ASR(2) ADC SBC ROR TST NEG CMP(2) CMN ORR MUL BIC MVN (in val_tabl order)
				static const UInt32 val_tabl[16] = {0x00100000UL, 0x00300000UL, 0x01B00010UL, 0x01B00030UL, 0x01B00050UL, 0x00B00000UL, 0x00D00000UL, 0x01B00070UL, 0x01100000UL, 0x02700000UL, 0x01500000UL, 0x01700000UL, 0x01900000UL, 0x00100090UL, 0x01D00000UL, 0x01F00000UL};
				
				//00 = none
				//10 = bit0 val
				//11 = bit3 val
				//MVN BIC MUL ORR CMN CMP(2) NEG TST ROR SBC ADC ASR(2) LSR(2) LSL(2) EOR AND
				
				const UInt32 use16 = 0x2AAE280AUL; //0010 1010 1010 1110 0010 1000 0000 1010
				const UInt32 use12 = 0xA208AAAAUL; //1010 0010 0000 1000 1010 1010 1010 1010
				const UInt32 use8  = 0x0800C3F0UL; //0000 1000 0000 0000 1100 0011 1111 0000
				const UInt32 use0  = 0xFFF3BEAFUL; //1111 1111 1111 0011 1011 1110 1010 1111
				UInt8 vals[4] = {0};
				
				vals[2] = (instrT & 7);
				vals[3] = (instrT >> 3) & 7;
				v8 = (instrT >> 6) & 15;
				instr |= val_tabl[v8];
				v8 <<= 1;
				instr |= ((UInt32)(vals[(use16 >> v8) & 3UL])) << 16;
				instr |= ((UInt32)(vals[(use12 >> v8) & 3UL])) << 12;
				instr |= ((UInt32)(vals[(use8  >> v8) & 3UL])) <<  8;
				instr |= ((UInt32)(vals[(use0  >> v8) & 3UL])) <<  0;
			}
			break;
		
		case 5:		// STR(2)  STRH(2) STRB(2) LDRSB LDR(2) LDRH(2) LDRB(2) LDRSH		(in val_tbl orver)
			{
				static const UInt32 val_tabl[8] = {0x07800000UL, 0x018000B0UL, 0x07C00000UL, 0x019000D0UL, 0x07900000UL, 0x019000B0UL, 0x07D00000UL, 0x019000F0UL};
				instr |= ((instrT >> 6) & 7) | ((instrT & 7) << 12) | (((UInt32)(instrT & 0x38)) << 13) | val_tabl[(instrT >> 9) & 7];
			}
			break;
		
		case 6:		// LDR(1) STR(1)	(bit11 set = ldr)
			
			instr |= ((instrT & 7) << 12) | (((UInt32)(instrT & 0x38)) << 13) | ((instrT >> 4) & 0x7C) | 0x05800000UL;
			if(instrT & 0x0800) instr |= 0x00100000UL;
			break;
			
		case 7:		// LDRB(1) STRB(1)	(bit11 set = ldrb)
		
			instr |= ((instrT & 7) << 12) | (((UInt32)(instrT & 0x38)) << 13) | ((instrT >> 6) & 0x1F) | 0x05C00000UL;
			if(instrT & 0x0800) instr |= 0x00100000UL;
			break;
		
		case 8:		// LDRH(1) STRH(1)	(bit11 set = ldrh)
			
			instr |= ((instrT & 7) << 12) | (((UInt32)(instrT & 0x38)) << 13) | ((instrT >> 5) & 0x0E) | ((instrT >> 1) & 0x300) | 0x01C000B0UL;
			if(instrT & 0x0800) instr |= 0x00100000UL;
			break;
		
		case 9:		// LDR(4) STR(3)	(bit11 set = ldr)
			
			instr |= ((instrT & 0x700) << 4) | ((instrT & 0xFF) << 2) | 0x058D0000UL;
			if(instrT & 0x0800) instr |= 0x00100000UL;
			break;
		
		case 10:	// ADD(5) ADD(6)	(bit11 set = add(6))
			
			instr |= ((instrT & 0x700) << 4) | (instrT &0xFF) | 0x028D0F00UL;	//encode add to SP, line below sets the bit needed to reference PC instead when needed)
			if(!(instrT & 0x0800)) instr |= 0x00020000UL;
			else specialPC = true;
			break;
		
		case 11:	// ADD(7) SUB(4) PUSH POP BKPT
		
			if((instrT & 0x0600) == 0x0400){		//PUSH POP
				
				instr |= (instrT & 0xFF) | 0x000D0000UL;
				
				if(instrT & 0x0800){			//POP
				
					if(instrT & 0x0100) instr |= 0x00008000UL;
					instr |= 0x08B00000UL;
				}
				else{					//PUSH
					
					if(instrT & 0x0100) instr |= 0x00004000UL;
					instr |= 0x09200000UL;
				}
			}
			else if(instrT & 0x0100){
				
				goto undefined;
			}
			else switch((instrT >> 9) & 7){
				
				case 0:			// ADD(7) SUB(4)
					
					instr |= 0x020DDF00UL | (instrT & 0x7F) | ((instrT & 0x0080) ? 0x00400000UL : 0x00800000UL);
					break;
		#ifdef ARM_V6
				case 1:			//SXTH SXTB UXTH UXTB
					
					instr |= 0x060F0070UL | ((instrT >> 3) & 7) | ((instrT & 7) << 12);
					switch((instrT >> 6) & 3){
						
						case 0:		//SXTH
							instr |= 0x00B00000UL;
							break;
						
						case 1:		//SXTB
							instr |= 0x00A00000UL;
							break;
						
						case 2:		//UXTH
							instr |= 0x00F00000UL;
							break;
						
						case 3:		//UXTB
							instr |= 0x00E00000UL;
							break;
					}
					break;
				
				case 3:	//SETEND, CPY
					
					if((instrT & 0x00FE) == 0x0050){	//SETEND
						
						instr |= 0x01010000UL;
						if(instrT & 0x0008) instr |= 0x00000200UL;
					}
					else if((instrT & 0x00E8) == 0x0060){	//CPS
						
						instr |= 0x01080000UL | ((instrT & 7) << 6);
						if(instrT & 0x0010) instr |= 0x00040000UL;
					}
					else goto undefined;
					break;
				
				case 5:	//REV REV16, REVSH
					
					instr |= 0x060F0F00UL | ((instrT >> 3) & 7) | ((instrT & 7) << 12);
					switch((instrT >> 6) & 3){
						
						case 0:		//REV
							instr |= 0x00B00030UL;
							break;
						
						case 1:		//REV16
							instr |= 0x00B000B0UL;
							break;
						
						case 2:		// ???
							goto undefined;
						
						case 3:
							instr |= 0x00F000B0UL;
							break;
					}
					break;
		#endif
				case 7:	//BKPT
					
					instr |= 0x01200070UL | (instrT & 0x0F) | ((instrT & 0xF0) << 4);
					break;
				
				default:
					
					goto undefined;
			}
			break;
		
		case 12:	// LDMIA STMIA		(bit11 set = ldmia)
			instr |= 0x08800000UL | (((UInt32)(instrT & 0x700)) << 8) | (instrT & 0xFF);
			if(instrT & 0x0800) instr |= 0x00100000UL;
			if(!((1UL << ((instrT >> 8) & 0x07)) & instrT)) instr |= 0x00200000UL;	//set W bit if needed
			break;
		
		case 13:	// B(1), SWI, undefined instr space
			v8 = ((instrT >> 8) & 0x0F);
			if(v8 == 14){			// undefined instr
				goto undefined;
			}
			else if(v8 == 15){		// SWI
				instr |= 0x0F000000UL | (instrT & 0xFF);
			}
			else{				// B(1)
				instr = (((UInt32)v8) << 28) | 0x0A000000UL | (instrT & 0xFF);
				if(instrT & 0x80) instr |= 0x00FFFF00UL;
			}
			break;
		
		case 14:	// B(2) BL BLX(1) undefined instr space
		case 15:
			v16 = (instrT & 0x7FF);
			switch((instrT >> 11) & 3){
				
				case 0:		//B(2)
					
					instr |= 0x0A000000UL | v16;
					if(instrT & 0x0400) instr |= 0x00FFF800UL;
					break;
				
				case 1:		//BLX(1)_suffix
					instr = cpu->regs[15];
					cpu->regs[15] = (cpu->regs[14] + 2 + (((UInt32)v16) << 1)) &~ 3UL;
					cpu->regs[14] = instr | 1UL;
					cpu->CPSR &=~ ARM_SR_T;
					goto instr_done;
				
				case 2:		//BLX(1)_prefix BL_prefix
					instr = v16;
					if(instrT & 0x0400) instr |= 0x000FF800UL;
					cpu->regs[14] = cpu->regs[15] + (instr << 12);
					goto instr_done;
				
				case 3:		//BL_suffix
					instr = cpu->regs[15];
					cpu->regs[15] = cpu->regs[14] + 2 + (((UInt32)v16) << 1);
					cpu->regs[14] = instr | 1UL;
					goto instr_done;
			}
			
			if(instrT & 0x0800) goto undefined;	//avoid BLX_suffix and undefined instr space in there
			instr |= 0x0A000000UL | (instrT & 0x7FF);
			if(instrT & 0x0400) instr |= 0x00FFF800UL;
			break;
	}

instr_execute:
	return cpuPrvExecInstr(cpu, instr, pc, true, privileged, specialPC);
instr_done:
	return errNone;
undefined:
	if(instrT == HYPERCALL_THUMB){
		instr = HYPERCALL_ARM;
		goto instr_execute;
	}

	instr = 0xE7F000F0UL | (instrT & 0x0F) | ((instrT & 0xFFF0) << 4);	//guranteed undefined instr, inside it we store the original thumb instr :)=-)
	goto instr_execute;
}

Err cpuInit(ArmCpu* cpu, UInt32 pc, ArmCpuMemF memF, ArmCpuEmulErr emulErrF, ArmCpuHypercall hypercallF, ArmSetFaultAdrF setFaultAdrF){
	
	if(!TYPE_CHECK){
		emulErrF(cpu, "Type size error! CPU init aborted");
		return errInternal;
	}

	__mem_zero(cpu, sizeof(ArmCpu));
	
	cpu->CPSR = ARM_SR_I | ARM_SR_F | ARM_SR_MODE_SVC;	//start w/o interrupts in supervisor mode
	cpuPrvSetPC(cpu, pc);

	cpu->memF = memF;
	cpu->emulErrF = emulErrF;
	cpu->hypercallF = hypercallF;
	cpu->setFaultAdrF = setFaultAdrF;

	icacheInit(&cpu->ic, cpu, memF);

	return errNone;
}

Err cpuDeinit(_UNUSED_ ArmCpu* cpu){

	return errNone;
}

void cpuCycle(ArmCpu* cpu){

	UInt32 vector, newCPSR;

	if(cpu->waitingFiqs && !(cpu->CPSR & ARM_SR_F)){
		
		newCPSR = ARM_CPSR_FIQ_ORR | (cpu->CPSR & ARM_CPSR_FIQ_AND);
		vector = cpu->vectorBase + ARM_VECTOR_OFFT_FIQ;
	}
	else if(cpu->waitingIrqs && !(cpu->CPSR & ARM_SR_I)){
		
		newCPSR = ARM_CPSR_IRQ_ORR | (cpu->CPSR & ARM_CPSR_IRQ_AND);
		vector = cpu->vectorBase + ARM_VECTOR_OFFT_IRQ;
	}
#ifdef ARM_V6
	else if(cpu->impreciseAbtWaiting && !(cpu->CPSR & ARM_SR_A)){
		
		newCPSR = ARM_CPSR_DAB_ORR | (cpu->CPSR & ARM_CPSR_DAB_AND);
		vector = cpu->vectorBase + ARM_VECTOR_OFFT_D_ABT;
	}
#endif
	else{
		goto normal;
	}

	cpuPrvException(cpu, vector, cpu->regs[15] + 4, newCPSR);

normal:

	if(cpu->CPSR & ARM_SR_T){
		cpuPrvCycleThumb(cpu);
	}
	else{
		
		cpuPrvCycleArm(cpu);
	}
}

void cpuIrq(ArmCpu* cpu, Boolean fiq, Boolean raise){	//unraise when acknowledged

	if(fiq){
		if(raise){
			cpu->waitingFiqs++;
		}
		else if(cpu->waitingFiqs){
			cpu->waitingFiqs--;
		}
		else{
			cpu->emulErrF(cpu,"Cannot unraise FIQ when none raised");
		}
	}
	else{
		if(raise){
			cpu->waitingIrqs++;
		}
		else if(cpu->waitingIrqs){
			cpu->waitingIrqs--;
		}
		else{
			cpu->emulErrF(cpu,"Cannot unraise IRQ when none raised");
		}
	}
}

void cpuIcacheInval(ArmCpu* cpu){

	icacheInval(&cpu->ic);
}

void cpuIcacheInvalAddr(ArmCpu* cpu, UInt32 addr){

	icacheInvalAddr(&cpu->ic, addr);
}


void cpuCoprocessorRegister(ArmCpu* cpu, UInt8 cpNum, ArmCoprocessor* coproc){

	cpu->coproc[cpNum] = *coproc;
}

void cpuCoprocessorUnregister(ArmCpu* cpu, UInt8 cpNum){

	ArmCoprocessor cp;

	__mem_zero(&cp, sizeof(ArmCoprocessor));

	cpu->coproc[cpNum] = cp;	
}

void cpuSetVectorAddr(ArmCpu* cpu, UInt32 adr){
	
	cpu->vectorBase = adr;	
}

UInt16 cpuGetCPAR(ArmCpu* cpu){
	
	return cpu->CPAR;	
}

void cpuSetCPAR(ArmCpu* cpu, UInt16 cpar){

	cpu->CPAR = cpar;	
}

#ifdef ARM_V6

	void cpuSignalImpreciseAbt(ArmCpu* cpu, Boolean raise){
		
		cpu->impreciseAbtWaiting = raise;
	}


#endif

