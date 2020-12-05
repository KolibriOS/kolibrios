#include "math64.h"
#include "pxa255_DSP.h"




Boolean pxa255dspAccess(struct ArmCpu* cpu, void* userData, Boolean MRRC, UInt8 op, UInt8 RdLo, UInt8 RdHi, UInt8 acc){
	
	Pxa255dsp* dsp = userData;
	
	if(acc != 0 || op != 0) return false;				//bad encoding
	
	if(MRRC){	//MRA: read acc0
		
		cpuSetReg(cpu, RdLo, u64_64_to_32(dsp->acc0));
		cpuSetReg(cpu, RdHi, (UInt8)u64_get_hi(dsp->acc0));
	}
	else{		//MAR: write acc0
		
		dsp->acc0 = u64_from_halves(cpuGetRegExternal(cpu, RdHi) & 0xFF, cpuGetRegExternal(cpu, RdLo));
	}
	
	return true;	
}

Boolean	pxa255dspOp(struct ArmCpu* cpu, void* userData, Boolean two/* MCR2/MRC2 ? */, Boolean MRC, UInt8 op1, UInt8 Rs, UInt8 opcode_3, UInt8 Rm, UInt8 acc){
	
	Pxa255dsp* dsp = userData;
	UInt64 addend = u64_zero();
	UInt32 Vs, Vm;
	
	if(op1 != 1 || two || MRC || acc != 0) return false;		//bad encoding
	
	Vs = cpuGetRegExternal(cpu, Rs);
	Vm = cpuGetRegExternal(cpu, Rm);
	
	switch(opcode_3 >> 2){
		
		case 0:	//MIA
			addend = u64_smul3232(Vm, Vs);
			break;
		
		case 1:	//invalid
			return false;
		
		case 2:	//MIAPH
			addend = u64_smul3232((Int32)((Int16)(Vm >>  0)), (Int32)((Int16)(Vs >>  0)));
			addend = u64_add(addend, u64_smul3232((Int32)((Int16)(Vm >> 16)), (Int32)((Int16)(Vs >> 16))));
			break;
		
		case 3:	//MIAxy
			if(opcode_3 & 2) Vm >>= 16;	//X set
			if(opcode_3 & 1) Vs >>= 16;	//Y set
			addend = u64_smul3232((Int32)((Int16)Vm), (Int32)((Int16)Vs));
			break;
	}
	
	dsp->acc0 = u64_and(u64_add(dsp->acc0, addend), u64_from_halves(0xFF, 0xFFFFFFFFUL));
	
	return true;
}



Boolean pxa255dspInit(Pxa255dsp* dsp, ArmCpu* cpu){
	
	ArmCoprocessor cp;
	
	
	__mem_zero(dsp, sizeof(Pxa255dsp));
	
	cp.regXfer = pxa255dspOp;
	cp.dataProcessing = NULL;
	cp.memAccess = NULL;
	cp.twoRegF = pxa255dspAccess;
	cp.userData = dsp;
	
	cpuCoprocessorRegister(cpu, 0, &cp);

	return true;
}