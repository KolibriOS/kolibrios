#include "mem.h"
#include "callout_RAM.h"


Boolean coRamInit(CalloutRam* ram, ArmMem* mem, UInt32 adr, UInt32 sz, ArmMemAccessF* coF){

	ram->adr = adr;
	ram->sz = sz;
	
	return memRegionAdd(mem, adr, sz, (void*)coF, ram);	
}

Boolean coRamDeinit(CalloutRam* ram, ArmMem* mem){
	
	return memRegionDel(mem, ram->adr, ram->sz);
}
