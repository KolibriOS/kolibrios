#ifndef _LZMA_ENCODER_H
#define _LZMA_ENCODER_H

#include "lzma.h"
#include "RangeCoderBitTree.h"

typedef struct
{
	CState State;
	bool Prev1IsChar;
	bool Prev2;
	unsigned PosPrev2;
	unsigned BackPrev2;
	unsigned Price;
	unsigned PosPrev;
	unsigned BackPrev;
	unsigned Backs[kNumRepDistances];
} COptimal;
#define COptimal_MakeAsChar(a) (a)->BackPrev=(unsigned)-1,(a)->Prev1IsChar=false
#define COptimal_MakeAsShortRep(a) (a)->BackPrev=0,(a)->Prev1IsChar=false
#define COptimal_IsShortRep(a) ((a)->BackPrev==0)

#define kIfinityPrice 0xFFFFFFF
#define kNumOpts (1<<12)

typedef CMyBitEncoder CLiteralEncoder2[0x300];
typedef struct
{
	CLiteralEncoder2* _coders;
	int _numPrevBits;
	int _numPosBits;
	unsigned _posMask;
} CLiteralEncoder;

typedef struct
{
	CMyBitEncoder _choice;
	CMyBitEncoder _choice2;
	NRangeCoder_CBitTreeEncoder _lowCoder[kNumPosStatesEncodingMax];
	NRangeCoder_CBitTreeEncoder _midCoder[kNumPosStatesEncodingMax];
	NRangeCoder_CBitTreeEncoder _highCoder;
} NLength_CEncoder;

typedef struct
{
	NLength_CEncoder base;
	unsigned _prices[kNumSymbolsTotal][kNumPosStatesEncodingMax];
	unsigned _tableSize;
	unsigned _counters[kNumPosStatesEncodingMax];
} NLength_CPriceTableEncoder;
#define CPriceTableEncoder_Init(a,b) NLength_CEncoder_Init(&a.base,b)

#endif
