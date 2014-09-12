#include "common.h"
#define kNumRepDistances 4
#define kNumStates 12
extern const byte kLiteralNextStates[kNumStates];
extern const byte kMatchNextStates[kNumStates];
extern const byte kRepNextStates[kNumStates];
extern const byte kShortRepNextStates[kNumStates];

typedef byte CState;
#define CState_Init(a) a=0
#define CState_UpdateChar(a) a=kLiteralNextStates[a]
#define CState_UpdateMatch(a) a=kMatchNextStates[a]
#define CState_UpdateRep(a) a=kRepNextStates[a]
#define CState_UpdateShortRep(a) a=kShortRepNextStates[a]
#define CState_IsCharState(a) (a<7)

#define kNumPosSlotBits 6
#define kDicLogSizeMin 0
#define kDicLogSizeMax 32
#define kDistTableSizeMax (kDicLogSizeMax*2)
#define kNumLenToPosStates 4

#define GetLenToPosState(len) ((len<kNumLenToPosStates+2)?len-2:kNumLenToPosStates-1)

#define kNumPosStatesBitsMax	4
#define kNumPosStatesMax	(1<<kNumPosStatesBitsMax)

#define kNumPosStatesBitsEncodingMax	4
#define kNumPosStatesEncodingMax	(1 << kNumPosStatesBitsEncodingMax)

#define kNumLowBits	3
#define kNumMidBits	3
#define kNumHighBits	8
#define kNumLowSymbols	(1<<kNumLowBits)
#define kNumMidSymbols	(1<<kNumMidBits)
#define kNumSymbolsTotal (kNumLowSymbols + kNumMidSymbols + (1<<kNumHighBits))

#define kMatchMinLen 2
#define kMatchMaxLen (kMatchMinLen + kNumSymbolsTotal - 1)

#define kNumAlignBits 4
#define kAlignTableSize (1<<kNumAlignBits)
#define kAlignMask (kAlignTableSize-1)

#define kStartPosModelIndex 4
#define kEndPosModelIndex 14
#define kNumPosModels (kEndPosModelIndex-kStartPosModelIndex)

#define kNumFullDistances (1<<(kEndPosModelIndex/2))
#define kNumLitPosStatesBitsEncodingMax 4
#define kNumLitContextBitsMax 8
#define kNumMoveBits 5
