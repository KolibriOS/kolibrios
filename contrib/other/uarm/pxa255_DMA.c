#include "pxa255_DMA.h"
#include "mem.h"

#define REG_DAR 	0
#define REG_SAR 	1
#define REG_TAR 	2
#define REG_CR		3
#define REG_CSR		4


static void pxa255dmaPrvChannelRegWrite(_UNUSED_ Pxa255dma* dma, UInt8 channel, UInt8 reg, UInt32 val){
	
	if(val){	//we start with zeros, so non-zero writes are all we care about
		
		const char* regs[] = {"DADDR", "SADDR", "TADDR", "CR", "CSR"};
		
		err_str("dma: writes unimpl!");
	//	err_str("PXA255 dma engine: writes unimpl! (writing 0x");
	//	err_hex(val);
	//	err_str(" to channel ");
	//	err_dec(channel);
	//	err_str(" reg ");
	//	err_str(regs[reg]);
	//	err_str(". Halting.\r\n");
		while(1);	
	}
}

static UInt32 pxa255dmaPrvChannelRegRead(_UNUSED_ Pxa255dma* dma, _UNUSED_ UInt8 channel, _UNUSED_ UInt8 reg){
	
	
	return 0;	
}

static Boolean pxa255dmaPrvMemAccessF(void* userData, UInt32 pa, UInt8 size, Boolean write, void* buf){

	Pxa255dma* dma = userData;
	UInt8 reg, set;
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
	
	pa = (pa - PXA255_DMA_BASE) >> 2;
	
	if(write){
		val = *(UInt32*)buf;
		
		switch(pa >> 6){		//weird, but quick way to avoide repeated if-then-elses. this is faster
			case 0:
				if(pa < 16){
					reg = REG_CSR;
					set = pa;
					pxa255dmaPrvChannelRegWrite(dma, set, reg, val);
				}
				break;
				
			case 1:
				pa -= 64;
				if(pa < 40) dma->CMR[pa] = val;
				break;
			
			case 2:
				pa -= 128;
				set = pa >> 2;
				reg = pa & 3;
				pxa255dmaPrvChannelRegWrite(dma, set, reg, val);
				break;
		}
	}
	else{
		switch(pa >> 6){		//weird, but quick way to avoide repeated if-then-elses. this is faster
			case 0:
				if(pa < 16){
					reg = REG_CSR;
					set = pa;
					val = pxa255dmaPrvChannelRegRead(dma, set, reg);
				}
				break;
				
			case 1:
				pa -= 64;
				if(pa < 40) val = dma->CMR[pa];
				break;
			
			case 2:
				pa -= 128;
				set = pa >> 2;
				reg = pa & 3;
				val = pxa255dmaPrvChannelRegRead(dma, set, reg);
				break;
		}
		
		*(UInt32*)buf = val;
	}
	
	return true;
}


Boolean pxa255dmaInit(Pxa255dma* dma, ArmMem* physMem, Pxa255ic* ic){
	
	__mem_zero(dma, sizeof(Pxa255dma));
	dma->ic = ic;
	dma->mem = physMem;
	
	return memRegionAdd(physMem, PXA255_DMA_BASE, PXA255_DMA_SIZE, pxa255dmaPrvMemAccessF, dma);
}
