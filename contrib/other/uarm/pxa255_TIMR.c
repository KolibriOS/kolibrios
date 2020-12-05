#include "pxa255_TIMR.h"
#include "mem.h"


static void pxa255timrPrvRaiseLowerInts(Pxa255timr* timr){
	
	pxa255icInt(timr->ic, PXA255_I_TIMR0, (timr->OSSR & 1) != 0);
	pxa255icInt(timr->ic, PXA255_I_TIMR1, (timr->OSSR & 2) != 0);
	pxa255icInt(timr->ic, PXA255_I_TIMR2, (timr->OSSR & 4) != 0);
	pxa255icInt(timr->ic, PXA255_I_TIMR3, (timr->OSSR & 8) != 0);
}

static void pxa255timrPrvCheckMatch(Pxa255timr* timr, UInt8 idx){
	
	UInt8 v = 1UL << idx;
	
	if((timr->OSCR == timr->OSMR[idx]) && (timr->OIER & v)){
		timr->OSSR |= v;
	}
}

static void pxa255timrPrvUpdate(Pxa255timr* timr){
	
	pxa255timrPrvCheckMatch(timr, 0);
	pxa255timrPrvCheckMatch(timr, 1);
	pxa255timrPrvCheckMatch(timr, 2);
	pxa255timrPrvCheckMatch(timr, 3);
}

static Boolean pxa255timrPrvMemAccessF(void* userData, UInt32 pa, UInt8 size, Boolean write, void* buf){

	Pxa255timr* timr = userData;
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
	
	pa = (pa - PXA255_TIMR_BASE) >> 2;
	
	if(write){
		val = *(UInt32*)buf;
		
		switch(pa){
			case 0:
			case 1:
			case 2:
			case 3:
				timr->OSMR[pa] = val;
				break;
			
			case 4:
				timr->OSCR = val;
				break;
			
			case 5:
				timr->OSSR = timr->OSSR &~ val;
				pxa255timrPrvRaiseLowerInts(timr);
				break;
			
			case 6:
				timr->OWER = val;
				break;
			
			case 7:
				timr->OIER = val;
				pxa255timrPrvUpdate(timr);
				pxa255timrPrvRaiseLowerInts(timr);
				break;
		}
	}
	else{
		switch(pa){
			case 0:
			case 1:
			case 2:
			case 3:
				val = timr->OSMR[pa];
				break;
			
			case 4:
				val = timr->OSCR;
				break;
			
			case 5:
				val = timr->OSSR;
				break;
			
			case 6:
				val = timr->OWER;
				break;
			
			case 7:
				val = timr->OIER;
				break;
		}
		*(UInt32*)buf = val;
	}
	
	return true;
}


Boolean pxa255timrInit(Pxa255timr* timr, ArmMem* physMem, Pxa255ic* ic){
	
	__mem_zero(timr, sizeof(Pxa255timr));
	timr->ic = ic;
	return memRegionAdd(physMem, PXA255_TIMR_BASE, PXA255_TIMR_SIZE, pxa255timrPrvMemAccessF, timr);
}

void pxa255timrTick(Pxa255timr* timr){
	
	timr->OSCR++;
	pxa255timrPrvUpdate(timr);
	pxa255timrPrvRaiseLowerInts(timr);
}
