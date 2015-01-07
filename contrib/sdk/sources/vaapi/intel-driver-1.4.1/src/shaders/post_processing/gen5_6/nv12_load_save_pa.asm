// Module name: NV12_LOAD_SAVE_pl1
.kernel NV12_LOAD_SAVE_PL1 // what's usage of it? just a name?
.code

#include "SetupVPKernel.asm"
#include "Multiple_Loop_Head.asm"
#include "NV12_Load_8x5.asm"   
#include "PL8x5_PL8x8.asm"     
#include "PL8x8_Save_PA.asm"
#include "Multiple_Loop.asm"

END_THREAD  // End of Thread

.end_code  

.end_kernel

// end of nv12_load_save_pl1.asm
