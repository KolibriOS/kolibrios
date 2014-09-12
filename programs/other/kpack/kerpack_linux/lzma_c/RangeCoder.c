#include "RangeCoderBitTree.h"
#include "lzma.h"

static unsigned _cacheSize;
static byte _cache;
static uint64 low;
static unsigned range;

static unsigned PriceTable[kBitModelTotal >> kNumMoveReducingBits];

void RangeEncoder_Init(void)
{
	int i;
	unsigned start,end,j;
	low = 0;
	range = 0xFFFFFFFF;
	_cacheSize = 1;
	_cache = 0;
	/* init price table */
#define kNumBits (kNumBitModelTotalBits - kNumMoveReducingBits)
	for (i=kNumBits;i--;)
	{
		start = 1 << (kNumBits - i - 1);
		end = 1 << (kNumBits - i);
		for (j=start;j<end;j++)
			PriceTable[j] = (i<<kNumBitPriceShiftBits) +
			(((end-j)<<kNumBitPriceShiftBits) >> (kNumBits - i - 1));
	}
#undef kNumBits
}

void RangeEncoder_ShiftLow(void)
{
	if ((unsigned)low < 0xFF000000U || (int)(low>>32))
	{
		byte temp = _cache;
		do
		{
			*curout++ = (byte)(temp + (byte)(low>>32));
			temp = 0xFF;
		} while (--_cacheSize);
		_cache = (byte)((unsigned)low>>24);
	}
	_cacheSize++;
	low = (unsigned)low << 8;
}

void RangeEncoder_FlushData(void)
{
	int i;
	for (i=0;i<5;i++)
		RangeEncoder_ShiftLow();
}

void RangeEncoder_EncodeDirectBits(unsigned value,int numTotalBits)
{
	int i;
	for (i=numTotalBits;i--;)
	{
		range >>= 1;
		if (((value >> i) & 1) == 1)
			low += range;
		if (range < kTopValue)
		{
			range <<= 8;
			RangeEncoder_ShiftLow();
		}
	}
}

void CMyBitEncoder_Encode(CMyBitEncoder* e,unsigned symbol)
{
	unsigned newBound;
	newBound = (range >> kNumBitModelTotalBits) * *e;
	if (symbol == 0)
	{
		range = newBound;
		*e += (kBitModelTotal - *e) >> kNumMoveBits;
	}
	else
	{
		low += newBound;
		range -= newBound;
		*e -= *e >> kNumMoveBits;
	}
	if (range < kTopValue)
	{
		range <<= 8;
		RangeEncoder_ShiftLow();
	}
}

unsigned CMyBitEncoder_GetPrice(CMyBitEncoder* e, unsigned symbol)
{
	return PriceTable[(((*e-symbol)^((-(int)symbol))) & (kBitModelTotal-1)) >> kNumMoveReducingBits];
}
unsigned CMyBitEncoder_GetPrice0(CMyBitEncoder* e)
{
	return PriceTable[*e >> kNumMoveReducingBits];
}
unsigned CMyBitEncoder_GetPrice1(CMyBitEncoder* e)
{
	return PriceTable[(kBitModelTotal - *e) >> kNumMoveReducingBits];
}

void CBitTreeEncoder_Init(NRangeCoder_CBitTreeEncoder*e,int numBitLevels)
{
	unsigned i;
	e->numBitLevels = numBitLevels;
	for (i=1;i<((unsigned)1<<numBitLevels);i++)
		CMyBitEncoder_Init(e->Models[i]);
}
void CBitTreeEncoder_Encode(NRangeCoder_CBitTreeEncoder*e,unsigned symbol)
{
	unsigned modelIndex = 1;
	int bitIndex;
	unsigned bit;
	for (bitIndex = e->numBitLevels; bitIndex--;)
	{
		bit = (symbol >> bitIndex) & 1;
		CMyBitEncoder_Encode(&e->Models[modelIndex],bit);
		modelIndex = (modelIndex << 1) | bit;
	}
}
void CBitTreeEncoder_ReverseEncode(NRangeCoder_CBitTreeEncoder*e,unsigned symbol)
{
	unsigned modelIndex = 1;
	int i;
	unsigned bit;
	for (i=0;i<e->numBitLevels;i++)
	{
		bit = symbol & 1;
		CMyBitEncoder_Encode(&e->Models[modelIndex],bit);
		modelIndex = (modelIndex << 1) | bit;
		symbol >>= 1;
	}
}
unsigned CBitTreeEncoder_GetPrice(NRangeCoder_CBitTreeEncoder*e,unsigned symbol)
{
	unsigned price = 0;
	symbol |= (1 << e->numBitLevels);
	while (symbol != 1)
	{
		price += CMyBitEncoder_GetPrice(&e->Models[symbol>>1],symbol&1);
		symbol >>= 1;
	}
	return price;
}
unsigned CBitTreeEncoder_ReverseGetPrice(NRangeCoder_CBitTreeEncoder*e,unsigned symbol)
{
	unsigned price=0;
	unsigned modelIndex=1;
	int i;
	unsigned bit;
	for (i=e->numBitLevels;i;i--)
	{
		bit = symbol&1;
		symbol >>= 1;
		price += CMyBitEncoder_GetPrice(&e->Models[modelIndex],bit);
		modelIndex = (modelIndex<<1)|bit;
	}
	return price;
}
unsigned ReverseBitTreeGetPrice(CMyBitEncoder*Models,unsigned NumBitLevels,unsigned symbol)
{
	unsigned price=0;
	unsigned modelIndex=1;
	unsigned bit;
	int i;
	for (i=NumBitLevels;i;i--)
	{
		bit = symbol & 1;
		symbol >>= 1;
		price += CMyBitEncoder_GetPrice(Models+modelIndex,bit);
		modelIndex = (modelIndex<<1)|bit;
	}
	return price;
}
void ReverseBitTreeEncode(CMyBitEncoder*Models,int NumBitLevels,unsigned symbol)
{
	unsigned modelIndex = 1;
	int i;
	unsigned bit;
	for (i=0;i<NumBitLevels;i++)
	{
		bit = symbol & 1;
		CMyBitEncoder_Encode(Models+modelIndex,bit);
		modelIndex = (modelIndex<<1)|bit;
		symbol >>= 1;
	}
}
