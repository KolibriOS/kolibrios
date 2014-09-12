#include "MatchFinder.h"
/* memcpy must be inlined - we do not want to use RTL */
#include <memory.h>
/*#pragma function(memcpy)
void* __cdecl memcpy(void* _Dst, const void* _Src, size_t _Size)
{
	unsigned long i;
	for (i = 0; i < _Size; i++)
		((char*)_Dst)[i] = ((char*)_Src)[i];
	return _Dst;
}*/
//#pragma intrinsic(memcpy)

#define kMaxValForNormalize (((unsigned)1<<31)-1)

/* settings for bt4:
	defined HASH_ARRAY_2
	defined HASH_ARRAY_3
*/

//#define kHash2Size 0x400
#define kNumHashDirectBytes 0
#define kNumHashBytes 3
//#define kHash3Size 0x40000
#define kHash2Size 0x10000
#define kHashSize 0x100000

#define kHashSizeSum (kHashSize+kHash2Size)
#define kHash2Offset kHashSize

static unsigned _cyclicBufferPos;
static unsigned _cyclicBufferSize;
static unsigned _matchMaxLen;
static unsigned* _hash;
static unsigned _cutValue;

#ifdef GENERIC_INPUT
static byte* _bufferBase;
static unsigned _posLimit;
static bool _streamEndWasReached;
static byte* _pointerToLastSafePosition;
static byte* _buffer;
static unsigned _blockSize;
static unsigned _pos;
static unsigned _keepSizeBefore;
static unsigned _keepSizeAfter;
static unsigned _keepSizeReserv;
static unsigned _streamPos;
#else
#define _buffer curin
#define _pos pack_pos
#define _streamPos pack_length
#endif

#ifdef GENERIC_INPUT
/* LZ Window */

static void LZInWindow_Create(unsigned keepSizeBefore,unsigned keepSizeAfter,unsigned keepSizeReserv,byte**mem)
{
	_keepSizeBefore = keepSizeBefore;
	_keepSizeAfter = keepSizeAfter;
	_keepSizeReserv = keepSizeReserv;
	_blockSize = keepSizeBefore + keepSizeAfter + keepSizeReserv;
	_bufferBase = *mem;
	_blockSize = (_blockSize + 3) & ~3;
	*mem += _blockSize;
	_pointerToLastSafePosition = _bufferBase + _blockSize - keepSizeAfter;
}

static void ReadBlock(void)
{
	if (_streamEndWasReached)
		return;
	for (;;)
	{
		unsigned size;
		size = (unsigned)(_bufferBase-_buffer) + _blockSize - _streamPos;
		if (!size) return;
		if (size > pack_length - pack_pos)
			size = pack_length - pack_pos;
		memcpy(_buffer+_streamPos,curin,size);
		curin += size;
		pack_pos += size;
		if (size == 0)
		{
			byte* pointerToPosition;
			_posLimit = _streamPos;
			pointerToPosition = _buffer + _posLimit;
			if (pointerToPosition > _pointerToLastSafePosition)
				_posLimit = _pointerToLastSafePosition - _buffer;
			_streamEndWasReached = true;
			return;
		}
		_streamPos += size;
		if (_streamPos >= _pos + _keepSizeAfter)
		{
			_posLimit = _streamPos - _keepSizeAfter;
			return;
		}
	}
}

static void LZInWindow_Init(void)
{
	_buffer = _bufferBase;
	_pos = 0;
	_streamPos = 0;
	_streamEndWasReached = false;
	ReadBlock();
}
#else
#define LZInWindow_Create(a,b,c,d) /* nothing */
#define LZInWindow_Init() _buffer--, _pos++, _streamPos++
#endif

const byte* GetPointerToCurrentPos(void) {return _buffer+_pos;}

#ifdef GENERIC_INPUT
static void MoveBlock(void)
{
	unsigned offset,numBytes;
	offset = _buffer-_bufferBase+_pos-_keepSizeBefore;
	numBytes = _buffer-_bufferBase+_streamPos-offset;
	// copying backwards: safe to use memcpy instead of memmove
	memcpy(_bufferBase,_bufferBase+offset,numBytes);
	_buffer -= offset;
}

static void LZInWindow_MovePos(void)
{
	_pos++;
	if (_pos > _posLimit)
	{
		const byte* pointerToPosition = _buffer+_pos;
		if (pointerToPosition > _pointerToLastSafePosition)
			MoveBlock();
		ReadBlock();
	}
}
#else
#define LZInWindow_MovePos() _pos++
#endif

byte GetIndexByte(int index) {return _buffer[_pos+index];}

unsigned GetMatchLen(int index,unsigned distance,unsigned limit)
{
	const byte* pby;
	unsigned i;
#ifdef GENERIC_INPUT
	if (_streamEndWasReached)
		if ((_pos+index)+limit > _streamPos)
			limit = _streamPos - (_pos+index);
#else
	unsigned limit2 = pack_length - (pack_pos + index);
	if (limit > limit2)
		limit = limit2;
#endif
	distance++;
	pby = _buffer + _pos + index;
	for (i=0;i<limit && pby[i]==pby[(int)(i-distance)];i++) ;
	return i;
}

unsigned GetNumAvailableBytes(void) {return _streamPos-_pos;}

#ifdef GENERIC_INPUT
void ReduceOffsets(int subValue)
{
	_buffer += subValue;
	_posLimit -= subValue;
	_pos -= subValue;
	_streamPos -= subValue;
}
#else
#define ReduceOffsets(a) /* nothing */
#endif

/* Binary tree Match Finder */

static unsigned crc_table[256];

void MatchFinder_Create(unsigned historySize,unsigned keepAddBufferBefore,unsigned matchMaxLen,unsigned keepAddBufferAfter,byte**mem)
{
	unsigned sizeReserv;
	sizeReserv = (historySize + keepAddBufferBefore + matchMaxLen + keepAddBufferAfter)/2+256;
	LZInWindow_Create(historySize+keepAddBufferBefore,matchMaxLen+keepAddBufferAfter,sizeReserv,mem);
	_matchMaxLen = matchMaxLen;
	_cyclicBufferSize = historySize+1;
	_hash = (unsigned*)*mem;
	*mem += (kHashSizeSum + _cyclicBufferSize*2) * sizeof(unsigned);
	_cutValue = 0xFF;
}

void MatchFinder_Init(void)
{
	unsigned i,j,r;
	LZInWindow_Init();
	for (i=0;i<kHashSizeSum;i++)
		_hash[i] = 0;
	_cyclicBufferPos = 0;
	ReduceOffsets(-1);
	for (i=0;i<256;i++)
	{
		r = i;
		for (j=0;j<8;j++)
		{
			if (r & 1)
				r = (r>>1) ^ 0xEDB88320;
			else
				r >>= 1;
		}
		crc_table[i] = r;
	}
}

static unsigned Hash(const byte* ptr, unsigned* hash2Value)
{
	unsigned temp;
	temp = crc_table[ptr[0]] ^ ptr[1];
	*hash2Value = *(word*)ptr; //ptr[0] + ((unsigned)ptr[1] << 8);
	return (temp ^ ((unsigned)ptr[2]<<8)) & (kHashSize - 1);
}

unsigned GetLongestMatch(unsigned* distances)
{
	unsigned lenLimit,maxLen=0;
	unsigned matchMinPos;
	const byte* cur;
	unsigned hash2Value,hashValue;
	unsigned curMatch,curMatch2;
	unsigned *son,*ptr0,*ptr1;
	unsigned len0,len1,count;
	if (_pos + _matchMaxLen <= _streamPos)
		lenLimit = _matchMaxLen;
	else
	{
		lenLimit = _streamPos - _pos;
		if (lenLimit < kNumHashBytes)
			return 0;
	}
	matchMinPos = (_pos>_cyclicBufferSize) ? (_pos-_cyclicBufferSize) : 0;
	cur = _buffer+_pos;
	hashValue = Hash(cur,&hash2Value);
	curMatch = _hash[hashValue];
	curMatch2 = _hash[kHash2Offset + hash2Value];
	_hash[kHash2Offset + hash2Value] = _pos;
	distances[2] = 0xFFFFFFFF;
	if (curMatch2 > matchMinPos)
		//if (_buffer[curMatch2] == cur[0])
		{
			distances[2] = _pos - curMatch2 - 1;
			maxLen = 2;
		}
	_hash[hashValue] = _pos;
	son = _hash + kHashSizeSum;
	ptr0 = son + (_cyclicBufferPos << 1) + 1;
	ptr1 = son + (_cyclicBufferPos << 1);
	distances[kNumHashBytes] = 0xFFFFFFFF;
	len0 = len1 = kNumHashDirectBytes;
	count = _cutValue;
	for (;;)
	{
		const byte* pb;
		unsigned len,delta;
		unsigned cyclicPos;
		unsigned* pair;
		if (curMatch <= matchMinPos || count--==0)
		{
			*ptr0 = *ptr1 = 0;
			break;
		}
		pb = _buffer+curMatch;
		len = (len0<len1) ? len0 : len1;
		do
			if (pb[len] != cur[len]) break;
		while (++len != lenLimit);
		delta = _pos - curMatch;
		while (maxLen < len)
			distances[++maxLen] = delta-1;
		cyclicPos = (delta <= _cyclicBufferPos) ?
			(_cyclicBufferPos - delta) :
			(_cyclicBufferPos - delta + _cyclicBufferSize);
		pair = son + (cyclicPos<<1);
		if (len != lenLimit)
		{
			if (pb[len] < cur[len])
			{
				*ptr1 = curMatch;
				ptr1 = pair+1;
				curMatch = *ptr1;
				len1 = len;
			}
			else
			{
				*ptr0 = curMatch;
				ptr0 = pair;
				curMatch = *ptr0;
				len0 = len;
			}
		}
		else
		{
			*ptr1 = pair[0];
			*ptr0 = pair[1];
			break;
		}
	}
	/*if (distances[4] < distances[3])
		distances[3] = distances[4];
	if (distances[3] < distances[2])
		distances[2] = distances[3];*/
	return maxLen;
}

void DummyLongestMatch(void)
{
	unsigned lenLimit;
	unsigned matchMinPos;
	const byte* cur;
	unsigned hash2Value,hashValue;
	unsigned curMatch;
	unsigned* son,*ptr0,*ptr1;
	unsigned len0,len1,count;
	if (_pos + _matchMaxLen <= _streamPos)
		lenLimit = _matchMaxLen;
	else
	{
		lenLimit = _streamPos - _pos;
		if (lenLimit < kNumHashBytes)
			return;
	}
	matchMinPos = (_pos > _cyclicBufferSize) ? (_pos - _cyclicBufferSize) : 0;
	cur = _buffer+_pos;
	hashValue = Hash(cur,&hash2Value);
	_hash[kHash2Offset + hash2Value] = _pos;
	curMatch = _hash[hashValue];
	_hash[hashValue] = _pos;
	son = _hash+kHashSizeSum;
	ptr0 = son + (_cyclicBufferPos << 1) + 1;
	ptr1 = son + (_cyclicBufferPos << 1);
	len0 = len1 = kNumHashDirectBytes;
	count = _cutValue;
	for (;;)
	{
		const byte* pb;
		unsigned len;
		unsigned delta,cyclicPos;
		unsigned* pair;
		if (curMatch <= matchMinPos || count--==0)
			break;
		pb = _buffer+curMatch;
		len = (len0<len1) ? len0 : len1;
		do
			if (pb[len] != cur[len]) break;
		while (++len != lenLimit);
		delta = _pos - curMatch;
		cyclicPos = (delta <= _cyclicBufferPos) ?
			(_cyclicBufferPos - delta) :
			(_cyclicBufferPos - delta + _cyclicBufferSize);
		pair = son + (cyclicPos << 1);
		if (len != lenLimit)
		{
			if (pb[len] < cur[len])
			{
				*ptr1 = curMatch;
				ptr1 = pair+1;
				curMatch = *ptr1;
				len1 = len;
			}
			else
			{
				*ptr0 = curMatch;
				ptr0 = pair;
				curMatch = *ptr0;
				len0 = len;
			}
		}
		else
		{
			*ptr1 = pair[0];
			*ptr0 = pair[1];
			return;
		}
	}
	*ptr0 = *ptr1 = 0;
}

#ifdef GENERIC_INPUT
// for memory input size is always less than kMaxValForNormalize
static void Normalize(void)
{
	unsigned subValue;
	unsigned* items;
	unsigned i,numItems;
	subValue = _pos - _cyclicBufferSize;
	items = _hash;
	numItems = (kHashSizeSum + _cyclicBufferSize*2);
	for (i=0;i<numItems;i++)
	{
		unsigned value;
		value = items[i];
		if (value <= subValue)
			value = 0;
		else
			value -= subValue;
		items[i] = value;
	}
	ReduceOffsets(subValue);
}
#endif

void MatchFinder_MovePos(void)
{
	if (++_cyclicBufferPos == _cyclicBufferSize)
		_cyclicBufferPos = 0;
	LZInWindow_MovePos();
#ifdef GENERIC_INPUT
	if (_pos == kMaxValForNormalize)
		Normalize();
#endif
}
