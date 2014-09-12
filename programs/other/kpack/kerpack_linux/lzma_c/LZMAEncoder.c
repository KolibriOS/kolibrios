#include "LZMAEncoder.h"
#include "MatchFinder.h"

const byte kLiteralNextStates[kNumStates] = {0,0,0,0,1,2,3,4,5,6,4,5};
const byte kMatchNextStates[kNumStates] = {7,7,7,7,7,7,7,10,10,10,10,10};
const byte kRepNextStates[kNumStates] = {8,8,8,8,8,8,8,11,11,11,11,11};
const byte kShortRepNextStates[kNumStates] = {9,9,9,9,9,9,9,11,11,11,11,11};

static CState _state;
static byte _previousByte;
static unsigned _repDistances[kNumRepDistances];

static COptimal _optimum[kNumOpts];
static CMyBitEncoder _isMatch[kNumStates][kNumPosStatesEncodingMax];
static CMyBitEncoder _isRep[kNumStates];
static CMyBitEncoder _isRepG0[kNumStates];
static CMyBitEncoder _isRepG1[kNumStates];
static CMyBitEncoder _isRepG2[kNumStates];
static CMyBitEncoder _isRep0Long[kNumStates][kNumPosStatesEncodingMax];
static NRangeCoder_CBitTreeEncoder _posSlotEncoder[kNumLenToPosStates];
static CMyBitEncoder _posEncoders[kNumFullDistances - kEndPosModelIndex];
static NRangeCoder_CBitTreeEncoder _posAlignEncoder;
static NLength_CPriceTableEncoder _lenEncoder;
static NLength_CPriceTableEncoder _repMatchLenEncoder;
static CLiteralEncoder _literalEncoder;
static unsigned _matchDistances[kMatchMaxLen+1];
static unsigned _numFastBytes;
static unsigned _longestMatchLength;
static unsigned _additionalOffset;
static unsigned _optimumEndIndex;
static unsigned _optimumCurrentIndex;
static bool _longestMatchWasFound;
static unsigned _posSlotPrices[kNumLenToPosStates][kDistTableSizeMax];
static unsigned _distancesPrices[kNumLenToPosStates][kNumFullDistances];
static unsigned _alignPrices[kAlignTableSize];
static unsigned _alignPriceCount;
static unsigned _distTableSize;
static unsigned _posStateBits;
static unsigned _posStateMask;
static unsigned _numLiteralPosStateBits;
static unsigned _numLiteralContextBits;
static unsigned _dictionarySize;
static uint64 lastPosSlotFillingPos;
static uint64 nowPos64;
static bool _finished;
static bool _writeEndMark;

static byte g_FastPos[1024];

// must be called before work
static void FastPosInit(void)
{
	int c = 2;
	int slotFast;
	unsigned j,k;
	g_FastPos[0] = 0;
	g_FastPos[1] = 1;
	for (slotFast = 2; slotFast < 20; slotFast++)
	{
		k = (1 << ((slotFast >> 1) - 1));
		for (j=0;j<k;j++) g_FastPos[c++] = slotFast;
	}
}
static unsigned GetPosSlot(unsigned pos)
{
	if (pos < (1<<10))
		return g_FastPos[pos];
	if (pos < (1<<19))
		return g_FastPos[pos>>9]+18;
	return g_FastPos[pos>>18]+36;
}
static unsigned GetPosSlot2(unsigned pos)
{
	if (pos < (1<<16))
		return g_FastPos[pos>>6]+12;
	if (pos < (1<<25))
		return g_FastPos[pos>>15]+30;
	return g_FastPos[pos>>24]+48;
}

unsigned pack_length;
unsigned pack_pos;
const byte* curin;
byte* curout;

static void NLength_CEncoder_Init(NLength_CEncoder*e, unsigned numPosStates)
{
	unsigned posState;
	CMyBitEncoder_Init(e->_choice);
	CMyBitEncoder_Init(e->_choice2);
	for (posState=0;posState<numPosStates;posState++)
	{
		CBitTreeEncoder_Init(&e->_lowCoder[posState],kNumLowBits);
		CBitTreeEncoder_Init(&e->_midCoder[posState],kNumMidBits);
	}
	CBitTreeEncoder_Init(&e->_highCoder,kNumHighBits);
}

static void NLength_CEncoder_Encode(NLength_CEncoder*e, unsigned symbol, unsigned posState)
{
	if (symbol < kNumLowSymbols)
	{
		CMyBitEncoder_Encode(&e->_choice,0);
		CBitTreeEncoder_Encode(&e->_lowCoder[posState],symbol);
	}
	else
	{
		CMyBitEncoder_Encode(&e->_choice,1);
		if (symbol < kNumLowSymbols + kNumMidSymbols)
		{
			CMyBitEncoder_Encode(&e->_choice2,0);
			CBitTreeEncoder_Encode(&e->_midCoder[posState],symbol-kNumLowSymbols);
		}
		else
		{
			CMyBitEncoder_Encode(&e->_choice2,1);
			CBitTreeEncoder_Encode(&e->_highCoder,symbol-kNumLowSymbols-kNumMidSymbols);
		}
	}
}

static unsigned NLength_CEncoder_GetPrice(NLength_CEncoder*e, unsigned symbol, unsigned posState)
{
	unsigned price;
	if (symbol < kNumLowSymbols)
		return CMyBitEncoder_GetPrice0(&e->_choice) +
			CBitTreeEncoder_GetPrice(&e->_lowCoder[posState],symbol);
	price = CMyBitEncoder_GetPrice1(&e->_choice);
	if (symbol < kNumLowSymbols + kNumMidSymbols)
	{
		price += CMyBitEncoder_GetPrice0(&e->_choice2);
		price += CBitTreeEncoder_GetPrice(&e->_midCoder[posState],symbol-kNumLowSymbols);
	}
	else
	{
		price += CMyBitEncoder_GetPrice1(&e->_choice2);
		price += CBitTreeEncoder_GetPrice(&e->_highCoder,symbol-kNumLowSymbols-kNumMidSymbols);
	}
	return price;
}

static void CPriceTableEncoder_SetTableSize(NLength_CPriceTableEncoder*pte,unsigned tableSize)
{pte->_tableSize = tableSize;}
static unsigned CPriceTableEncoder_GetPrice(NLength_CPriceTableEncoder*pte,unsigned symbol,unsigned posState)
{return pte->_prices[symbol][posState];}
static void CPriceTableEncoder_UpdateTable(NLength_CPriceTableEncoder*pte,unsigned posState)
{
	unsigned len;
	for (len=0;len<pte->_tableSize;len++)
		pte->_prices[len][posState] = NLength_CEncoder_GetPrice(&pte->base,len,posState);
	pte->_counters[posState] = pte->_tableSize;
}
static void CPriceTableEncoder_UpdateTables(NLength_CPriceTableEncoder*pte,unsigned numPosStates)
{
	unsigned posState;
	for (posState=0;posState<numPosStates;posState++)
		CPriceTableEncoder_UpdateTable(pte,posState);
}
static void CPriceTableEncoder_Encode(NLength_CPriceTableEncoder*pte, unsigned symbol, unsigned posState)
{
	NLength_CEncoder_Encode(&pte->base,symbol,posState);
	if (--pte->_counters[posState] == 0)
		CPriceTableEncoder_UpdateTable(pte,posState);
}

static void CBaseState_Init(void)
{
	unsigned i;
	CState_Init(_state);
	_previousByte = 0;
	for (i=0;i<kNumRepDistances;i++)
		_repDistances[i] = 0;
}

static void CLiteralEncoder2_Init(CLiteralEncoder2 le)
{
	int i;
	for (i=0;i<0x300;i++)
		CMyBitEncoder_Init(le[i]);
}

static void CLiteralEncoder2_Encode(CLiteralEncoder2 le, byte symbol)
{
	unsigned context = 1;
	int i;
	unsigned bit;
	for (i=8;i--;)
	{
		bit = (symbol >> i) & 1;
		CMyBitEncoder_Encode(&le[context],bit);
		context = (context << 1) | bit;
	}
}

static void CLiteralEncoder2_EncodeMatched(CLiteralEncoder2 le, byte matchByte, byte symbol)
{
	unsigned context = 1;
	int i;
	unsigned bit,matchBit;
	for (i=8;i--;)
	{
		bit = (symbol >> i) & 1;
		matchBit = (matchByte >> i) & 1;
		CMyBitEncoder_Encode(&le[0x100 + (matchBit<<8) + context],bit);
		context = (context << 1) | bit;
		if (matchBit != bit)
		{
			while (i--)
			{
				bit = (symbol >> i) & 1;
				CMyBitEncoder_Encode(&le[context],bit);
				context = (context << 1) | bit;
			}
			break;
		}
	}
}

static unsigned CLiteralEncoder2_GetPrice(CLiteralEncoder2 le, bool matchMode, byte matchByte, byte symbol)
{
	unsigned price = 0;
	unsigned context = 1;
	unsigned bit,matchBit;
	int i = 8;
	if (matchMode)
	{
		do
		{
			i--;
			matchBit = (matchByte >> i) & 1;
			bit = (symbol >> i) & 1;
			price += CMyBitEncoder_GetPrice(&le[0x100 + (matchBit<<8) + context],bit);
			context = (context << 1) | bit;
			if (matchBit != bit)
				break;
		} while (i);
	}
	while (i--)
	{
		bit = (symbol >> i) & 1;
		price += CMyBitEncoder_GetPrice(&le[context],bit);
		context = (context << 1) | bit;
	}
	return price;
}

static void WriteEndMarker(unsigned posState)
{
	unsigned posSlot;
	if (!_writeEndMark)
		return;
	CMyBitEncoder_Encode(&_isMatch[_state][posState],1);
	CMyBitEncoder_Encode(&_isRep[_state],0);
	CState_UpdateMatch(_state);
	CPriceTableEncoder_Encode(&_lenEncoder,0,posState);
	posSlot = (1<<kNumPosSlotBits) - 1;
	CBitTreeEncoder_Encode(&_posSlotEncoder[GetLenToPosState(kMatchMinLen)],posSlot);
	RangeEncoder_EncodeDirectBits(((1<<30)-1)>>kNumAlignBits,30-kNumAlignBits);
	CBitTreeEncoder_ReverseEncode(&_posAlignEncoder,((1<<30)-1) & kAlignMask);
}

static void CEncoder_Flush(void)
{
	WriteEndMarker((unsigned)nowPos64 & _posStateMask);
	RangeEncoder_FlushData();
}

static void CLiteralEncoder_Create(CLiteralEncoder*le, byte** memory, int numPosBits, int numPrevBits)
{
	unsigned numStates;
	le->_coders = (CLiteralEncoder2*)*memory;
	numStates = 1 << (numPosBits+numPrevBits);
	*memory = (byte*)(le->_coders + numStates);
	le->_numPosBits = numPosBits;
	le->_posMask = (1<<numPosBits) - 1;
	le->_numPrevBits = numPrevBits;
}

static void CLiteralEncoder_Init(CLiteralEncoder*le)
{
	unsigned numStates,i;
	numStates = 1 << (le->_numPosBits + le->_numPrevBits);
	for (i=0;i<numStates;i++)
		CLiteralEncoder2_Init(le->_coders[i]);
}

static unsigned CLiteralEncoder_GetState(CLiteralEncoder*le,unsigned pos,byte prevByte)
{return ((pos&le->_posMask)<<le->_numPrevBits)+(prevByte>>(8-le->_numPrevBits));}
static CLiteralEncoder2* CLiteralEncoder_GetSubCoder(CLiteralEncoder*le,unsigned pos,byte prevByte)
{return &le->_coders[CLiteralEncoder_GetState(le,pos,prevByte)];}

static unsigned CLiteralEncoder_GetPrice(CLiteralEncoder*le,unsigned pos,byte prevByte,
	bool matchMode, byte matchByte, byte symbol)
{
	return CLiteralEncoder2_GetPrice(le->_coders[CLiteralEncoder_GetState(le,pos,prevByte)],
		matchMode, matchByte, symbol);
}

static void CEncoder_Create(void*workmem)
{
	byte* workpos = (byte*)workmem;
	/* align on dword boundary */
	unsigned a;
	a = (unsigned)workpos & 3;
	if (a) workpos += 4-a;
	/* sizeof(CLiteralEncoder2) * (1<<(numPosBits+numPrevBits)) for literal encoders */
	/* = 0xC00 * 8 = 0x6000 with current settings */
	CLiteralEncoder_Create(&_literalEncoder,&workpos,_numLiteralPosStateBits,_numLiteralContextBits);
	/* (dictsize+0x1223)*1.5+256 for LZ input window */
	/* (0x140400 + (dictsize+1)*2) * 4 for match finder hash */
	MatchFinder_Create(_dictionarySize,kNumOpts,_numFastBytes,
		kMatchMaxLen*2+1-_numFastBytes,&workpos);
	/* total 0x508C3C + dictsize*9.5 */
	/* plus max 6 bytes for alignment */
}

static void CEncoder_Init(void)
{
	int i;
	unsigned j;
	CBaseState_Init();
	RangeEncoder_Init();
	for (i=0;i<kNumStates;i++)
	{
		for (j=0;j<=_posStateMask;j++)
		{
			CMyBitEncoder_Init(_isMatch[i][j]);
			CMyBitEncoder_Init(_isRep0Long[i][j]);
		}
		CMyBitEncoder_Init(_isRep[i]);
		CMyBitEncoder_Init(_isRepG0[i]);
		CMyBitEncoder_Init(_isRepG1[i]);
		CMyBitEncoder_Init(_isRepG2[i]);
	}
	CLiteralEncoder_Init(&_literalEncoder);
	for (i=0;i<kNumLenToPosStates;i++)
		CBitTreeEncoder_Init(_posSlotEncoder+i,kNumPosSlotBits);
	for (i=0;i<kNumFullDistances-kEndPosModelIndex;i++)
		CMyBitEncoder_Init(_posEncoders[i]);
	CPriceTableEncoder_Init(_lenEncoder, 1<<_posStateBits);
	CPriceTableEncoder_Init(_repMatchLenEncoder,1<<_posStateBits);
	CBitTreeEncoder_Init(&_posAlignEncoder,kNumAlignBits);
	_longestMatchWasFound = false;
	_optimumEndIndex = 0;
	_optimumCurrentIndex = 0;
	_additionalOffset = 0;
}

static void MovePos(unsigned num)
{
	for (;num--;)
	{
		DummyLongestMatch();
		MatchFinder_MovePos();
		_additionalOffset++;
	}
}

static unsigned Backward(unsigned* backRes, unsigned cur)
{
	unsigned posMem,backMem;
	unsigned posPrev,backCur;
	_optimumEndIndex = cur;
	posMem = _optimum[cur].PosPrev;
	backMem = _optimum[cur].BackPrev;
	do
	{
		if (_optimum[cur].Prev1IsChar)
		{
			COptimal_MakeAsChar(&_optimum[posMem]);
			_optimum[posMem].PosPrev = posMem-1;
			if (_optimum[cur].Prev2)
			{
				_optimum[posMem-1].Prev1IsChar = false;
				_optimum[posMem-1].PosPrev = _optimum[cur].PosPrev2;
				_optimum[posMem-1].BackPrev = _optimum[cur].BackPrev2;
			}
		}
		posPrev = posMem;
		backCur = backMem;
		backMem = _optimum[posPrev].BackPrev;
		posMem = _optimum[posPrev].PosPrev;
		_optimum[posPrev].BackPrev = backCur;
		_optimum[posPrev].PosPrev = cur;
		cur = posPrev;
	} while (cur);
	*backRes = _optimum[0].BackPrev;
	_optimumCurrentIndex = _optimum[0].PosPrev;
	return _optimumCurrentIndex;
}

static unsigned ReadMatchDistances(void)
{
	unsigned res;
	res = GetLongestMatch(_matchDistances);
	if (res == _numFastBytes)
		res += GetMatchLen(res,_matchDistances[res],kMatchMaxLen-res);
	_additionalOffset++;
	MatchFinder_MovePos();
	return res;
}

static void FillPosSlotPrices(void)
{
	unsigned lenToPosState,posSlot;
	for (lenToPosState=0;lenToPosState<kNumLenToPosStates;lenToPosState++)
	{
		for (posSlot=0;posSlot<kEndPosModelIndex && posSlot<_distTableSize;posSlot++)
			_posSlotPrices[lenToPosState][posSlot] = CBitTreeEncoder_GetPrice(&_posSlotEncoder[lenToPosState],posSlot);
		for (;posSlot<_distTableSize;posSlot++)
			_posSlotPrices[lenToPosState][posSlot] = CBitTreeEncoder_GetPrice(&_posSlotEncoder[lenToPosState],posSlot) +
				(((posSlot>>1)-1-kNumAlignBits)<<kNumBitPriceShiftBits);
	}
}

static void FillDistancesPrices(void)
{
	unsigned lenToPosState,i;
	unsigned posSlot,footerBits,base;
	for (lenToPosState=0;lenToPosState<kNumLenToPosStates;lenToPosState++)
	{
		for (i=0;i<kStartPosModelIndex;i++)
			_distancesPrices[lenToPosState][i] = _posSlotPrices[lenToPosState][i];
		for (;i<kNumFullDistances;i++)
		{
			posSlot = GetPosSlot(i);
			footerBits = ((posSlot>>1)-1);
			base = (2|(posSlot&1))<<footerBits;
			_distancesPrices[lenToPosState][i] = _posSlotPrices[lenToPosState][posSlot] +
				ReverseBitTreeGetPrice(_posEncoders+base-posSlot-1,footerBits,i-base);
		}
	}
}

static void FillAlignPrices(void)
{
	unsigned i;
	for (i=0;i<kAlignTableSize;i++)
		_alignPrices[i] = CBitTreeEncoder_ReverseGetPrice(&_posAlignEncoder,i);
	_alignPriceCount = kAlignTableSize;
}

static unsigned GetRepLen1Price(CState state, unsigned posState)
{
	return CMyBitEncoder_GetPrice0(&_isRepG0[state]) +
		CMyBitEncoder_GetPrice0(&_isRep0Long[state][posState]);
}
static unsigned GetRepPrice(unsigned repIndex, unsigned len, CState state, unsigned posState)
{
	unsigned price;
	price = CPriceTableEncoder_GetPrice(&_repMatchLenEncoder,len-kMatchMinLen,posState);
	if (!repIndex)
	{
		price += CMyBitEncoder_GetPrice0(&_isRepG0[state]);
		price += CMyBitEncoder_GetPrice1(&_isRep0Long[state][posState]);
	}
	else
	{
		price += CMyBitEncoder_GetPrice1(&_isRepG0[state]);
		if (repIndex == 1)
			price += CMyBitEncoder_GetPrice0(&_isRepG1[state]);
		else
		{
			price += CMyBitEncoder_GetPrice1(&_isRepG1[state]);
			price += CMyBitEncoder_GetPrice(&_isRepG2[state],repIndex-2);
		}
	}
	return price;
}
static unsigned GetPosLenPrice(unsigned pos, unsigned len, unsigned posState)
{
	unsigned price,lenToPosState;
	if (len==2 && pos>=0x80)
		return kIfinityPrice;
	lenToPosState = GetLenToPosState(len);
	if (pos < kNumFullDistances)
		price = _distancesPrices[lenToPosState][pos];
	else
		price = _posSlotPrices[lenToPosState][GetPosSlot2(pos)] +
			_alignPrices[pos & kAlignMask];
	return price + CPriceTableEncoder_GetPrice(&_lenEncoder,len-kMatchMinLen,posState);
}

static void GetOptimum(unsigned position,unsigned*backRes,unsigned*lenRes)
{
	int lenMain,lenEnd;
	COptimal* opt,*prevOpt;
	int reps[kNumRepDistances];
	int repLens[kNumRepDistances];
	int repIndex,repMaxIndex;
	int i,len,repLen,lenTest,newLen,lenTestTemp,lenTest2;
	int posState,posStateNext;
	byte currentByte,matchByte;
	unsigned matchPrice,repMatchPrice,shortRepPrice,normalMatchPrice,curAndLenPrice,curPrice,curAnd1Price,curAndLenCharPrice;
	unsigned nextMatchPrice,nextRepMatchPrice;
	int cur,posPrev,pos;
	CState state,state2;
	const byte* data;
	bool nextIsChar;
	int numAvailableBytesFull,numAvailableBytes;
	int backOffset,offset;
	int limit;
	if (_optimumEndIndex != _optimumCurrentIndex)
	{
		opt = &_optimum[_optimumCurrentIndex];
		*lenRes = opt->PosPrev - _optimumCurrentIndex;
		*backRes = opt->BackPrev;
		_optimumCurrentIndex = opt->PosPrev;
		return;
	}
	_optimumCurrentIndex = _optimumEndIndex = 0;
	if (!_longestMatchWasFound)
		lenMain = ReadMatchDistances();
	else
	{
		lenMain = _longestMatchLength;
		_longestMatchWasFound = false;
	}
	for (i=0;i<kNumRepDistances;i++)
	{
		reps[i] = _repDistances[i];
		repLens[i] = GetMatchLen(0-1,reps[i],kMatchMaxLen);
		if (i==0 || repLens[i] > repLens[repMaxIndex])
			repMaxIndex = i;
	}
	if (repLens[repMaxIndex] >= _numFastBytes)
	{
		*backRes = repMaxIndex;
		*lenRes = repLens[repMaxIndex];
		MovePos(*lenRes-1);
		return;
	}
	if (lenMain >= _numFastBytes)
	{
		*backRes = _matchDistances[_numFastBytes]+kNumRepDistances;
		*lenRes = lenMain;
		MovePos(lenMain-1);
		return;
	}
	currentByte = GetIndexByte(0-1);
	_optimum[0].State = _state;
	matchByte = GetIndexByte(0-_repDistances[0]-2);
	posState = position & _posStateMask;
	_optimum[1].Price = CMyBitEncoder_GetPrice0(&_isMatch[_state][posState]) +
		CLiteralEncoder_GetPrice(&_literalEncoder,position,_previousByte,
		(bool)!CState_IsCharState(_state),matchByte,currentByte);
	COptimal_MakeAsChar(&_optimum[1]);
	_optimum[1].PosPrev = 0;
	for (i=0;i<kNumRepDistances;i++)
		_optimum[0].Backs[i] = reps[i];
	matchPrice = CMyBitEncoder_GetPrice1(&_isMatch[_state][posState]);
	repMatchPrice = matchPrice + CMyBitEncoder_GetPrice1(&_isRep[_state]);
	if (matchByte == currentByte)
	{
		shortRepPrice = repMatchPrice + GetRepLen1Price(_state,posState);
		if (shortRepPrice < _optimum[1].Price)
		{
			_optimum[1].Price = shortRepPrice;
			COptimal_MakeAsShortRep(&_optimum[1]);
		}
	}
	if (lenMain < 2)
	{
		*backRes = _optimum[1].BackPrev;
		*lenRes = 1;
		return;
	}
	normalMatchPrice = matchPrice + CMyBitEncoder_GetPrice0(&_isRep[_state]);
	if (lenMain <= repLens[repMaxIndex])
		lenMain = 0;
	for (len=2;len<=lenMain;len++)
	{
		_optimum[len].PosPrev = 0;
		_optimum[len].BackPrev = _matchDistances[len] + kNumRepDistances;
		_optimum[len].Price = normalMatchPrice + GetPosLenPrice(_matchDistances[len],len,posState);
		_optimum[len].Prev1IsChar = false;
	}
	if (lenMain < repLens[repMaxIndex])
		lenMain = repLens[repMaxIndex];
	for (;len<=lenMain;len++)
		_optimum[len].Price = kIfinityPrice;
	for (i=0;i<kNumRepDistances;i++)
	{
		repLen = repLens[i];
		for (lenTest=2;lenTest<=repLen;lenTest++)
		{
			curAndLenPrice = repMatchPrice + GetRepPrice(i,lenTest,_state,posState);
			opt = &_optimum[lenTest];
			if (curAndLenPrice < opt->Price)
			{
				opt->Price = curAndLenPrice;
				opt->PosPrev = 0;
				opt->BackPrev = i;
				opt->Prev1IsChar = false;
			}
		}
	}
	cur=0;
	lenEnd = lenMain;
	while (1)
	{
		cur++;
		if (cur==lenEnd)
		{
			*lenRes = Backward(backRes,cur);
			return;
		}
		position++;
		opt = &_optimum[cur];
		posPrev = opt->PosPrev;
		if (opt->Prev1IsChar)
		{
			posPrev--;
			if (opt->Prev2)
			{
				state = _optimum[opt->PosPrev2].State;
				if (opt->BackPrev2 < kNumRepDistances)
					CState_UpdateRep(state);
				else
					CState_UpdateMatch(state);
			}
			else
				state = _optimum[posPrev].State;
			CState_UpdateChar(state);
		}
		else
			state = _optimum[posPrev].State;
		if (posPrev == cur-1)
		{
			if (COptimal_IsShortRep(opt))
				CState_UpdateShortRep(state);
			else
				CState_UpdateChar(state);
		}
		else
		{
			if (opt->Prev1IsChar && opt->Prev2)
			{
				posPrev = opt->PosPrev2;
				pos = opt->BackPrev2;
				CState_UpdateRep(state);
			}
			else
			{
				pos = opt->BackPrev;
				if (pos < kNumRepDistances)
					CState_UpdateRep(state);
				else
					CState_UpdateMatch(state);
			}
			prevOpt = &_optimum[posPrev];
			if (pos < kNumRepDistances)
			{
				reps[0] = prevOpt->Backs[pos];
				for (i=1;i<=pos;i++)
					reps[i] = prevOpt->Backs[i-1];
				for (;i<kNumRepDistances;i++)
					reps[i] = prevOpt->Backs[i];
			}
			else
			{
				reps[0] = pos-kNumRepDistances;
				for (i=1;i<kNumRepDistances;i++)
					reps[i] = prevOpt->Backs[i-1];
			}
		}
		opt->State = state;
		for (i=0;i<kNumRepDistances;i++)
			opt->Backs[i] = reps[i];
		newLen = ReadMatchDistances();
		if (newLen >= _numFastBytes)
		{
			_longestMatchLength = newLen;
			_longestMatchWasFound = true;
			*lenRes = Backward(backRes,cur);
			return;
		}
		curPrice = opt->Price;
		data = GetPointerToCurrentPos()-1;
		currentByte = *data;
		matchByte = data[-1-reps[0]];
		posState = position & _posStateMask;
		curAnd1Price = curPrice + CMyBitEncoder_GetPrice0(&_isMatch[state][posState]) +
			CLiteralEncoder_GetPrice(&_literalEncoder,position,data[-1],(bool)!CState_IsCharState(state),matchByte,currentByte);
		opt = &_optimum[cur+1];
		nextIsChar = false;
		if (curAnd1Price < opt->Price)
		{
			opt->Price = curAnd1Price;
			opt->PosPrev = cur;
			COptimal_MakeAsChar(opt);
			nextIsChar = true;
		}
		matchPrice = curPrice + CMyBitEncoder_GetPrice1(&_isMatch[state][posState]);
		repMatchPrice = matchPrice + CMyBitEncoder_GetPrice1(&_isRep[state]);
		if (matchByte == currentByte && !(opt->PosPrev<cur && !opt->BackPrev))
		{
			shortRepPrice = repMatchPrice + GetRepLen1Price(state,posState);
			if (shortRepPrice <= opt->Price)
			{
				opt->Price = shortRepPrice;
				opt->PosPrev = cur;
				COptimal_MakeAsShortRep(opt);
			}
		}
		numAvailableBytesFull = GetNumAvailableBytes()+1;
		if (numAvailableBytesFull > kNumOpts-1-cur)
			numAvailableBytesFull = kNumOpts-1-cur;
		numAvailableBytes = numAvailableBytesFull;
		if (numAvailableBytes < 2)
			continue;
		if (numAvailableBytes > _numFastBytes)
			numAvailableBytes = _numFastBytes;
		if (numAvailableBytes >= 3 && !nextIsChar)
		{
			// try Literal + rep0
			int temp;
			backOffset = reps[0]+1;
			for (temp=1;temp<numAvailableBytes;temp++)
				if (data[temp]!=data[temp-backOffset])
					break;
			lenTest = temp-1;
			if (lenTest>=2)
			{
				int posStateNext;
				unsigned nextRepMatchPrice;
				state2 = state;
				CState_UpdateChar(state2);
				posStateNext = (position+1) & _posStateMask;
				nextRepMatchPrice = curAnd1Price +
					CMyBitEncoder_GetPrice1(&_isMatch[state2][posStateNext]) +
					CMyBitEncoder_GetPrice1(&_isRep[state2]);
				while (lenEnd < cur+1+lenTest)
					_optimum[++lenEnd].Price = kIfinityPrice;
				curAndLenPrice = nextRepMatchPrice + GetRepPrice(0,lenTest,state2,posStateNext);
				opt = &_optimum[cur+1+lenTest];
				if (curAndLenPrice < opt->Price)
				{
					opt->Price = curAndLenPrice;
					opt->PosPrev = cur+1;
					opt->BackPrev = 0;
					opt->Prev1IsChar = true;
					opt->Prev2 = false;
				}
			}
		}
		for (repIndex=0;repIndex<kNumRepDistances;repIndex++)
		{
			backOffset = reps[repIndex]+1;
			if (data[0] != data[0-backOffset] ||
				data[1] != data[1-backOffset])
				continue;
			for (lenTest=2;lenTest<numAvailableBytes;lenTest++)
				if (data[lenTest]!=data[lenTest-backOffset])
					break;
			lenTestTemp = lenTest;
			do
			{
				while (lenEnd < cur+lenTest)
					_optimum[++lenEnd].Price = kIfinityPrice;
				curAndLenPrice = repMatchPrice + GetRepPrice(repIndex,lenTest,state,posState);
				opt = &_optimum[cur+lenTest];
				if (curAndLenPrice < opt->Price)
				{
					opt->Price = curAndLenPrice;
					opt->PosPrev = cur;
					opt->BackPrev = repIndex;
					opt->Prev1IsChar = false;
				}
			} while (--lenTest>=2);
			lenTest = lenTestTemp;
			lenTest2 = lenTest+1;
			limit = lenTest2 + _numFastBytes;
			if (limit > numAvailableBytesFull)
				limit = numAvailableBytesFull;
			for (;lenTest2<limit;lenTest2++)
				if (data[lenTest2] != data[lenTest2-backOffset])
					break;
			lenTest2 -= lenTest+1;
			if (lenTest2 >= 2)
			{
				unsigned nextMatchPrice,nextRepMatchPrice;
				int offset;
				state2 = state;
				CState_UpdateRep(state2);
				posStateNext = (position+lenTest)&_posStateMask;
				curAndLenCharPrice = repMatchPrice + GetRepPrice(repIndex,lenTest,state,posState) +
					CMyBitEncoder_GetPrice0(&_isMatch[state2][posStateNext]) +
					CLiteralEncoder_GetPrice(&_literalEncoder,position+lenTest,data[lenTest-1],true,data[lenTest-backOffset],data[lenTest]);
				CState_UpdateChar(state2);
				posStateNext = (position+lenTest+1)&_posStateMask;
				nextMatchPrice = curAndLenCharPrice + CMyBitEncoder_GetPrice1(&_isMatch[state2][posStateNext]);
				nextRepMatchPrice = nextMatchPrice + CMyBitEncoder_GetPrice1(&_isRep[state2]);
				offset = lenTest+1+lenTest2;
				while (lenEnd<cur+offset)
					_optimum[++lenEnd].Price = kIfinityPrice;
				curAndLenPrice = nextRepMatchPrice + GetRepPrice(0,lenTest2,state2,posStateNext);
				opt = &_optimum[cur+offset];
				if (curAndLenPrice < opt->Price)
				{
					opt->Price = curAndLenPrice;
					opt->PosPrev = cur+lenTest+1;
					opt->BackPrev = 0;
					opt->Prev1IsChar = true;
					opt->Prev2 = true;
					opt->PosPrev2 = cur;
					opt->BackPrev2 = repIndex;
				}
			}
		}
		if (newLen > numAvailableBytes)
			newLen = numAvailableBytes;
		if (newLen >= 2)
		{
			if (newLen==2 && _matchDistances[2] >= 0x80)
				continue;
			normalMatchPrice = matchPrice + CMyBitEncoder_GetPrice0(&_isRep[state]);
			while (lenEnd < cur+newLen)
				_optimum[++lenEnd].Price = kIfinityPrice;
			for (lenTest=newLen;lenTest>=2;lenTest--)
			{
				backOffset = _matchDistances[lenTest];
				curAndLenPrice = normalMatchPrice + GetPosLenPrice(backOffset,lenTest,posState);
				opt = &_optimum[cur+lenTest];
				if (curAndLenPrice < opt->Price)
				{
					opt->Price = curAndLenPrice;
					opt->PosPrev = cur;
					opt->BackPrev = backOffset+kNumRepDistances;
					opt->Prev1IsChar = false;
				}
				if (lenTest==newLen || backOffset!=_matchDistances[lenTest+1])
				{
					// Try Match + Literal + Rep0
					backOffset++;
					lenTest2 = lenTest+1;
					limit = lenTest2+_numFastBytes;
					if (limit > numAvailableBytesFull)
						limit = numAvailableBytesFull;
					for (;lenTest2<limit;lenTest2++)
						if (data[lenTest2]!=data[lenTest2-backOffset])
							break;
					lenTest2 -= lenTest+1;
					if (lenTest2 >= 2)
					{
						state2 = state;
						CState_UpdateMatch(state2);
						posStateNext = (position+lenTest)&_posStateMask;
						curAndLenCharPrice = curAndLenPrice + CMyBitEncoder_GetPrice0(&_isMatch[state2][posStateNext]) +
							CLiteralEncoder_GetPrice(&_literalEncoder,position+lenTest,data[lenTest-1],true,data[lenTest-backOffset],data[lenTest]);
						CState_UpdateChar(state2);
						posStateNext = (position+lenTest+1)&_posStateMask;
						nextMatchPrice = curAndLenCharPrice + CMyBitEncoder_GetPrice1(&_isMatch[state2][posStateNext]);
						nextRepMatchPrice = nextMatchPrice + CMyBitEncoder_GetPrice1(&_isRep[state2]);
						offset = lenTest+1+lenTest2;
						while (lenEnd<cur+offset)
							_optimum[++lenEnd].Price = kIfinityPrice;
						curAndLenPrice = nextRepMatchPrice + GetRepPrice(0,lenTest2,state2,posStateNext);
						opt = &_optimum[cur+offset];
						if (curAndLenPrice < opt->Price)
						{
							opt->Price = curAndLenPrice;
							opt->PosPrev = cur+lenTest+1;
							opt->BackPrev = 0;
							opt->Prev1IsChar = true;
							opt->Prev2 = true;
							opt->PosPrev2 = cur;
							opt->BackPrev2 = backOffset - 1 + kNumRepDistances;
						}
					}
				}
			}
		}
	}
}

static bool CodeOneBlock(void)
{
	unsigned posState;
	byte curByte,matchByte;
	unsigned pos,len,distance,i;
	unsigned posSlot,lenToPosState;
	CLiteralEncoder2* subCoder;
	uint64 progressPosValuePrev;

	if (_finished)
		return false;
	_finished = true;
	progressPosValuePrev = nowPos64;
	if (nowPos64 == 0)
	{
		if (GetNumAvailableBytes() == 0)
		{
			CEncoder_Flush();
			return false;
		}
		ReadMatchDistances();
		posState = (unsigned)nowPos64 & _posStateMask;
		CMyBitEncoder_Encode(&_isMatch[_state][posState],0);
		CState_UpdateChar(_state);
		curByte = GetIndexByte(0 - _additionalOffset);
		CLiteralEncoder2_Encode(
			*CLiteralEncoder_GetSubCoder(&_literalEncoder,(unsigned)nowPos64,_previousByte),
			curByte);
		_previousByte = curByte;
		_additionalOffset--;
		nowPos64++;
	}
	if (GetNumAvailableBytes() == 0)
	{
		CEncoder_Flush();
		return false;
	}
	for (;;)
	{
		posState = (unsigned)nowPos64 & _posStateMask;
		GetOptimum((unsigned)nowPos64,&pos,&len);
		if (len==1 && pos==0xFFFFFFFF)
		{
			CMyBitEncoder_Encode(&_isMatch[_state][posState],0);
			curByte = GetIndexByte(0-_additionalOffset);
			subCoder = CLiteralEncoder_GetSubCoder(&_literalEncoder,(unsigned)nowPos64,
				_previousByte);
			if (!CState_IsCharState(_state))
			{
				matchByte = GetIndexByte(0-_repDistances[0]-1-_additionalOffset);
				CLiteralEncoder2_EncodeMatched(*subCoder,matchByte,curByte);
			}
			else
				CLiteralEncoder2_Encode(*subCoder,curByte);
			CState_UpdateChar(_state);
			_previousByte = curByte;
		}
		else
		{
			CMyBitEncoder_Encode(&_isMatch[_state][posState],1);
			if (pos < kNumRepDistances)
			{
				CMyBitEncoder_Encode(&_isRep[_state],1);
				if (pos==0)
				{
					CMyBitEncoder_Encode(&_isRepG0[_state],0);
					CMyBitEncoder_Encode(&_isRep0Long[_state][posState],
						(len==1) ? 0 : 1);
				}
				else
				{
					CMyBitEncoder_Encode(&_isRepG0[_state],1);
					if (pos==1)
						CMyBitEncoder_Encode(&_isRepG1[_state],0);
					else
					{
						CMyBitEncoder_Encode(&_isRepG1[_state],1);
						CMyBitEncoder_Encode(&_isRepG2[_state],pos-2);
					}
				}
				if (len==1)
					CState_UpdateShortRep(_state);
				else
				{
					CPriceTableEncoder_Encode(&_repMatchLenEncoder,len-kMatchMinLen,posState);
					CState_UpdateRep(_state);
				}
				distance = _repDistances[pos];
				if (pos)
				{
					for (i=pos;i;i--)
						_repDistances[i] = _repDistances[i-1];
					_repDistances[0] = distance;
				}
			}
			else
			{
				CMyBitEncoder_Encode(&_isRep[_state],0);
				CState_UpdateMatch(_state);
				CPriceTableEncoder_Encode(&_lenEncoder,len-kMatchMinLen,posState);
				pos -= kNumRepDistances;
				posSlot = GetPosSlot(pos);
				lenToPosState = GetLenToPosState(len);
				CBitTreeEncoder_Encode(&_posSlotEncoder[lenToPosState],posSlot);
				if (posSlot >= kStartPosModelIndex)
				{
					unsigned footerBits;
					unsigned base,posReduced;
					footerBits = (posSlot>>1)-1;
					base = (2 | (posSlot&1)) << footerBits;
					posReduced = pos-base;
					if (posSlot < kEndPosModelIndex)
						ReverseBitTreeEncode(_posEncoders+base-posSlot-1,
							footerBits,posReduced);
					else
					{
						RangeEncoder_EncodeDirectBits(posReduced>>kNumAlignBits,footerBits-kNumAlignBits);
						CBitTreeEncoder_ReverseEncode(&_posAlignEncoder,posReduced&kAlignMask);
						if (--_alignPriceCount == 0)
							FillAlignPrices();
					}
				}
				distance = pos;
				for (i=kNumRepDistances-1;i;i--)
					_repDistances[i] = _repDistances[i-1];
				_repDistances[0] = distance;
			}
			_previousByte = GetIndexByte(len-1-_additionalOffset);
		}
		_additionalOffset -= len;
		nowPos64 += len;
		if (nowPos64 - lastPosSlotFillingPos >= (1<<9))
		{
			FillPosSlotPrices();
			FillDistancesPrices();
			lastPosSlotFillingPos = nowPos64;
		}
		if (!_additionalOffset)
		{
			if (GetNumAvailableBytes() == 0)
			{
				CEncoder_Flush();
				return false;
			}
			if (nowPos64 - progressPosValuePrev >= (1<<12))
			{
				_finished = false;
				return true;
			}
		}
	}
}

extern void __stdcall lzma_set_dict_size(
	unsigned logdictsize)
{
	_dictionarySize = 1 << logdictsize;
	_distTableSize = logdictsize*2;
}

extern unsigned __stdcall lzma_compress(
	const void* source,
	void* destination,
	unsigned length,
	void* workmem)
{
	FastPosInit();
	//memset(&encoder,0,sizeof(encoder));
	//memset(&rangeEncoder,0,sizeof(rangeEncoder));
	// CEncoder::CEncoder, CEncoder::SetCoderProperties
	_numFastBytes = 128;
#ifdef FOR_KERPACK
        _posStateBits = 0;
        _posStateMask = 0;
#else
	_posStateBits = 2;
        _posStateMask = 3;
#endif
	_numLiteralContextBits = 3;
	_numLiteralPosStateBits = 0;
	_writeEndMark = false;
	// CEncoder::Code - поехали!
	_finished = false;
	CEncoder_Create(workmem);
	CEncoder_Init();
	FillPosSlotPrices();
	FillDistancesPrices();
	FillAlignPrices();
	CPriceTableEncoder_SetTableSize(&_lenEncoder,_numFastBytes+1-kMatchMinLen);
	CPriceTableEncoder_UpdateTables(&_lenEncoder,1<<_posStateBits);
	CPriceTableEncoder_SetTableSize(&_repMatchLenEncoder,_numFastBytes+1-kMatchMinLen);
	CPriceTableEncoder_UpdateTables(&_repMatchLenEncoder,1<<_posStateBits);
	lastPosSlotFillingPos = 0;
	nowPos64 = 0;
	pack_length = length;
	pack_pos = 0;
	curin = (const byte*)source;
	curout = (byte*)destination;
	MatchFinder_Init();
	while (CodeOneBlock()) ;
	return curout - (byte*)destination;
}
