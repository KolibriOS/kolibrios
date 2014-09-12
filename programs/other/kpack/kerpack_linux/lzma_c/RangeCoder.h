#include "common.h"
#define kNumTopBits 24
#define kTopValue (1<<kNumTopBits)
extern void RangeEncoder_Init(void);
extern void RangeEncoder_FlushData(void);
extern void RangeEncoder_ShiftLow(void);
extern void RangeEncoder_EncodeDirectBits(unsigned value,int numTotalBits);
