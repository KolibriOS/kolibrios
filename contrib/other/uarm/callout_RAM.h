#ifndef _CO_RAM_H_
#define _CO_RAM_H_


#include "types.h"
#include "mem.h"

typedef struct{

	UInt32 adr;
	UInt32 sz;

}CalloutRam;


Boolean coRamInit(CalloutRam* ram, ArmMem* mem, UInt32 adr, UInt32 sz, ArmMemAccessF* coF);
Boolean coRamDeinit(CalloutRam* ram, ArmMem* mem);





#endif

