#include "mem.h"



void memInit(ArmMem* mem){
	
	UInt8 i;
	
	for(i = 0; i < MAX_MEM_REGIONS; i++){
		mem->regions[i].sz = 0;
	}
}


void memDeinit(_UNUSED_ ArmMem* mem){
	
	//nothing here
}

Boolean memRegionAdd(ArmMem* mem, UInt32 pa, UInt32 sz, ArmMemAccessF aF, void* uD){

	UInt8 i;
	
	//check for intersection with another region
	
	for(i = 0; i < MAX_MEM_REGIONS; i++){
		
		if(!mem->regions[i].sz) continue;
		if((mem->regions[i].pa <= pa && mem->regions[i].pa + mem->regions[i].sz > pa) || (pa <= mem->regions[i].pa && pa + sz > mem->regions[i].pa)){
		
			return false;		//intersection -> fail
		}
	}
	
	
	//find a free region and put it there
	
	for(i = 0; i < MAX_MEM_REGIONS; i++){
		if(mem->regions[i].sz == 0){
		
			mem->regions[i].pa = pa;
			mem->regions[i].sz = sz;
			mem->regions[i].aF = aF;
			mem->regions[i].uD = uD;
		
			return true;
		}
	}
	
	
	//fail miserably
	
	return false;	
}

Boolean memRegionDel(ArmMem* mem, UInt32 pa, UInt32 sz){

	UInt8 i;
	
	for(i = 0; i < MAX_MEM_REGIONS; i++){
		if(mem->regions[i].pa == pa && mem->regions[i].sz ==sz){
		
			mem->regions[i].sz = 0;
			return true;
		}
	}
	
	return false;
}

Boolean memAccess(ArmMem* mem, UInt32 addr, UInt8 size, Boolean write, void* buf){
	
	UInt8 i;
	
	for(i = 0; i < MAX_MEM_REGIONS; i++){
		if(mem->regions[i].pa <= addr && mem->regions[i].pa + mem->regions[i].sz > addr){
		
			return mem->regions[i].aF(mem->regions[i].uD, addr, size, write & 0x7F, buf);
		}
	}
	
	if(!(write & 0x80)){	//high bit in write tells us to not print this error (used by gdb stub)
		
		err_str("Memory ");
		err_str(write ? "write" : "read");
		err_str(" of ");
		err_dec(size);
		err_str(" bytes at physical addr 0x");
		err_hex(addr);
		err_str(" fails, halting\r\n");
		while(1);
	}
	
	return false;
}

