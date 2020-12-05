#include "pxa255_IC.h"
#include "mem.h"


static void pxa255icPrvHandleChanges(Pxa255ic* ic){

	Boolean nowIrq, nowFiq;
	UInt32 unmasked = ic->ICPR & ic->ICMR;
	
	nowFiq = (unmasked & ic->ICLR) != 0;
	nowIrq = (unmasked & ~ic->ICLR) != 0;
	
	if(nowFiq != ic->wasFiq) cpuIrq(ic->cpu, true, nowFiq);
	if(nowIrq != ic->wasIrq) cpuIrq(ic->cpu, false, nowIrq);

	ic->wasFiq = nowFiq;
	ic->wasIrq = nowIrq;
}

static Boolean pxa255icPrvMemAccessF(void* userData, UInt32 pa, UInt8 size, Boolean write, void* buf){
	
	Pxa255ic* ic = userData;
	UInt32 val = 0;
	
	if(size != 4) {
		err_str(__FILE__ ": Unexpected ");
	//	err_str(write ? "write" : "read");
	//	err_str(" of ");
	//	err_dec(size);
	//	err_str(" bytes to 0x");
	//	err_hex(pa);
	//	err_str("\r\n");
		return true;		//we do not support non-word accesses
	}
	
	pa = (pa - PXA255_IC_BASE) >> 2;
	
	if(write){
		if(pa == 1) ic->ICMR = *(UInt32*)buf;
		else if(pa == 2) ic->ICLR = *(UInt32*)buf;
		else if(pa == 5) ic->ICCR = *(UInt32*)buf;
		else return true;
		pxa255icPrvHandleChanges(ic);
	}
	else{
		switch(pa){
			
			case 0:
				val = ic->ICPR & ic->ICMR & ~ic->ICLR;
				break;
			
			case 1:
				val = ic->ICMR;
				break;
			
			case 2:
				val = ic->ICLR;
				break;
			
			case 3:
				val = ic->ICPR & ic->ICMR & ic->ICLR;
				break;
			
			case 4:
				val = ic->ICPR;
				break;
			
			case 5:
				val = ic->ICCR;
				break;
			
		}
		*(UInt32*)buf = val;
	}
	return true;
}

Boolean pxa255icInit(Pxa255ic* ic, ArmCpu* cpu, ArmMem* physMem){
	
	__mem_zero(ic, sizeof(Pxa255ic));
	ic->cpu = cpu;
	return memRegionAdd(physMem, PXA255_IC_BASE, PXA255_IC_SIZE, pxa255icPrvMemAccessF, ic);
}


void pxa255icInt(Pxa255ic* ic, UInt8 intNum, Boolean raise){		//interrupt caused by emulated hardware
	
	UInt32 old_, new_;
	
	old_ = new_ = ic->ICPR;
	
	if(raise) new_ |= (1UL << intNum);
	else new_ &=~ (1UL << intNum);
	
	if(new_ != old_){
		ic->ICPR = new_;
		pxa255icPrvHandleChanges(ic);
	}
}





