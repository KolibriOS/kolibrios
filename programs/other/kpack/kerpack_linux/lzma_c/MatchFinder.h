#include "common.h"

extern const byte* GetPointerToCurrentPos(void);
extern byte GetIndexByte(int index);
extern unsigned GetMatchLen(int index,unsigned distance,unsigned limit);
extern unsigned GetNumAvailableBytes(void);
extern void ReduceOffsets(int subValue);

extern void MatchFinder_Init(void);
extern void MatchFinder_Create(unsigned historySize,unsigned keepAddBufferBefore,unsigned matchMaxLen,unsigned keepAddBufferAfter,byte**mem);
extern unsigned GetLongestMatch(unsigned*distances);
extern void DummyLongestMatch(void);
extern void MatchFinder_MovePos(void);
