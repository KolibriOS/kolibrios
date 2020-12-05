#include "cp15.h"


#define CPUID_PXA255		0x69052D06UL	//spepping A0
#define CPUID_PXA270		0x69054114UL	//stepping C0

static Boolean cp15prvCoprocRegXferFunc(struct ArmCpu* cpu, void* userData, Boolean two, Boolean read, UInt8 op1, UInt8 Rx, UInt8 CRn, UInt8 CRm, UInt8 op2){
	
	ArmCP15* cp15 = userData;
	UInt32 val = 0, tmp;
	
	
	if(!read) val = cpuGetRegExternal(cpu, Rx);
	
	if(op1 != 0 || two) goto fail;					//CP15 only accessed with MCR/MRC with op1 == 0
	
	switch(CRn){
		
		case 0:		//ID codes
		
			if(!read) goto fail;				//cannot write to ID codes register
			if(CRm != 0) goto fail;				//CRm must be zero for this read
			if(op2 == 0){					//main ID register
				
				val = CPUID_PXA255;
				goto success;
			}
			else if(op2 == 1){				//cahce type register - we lie here
				
				val = 0x0B16A16AUL;
				goto success;	
			}
			break;
			
		case 1:		//control register
		
			if(op2 == 0){
				if(read){
					val = cp15->control;
				}
				else{
					tmp = val ^ cp15->control;		//see what changed and mask off then chack for what we support changing of
					if(tmp & 0x84F0UL){
						err_str("cp15: unknown bits changed 0x");
						err_hex(cp15->control);
						err_str("->0x");
						err_hex(val);
						err_str(", halting\r\n");
						while(true);
					}
					
					if(tmp & 0x00002000UL){			// V bit
						
						cpuSetVectorAddr(cp15->cpu, (val & 0x00002000UL) ? 0xFFFF0000UL : 0x00000000UL);
						cp15->control ^= 0x00002000UL;
					}
					if(tmp & 0x00000200UL){			// R bit
						
						mmuSetR(cp15->mmu, (val & 0x00000200UL) != 0);
						cp15->control ^= 0x00000200UL;
					}
					if(tmp & 0x00000100UL){			// S bit
						
						mmuSetS(cp15->mmu, (val & 0x00000100UL) != 0);
						cp15->control ^= 0x00000100UL;
					}
					if(tmp & 0x00000001UL){			// M bit
						
						mmuSetTTP(cp15->mmu, (val & 0x00000001UL) ? cp15->ttb : MMU_DISABLED_TTP);
						mmuTlbFlush(cp15->mmu);
						cp15->control ^= 0x00000001UL;
					}
					
				}
			}
			else if(op2 == 1){	//PXA-specific thing
				if(read) val = cp15->ACP;
				else cp15->ACP = val;
			}
			else break;
			goto success;
			
		case 2:		//translation tabler base
			if(read) val = cp15->ttb;
			else{
				if(cp15->control & 0x00000001UL){	//mmu is on
					
					mmuSetTTP(cp15->mmu, val);
					mmuTlbFlush(cp15->mmu);
				}
				cp15->ttb = val;
			}
			goto success;
		
		case 3:		//domain access control
			if(read) val = mmuGetDomainCfg(cp15->mmu);
			else mmuSetDomainCfg(cp15->mmu, val);
			goto success;
		
		case 5:		//FSR
			if(read) val = cp15->FSR;
			else cp15->FSR = val;
			goto success;
			
		case 6:		//FAR
			if(read) val = cp15->FAR;
			else cp15->FAR = val;
			goto success;
		
		case 7:		//cache ops
			if((CRm == 5 || CRm == 7)&& op2 == 0) cpuIcacheInval(cp15->cpu);		//invalidate entire {icache(5) or both i and dcache(7)}
			if((CRm == 5 || CRm == 7) && op2 == 1) cpuIcacheInvalAddr(cp15->cpu, val);	//invalidate {icache(5) or both i and dcache(7)} line, given VA
			if((CRm == 5 || CRm == 7) && op2 == 2) cpuIcacheInval(cp15->cpu);		//invalidate {icache(5) or both i and dcache(7)} line, given set/index. i dont know how to do this, so flush thee whole thing
			goto success;
		
		case 8:		//TLB ops
			mmuTlbFlush(cp15->mmu);
			goto success;
		
		case 9:		//cache lockdown
			if(CRm == 1 && op2 == 0){
				err_str("Attempt to lock 0x");
				err_hex(val);
				err_str("+32 in icache\r\n");	
			}
			else if(CRm == 2 && op2 == 0){
				err_str("Dcache now ");
				err_str(val ? "in" : "out of");
				err_str(" lock mode\r\n");
			}
			goto success;
		
		case 10:	//TLB lockdown
			goto success;
		
		case 13:	//FCSE
			err_str("FCSE not supported\n");
			break;
		
		case 15:
			if(op2 == 0 && CRm == 1){	//CPAR
				if(read) val = cpuGetCPAR(cp15->cpu);
				else cpuSetCPAR(cp15->cpu, val & 0x3FFF);
				goto success;
			}
			break;
	}
	
fail:
	//TODO: cause invalid instruction trap in cpu
	return false;

success:
	
	if(read) cpuSetReg(cpu, Rx, val);
	return true;
}

void cp15Init(ArmCP15* cp15, ArmCpu* cpu, ArmMmu* mmu){

	ArmCoprocessor cp;
	
	cp.regXfer = cp15prvCoprocRegXferFunc;
	cp.dataProcessing = NULL;
	cp.memAccess = NULL;
	cp.twoRegF = NULL;
	cp.userData = cp15;
	
	__mem_zero(cp15, sizeof(ArmCP15));
	cp15->cpu = cpu;
	cp15->mmu = mmu;
	cp15->control = 0x00004072UL;
	
	cpuCoprocessorRegister(cpu, 15, &cp);
}

void cp15Deinit(ArmCP15* cp15){
	
	cpuCoprocessorUnregister(cp15->cpu, 15);
}

void cp15SetFaultStatus(ArmCP15* cp15, UInt32 addr, UInt8 faultStatus){
	
	cp15->FAR = addr;
	cp15->FSR = faultStatus;
}
