#include "RangeCoderBit.h"

typedef struct
{
	CMyBitEncoder Models[1<<8];
	int numBitLevels;
} NRangeCoder_CBitTreeEncoder;

extern void CBitTreeEncoder_Init(NRangeCoder_CBitTreeEncoder*e,int numBitLevels);
extern void CBitTreeEncoder_Encode(NRangeCoder_CBitTreeEncoder*e,unsigned symbol);
extern void CBitTreeEncoder_ReverseEncode(NRangeCoder_CBitTreeEncoder*e,unsigned symbol);
extern unsigned CBitTreeEncoder_GetPrice(NRangeCoder_CBitTreeEncoder*e,unsigned symbol);
extern unsigned CBitTreeEncoder_ReverseGetPrice(NRangeCoder_CBitTreeEncoder*e,unsigned symbol);
extern unsigned ReverseBitTreeGetPrice(CMyBitEncoder*Models,unsigned NumBitLevels,unsigned symbol);
extern void ReverseBitTreeEncode(CMyBitEncoder*Models,int NumBitLevels,unsigned symbol);
