#ifndef _DCACHE_H_
#define _DCACHE_H_


#include "types.h"
#include "CPU.h"


#define DCACHE_L		5UL	//line size is 2^L bytes
#define DCACHE_S		6UL	//number of sets is 2^S
#define DCACHE_A		4UL	//set associativity

#define DCACHE_LINE_SZ		(1UL << ICACHE_L)
#define DCACHE_BUCKET_NUM	(1UL << ICACHE_S)
#define DCACHE_BUCKET_SZ	(ICACHE_A)


#define DCACHE_ADDR_MASK	((UInt32)-ICACHE_LINE_SZ)
#define DCACHE_USED_MASK	1
#define DCACHE_PRIV_MASK	2

typedef struct{

	UInt32 info;	//addr, masks
	UInt8 data[DCACHE_LINE_SZ];
	
}dcacheLine;

typedef struct{

	struct ArmCpu* cpu;
	ArmCpuMemF memF;
	dcacheLine lines[DCACHE_BUCKET_NUM][DCACHE_BUCKET_SZ];
	UInt8 ptr[DCACHE_BUCKET_NUM];

}dcache;


void dcacheInit(dcache* ic, struct ArmCpu* cpu, ArmCpuMemF memF);
Boolean dcacheFetch(dcache* ic, UInt32 va, UInt8 sz, Boolean priviledged, UInt8* fsr, void* buf);
Boolean dcacheWrite(dcache* ic, UInt32 va, UInt8 sz, Boolean priviledged, UInt8* fsr, void* buf);
void dcacheFlush(dcache* ic);
void dcacheFlushAddr(dcache* ic, UInt32 addr);
void dcacheInval(dcache* ic);
void dcacheInvalAddr(dcache* ic, UInt32 addr);



#endif
