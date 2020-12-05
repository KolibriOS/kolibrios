#include "mem.h"
#include "RAM.h"


	
static Boolean ramAccessF(void* userData, UInt32 pa, UInt8 size, Boolean write, void* bufP){
	
	ArmRam* ram = userData;
	UInt8* addr = (UInt8*)ram->buf;
	
	pa -= ram->adr;
	if(pa >= ram->sz) return false;
	
	addr += pa;
	
	if(write){
		
		switch(size){
			
			case 1:
		
				*((UInt8*)addr) = *(UInt8*)bufP;	//our memory system is little-endian
				break;
			
			case 2:
			
				*((UInt16*)addr) = *(UInt16*)bufP;	//our memory system is little-endian
				break;
			
			case 4:
			
				*((UInt32*)addr) = *(UInt32*)bufP;
				break;
			
			case 8:
			
				*((UInt32*)(addr + 0)) = ((UInt32*)bufP)[0];
				*((UInt32*)(addr + 4)) = ((UInt32*)bufP)[1];
				break;
			
			default:
			
				return false;
		}
	}
	else{
		
		switch(size){
			
			case 1:
				
				*(UInt8*)bufP = *((UInt8*)addr);
				break;
			
			case 2:
			
				*(UInt16*)bufP = *((UInt16*)addr);
				break;
			
			case 4:
			
				*(UInt32*)bufP = *((UInt32*)addr);
				break;
			
			case 64:
				((UInt32*)bufP)[ 8] = *((UInt32*)(addr + 32));
				((UInt32*)bufP)[ 9] = *((UInt32*)(addr + 36));
				((UInt32*)bufP)[10] = *((UInt32*)(addr + 40));
				((UInt32*)bufP)[11] = *((UInt32*)(addr + 44));
				((UInt32*)bufP)[12] = *((UInt32*)(addr + 48));
				((UInt32*)bufP)[13] = *((UInt32*)(addr + 52));
				((UInt32*)bufP)[14] = *((UInt32*)(addr + 56));
				((UInt32*)bufP)[15] = *((UInt32*)(addr + 60));
				//fallthrough
			case 32:
			
				((UInt32*)bufP)[4] = *((UInt32*)(addr + 16));
				((UInt32*)bufP)[5] = *((UInt32*)(addr + 20));
				((UInt32*)bufP)[6] = *((UInt32*)(addr + 24));
				((UInt32*)bufP)[7] = *((UInt32*)(addr + 28));
				//fallthrough
			case 16:
				
				((UInt32*)bufP)[2] = *((UInt32*)(addr +  8));
				((UInt32*)bufP)[3] = *((UInt32*)(addr + 12));
				//fallthrough
			case 8:
				((UInt32*)bufP)[0] = *((UInt32*)(addr +  0));
				((UInt32*)bufP)[1] = *((UInt32*)(addr +  4));
				break;
			
			default:
			
				return false;
		}
	}
	
	return true;
}

Boolean ramInit(ArmRam* ram, ArmMem* mem, UInt32 adr, UInt32 sz, UInt32* buf){

	ram->adr = adr;
	ram->sz = sz;
	ram->buf = buf;
	
	return memRegionAdd(mem, adr, sz, &ramAccessF, ram);	
}

Boolean ramDeinit(ArmRam* ram, ArmMem* mem){
	
	return memRegionDel(mem, ram->adr, ram->sz);
}
