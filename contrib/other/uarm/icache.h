#ifndef _ICACHE_H_
#define _ICACHE_H_


#include "types.h"
#include "CPU.h"


#define ICACHE_L		4UL	//line size is 2^L bytes
#define ICACHE_S		6UL	//number of sets is 2^S
#define ICACHE_A		6UL	//set associativity


#define ICACHE_LINE_SZ		(1UL << ICACHE_L)
#define ICACHE_BUCKET_NUM	(1UL << ICACHE_S)
#define ICACHE_BUCKET_SZ	(ICACHE_A)


#define ICACHE_ADDR_MASK	((UInt32)-ICACHE_LINE_SZ)
#define ICACHE_USED_MASK	1UL
#define ICACHE_PRIV_MASK	2UL

typedef struct{

	UInt32 info;	//addr, masks
	UInt8 data[ICACHE_LINE_SZ];
	
}icacheLine;

typedef struct{

	struct ArmCpu* cpu;
	ArmCpuMemF memF;
	icacheLine lines[ICACHE_BUCKET_NUM][ICACHE_BUCKET_SZ];
	UInt8 ptr[ICACHE_BUCKET_NUM];

}icache;


void icacheInit(icache* ic, struct ArmCpu* cpu, ArmCpuMemF memF);
Boolean icacheFetch(icache* ic, UInt32 va, UInt8 sz, Boolean priviledged, UInt8* fsr, void* buf);
void icacheInval(icache* ic);
void icacheInvalAddr(icache* ic, UInt32 addr);



#endif
