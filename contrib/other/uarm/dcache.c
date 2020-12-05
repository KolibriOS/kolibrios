#include "types.h"
#include "CPU.h"
#include "dcache.h"

//#define DCACHE_DEBUGGING



#ifdef DCACHE_DEBUGGING
	#define _dcache_fetch_func	dcacheFetch_
	#define _dcache_test_func	dcacheFetch
#else
	#define _dcache_fetch_func	dcacheFetch
	#define _dcache_test_func	dcacheFetch_test
#endif

void dcacheInval(dcache* dc){

	UInt8 i, j;
	
	for(i = 0; i < DCACHE_BUCKET_NUM; i++){
		for(j = 0; j < DCACHE_BUCKET_SZ; j++) dc->lines[i][j].info = 0;
		dc->ptr[i] = 0;
	}
}

void dcacheInit(dcache* dc, ArmCpu* cpu, ArmCpuMemF memF){

	dc->cpu = cpu;
	dc->memF = memF;
	
	dcacheInval(dc);	
}


static UInt8 dcachePrvHash(UInt32 addr){

	addr >>= DCACHE_L;
	addr &= (1UL << DCACHE_S) - 1UL;

	return addr;
}

void dcacheInvalAddr(dcache* dc, UInt32 va){

	UInt32 off = va % DCACHE_LINE_SZ;
	Int8 i, j, bucket;
	dcacheLine* lines;
	
	va -= off;

	bucket = dcachePrvHash(va);
	lines = dc->lines[bucket];
	
	for(i = 0, j = dc->ptr[bucket]; i < DCACHE_BUCKET_SZ; i++){
		
		if(--j == -1) j = DCACHE_BUCKET_SZ - 1;
		
		if((lines[j].info & (DCACHE_ADDR_MASK | DCACHE_USED_MASK)) == (va | DCACHE_USED_MASK)){	//found it!
		
			lines[j].info = 0;
		}
	}
}

void dcacheFlush(dcache* dc){

		
}

void dcacheFlushAddr(dcache* dc, UInt32 va){
	
	
}

/*
	we cannot have data overlap cachelines since data is self aligned (word on 4-byte boundary, halfwords on2, etc. this is enforced elsewhere
*/

Boolean dcacheWrite(dcache* dc, UInt32 va, UInt8 sz, Boolean priviledged, UInt8* fsrP, void* buf){

	
}

Boolean _dcache_fetch_func(dcache* dc, UInt32 va, UInt8 sz, Boolean priviledged, UInt8* fsrP, void* buf){

	UInt32 off = va % DCACHE_LINE_SZ;
	Int8 i, j, bucket;
	dcacheLine* lines;
	dcacheLine* line;
	
	va -= off;

	bucket = dcachePrvHash(va);
	lines = dc->lines[bucket];
	
	for(i = 0, j = dc->ptr[bucket]; i < DCACHE_BUCKET_SZ; i++){
		
		if(--j == -1) j = DCACHE_BUCKET_SZ - 1;
		
		if((lines[j].info & (DCACHE_ADDR_MASK | DCACHE_USED_MASK)) == (va | DCACHE_USED_MASK)){	//found it!
		
			if(sz == 4){
				*(UInt32*)buf = *(UInt32*)(lines[j].data + off);
			}
			else if(sz == 2){
				*(UInt16*)buf = *(UInt16*)(lines[j].data + off);
			}
			else __mem_copy(buf, lines[j].data + off, sz);
			return priviledged || !(lines[j].info & DCACHE_PRIV_MASK);	
		}
	}
	//if we're here, we found nothing - time to populate the cache
	j = dc->ptr[bucket]++;
	if(dc->ptr[bucket] == DCACHE_BUCKET_SZ) dc->ptr[bucket] = 0;
	line = lines + j;
	
	line->info = va | (priviledged ? DCACHE_PRIV_MASK : 0);
	if(!dc->memF(dc->cpu, line->data, va, DCACHE_LINE_SZ, false, priviledged, fsrP)){
	
		return false;	
	}
	line->info |= DCACHE_USED_MASK;
	
	if(sz == 4){
		*(UInt32*)buf = *(UInt32*)(line->data + off);
	}
	else if(sz == 2){
		*(UInt16*)buf = *(UInt16*)(line->data + off);
	}
	else __mem_copy(buf, line->data + off, sz);
	return true;
}

#include "stdio.h"
Boolean _dcache_test_func(dcache* dc, UInt32 va, UInt8 sz, Boolean priviledged, UInt8* fsrP, void* buf){

	UInt8 fsrO = -1, fsrT = -1;
	UInt8 dataO[4] = {0}, dataT[4] = {0};
	Boolean retO, retT;
	UInt8 i;
	
	retO = _dcache_fetch_func(dc, va, sz, priviledged, &fsrO, dataO);
	retT = dc->memF(dc->cpu, dataT, va, sz, false, priviledged, &fsrT);
	
	if((retT != retO) || (fsrT != fsrO) || (dataT[0] != dataO[0]) || (dataT[1] != dataO[1]) || (dataT[2] != dataO[2]) || (dataT[3] != dataO[3])){
	
		fprintf(stderr, "dcache fail!");	
	}

	for(i = 0; i < sz; i++) ((UInt8*)buf)[i] = dataT[i];
	if(retT) *fsrP = fsrT;
	return retT;
}

