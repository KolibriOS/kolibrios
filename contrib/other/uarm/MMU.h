#ifndef _MMU_H_
#define _MMU_H_


#include "types.h"


#define MMU_TLB_BUCKET_SIZE	8
#define MMU_TLB_BUCKET_NUM	32
#define MMU_DISABLED_TTP	0xFFFFFFFFUL


typedef Err (*ArmMmuReadF)(void* userData, UInt32* buf, UInt32 pa);	//read a word

#define errMmuTranslation		(errMmu + 1)
#define	errMmuDomain			(errMmu + 2)
#define errMmuPermission		(errMmu + 3)

typedef struct {
	
	UInt32 pa, va;
	UInt32 sz;
	UInt32 ap:2;
	UInt32 domain:4;
	
}ArmPrvTlb;

typedef struct ArmMmu{

	UInt32 transTablPA;
	UInt8 S:1;
	UInt8 R:1;
	UInt8 readPos[MMU_TLB_BUCKET_NUM];
	UInt8 replPos[MMU_TLB_BUCKET_NUM];
	ArmPrvTlb tlb[MMU_TLB_BUCKET_NUM][MMU_TLB_BUCKET_SIZE];
	UInt32 domainCfg;
	ArmMmuReadF readF;
	void* userData;

}ArmMmu;


void mmuInit(ArmMmu* mmu, ArmMmuReadF readF, void* userData);
void muDeinit(ArmMmu* mmu);
Boolean mmuTranslate(ArmMmu* mmu, UInt32 va, Boolean priviledged, Boolean write, UInt32* paP, UInt8* fsrP);

UInt32 mmuGetTTP(ArmMmu* mmu);
void mmuSetTTP(ArmMmu* mmu, UInt32 ttp);

void mmuSetS(ArmMmu* mmu, Boolean on);
void mmuSetR(ArmMmu* mmu, Boolean on);
Boolean mmuGetS(ArmMmu* mmu);
Boolean mmuGetR(ArmMmu* mmu);

UInt32 mmuGetDomainCfg(ArmMmu* mmu);
void mmuSetDomainCfg(ArmMmu* mmu, UInt32 val);

void mmuTlbFlush(ArmMmu* mmu);




#endif

