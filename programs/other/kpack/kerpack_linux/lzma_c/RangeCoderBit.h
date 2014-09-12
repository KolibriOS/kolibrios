#include "RangeCoder.h"

#define kNumBitModelTotalBits 11
#define kBitModelTotal (1<<kNumBitModelTotalBits)

#define kNumMoveReducingBits 2

#define kNumBitPriceShiftBits 6
#define kBitPrice (1<<kNumBitPriceShiftBits)

typedef unsigned NRangeCoder_CBitModel;
typedef NRangeCoder_CBitModel CMyBitEncoder;

extern void CMyBitEncoder_Encode(CMyBitEncoder* e,unsigned symbol);
extern unsigned CMyBitEncoder_GetPrice(CMyBitEncoder* e, unsigned symbol);
extern unsigned CMyBitEncoder_GetPrice0(CMyBitEncoder* e);
extern unsigned CMyBitEncoder_GetPrice1(CMyBitEncoder* e);
#define CMyBitEncoder_Init(a) a=kBitModelTotal/2
