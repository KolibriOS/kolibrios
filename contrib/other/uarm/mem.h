#ifndef _MEM_H_
#define _MEM_H_

#include "types.h"

#define MAX_MEM_REGIONS		16

#define errPhysMemNoSuchRegion	(errPhysMem + 1)		//this physical address is not claimed by any region
#define errPhysMemInvalidAdr	(errPhysMem + 2)		//address is IN a region but access to it is not allowed (it doesn't exist really)
#define errPhysMemInvalidSize	(errPhysMem + 3)		//access that is not 1, 2 or 4-byte big

typedef Boolean (*ArmMemAccessF)(void* userData, UInt32 pa, UInt8 size, Boolean write, void* buf);

typedef struct{

	UInt32 pa;
	UInt32 sz;
	ArmMemAccessF aF;
	void* uD;

}ArmMemRegion;

typedef struct{

	ArmMemRegion regions[MAX_MEM_REGIONS];

}ArmMem;


void memInit(ArmMem* mem);
void memDeinit(ArmMem* mem);

Boolean memRegionAdd(ArmMem* mem, UInt32 pa, UInt32 sz, ArmMemAccessF af, void* uD);
Boolean memRegionDel(ArmMem* mem, UInt32 pa, UInt32 sz);

Boolean memAccess(ArmMem* mem, UInt32 addr, UInt8 size, Boolean write, void* buf);

#endif
