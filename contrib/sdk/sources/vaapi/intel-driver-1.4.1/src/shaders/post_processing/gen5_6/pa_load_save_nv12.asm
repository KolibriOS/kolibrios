// Module name: PA_LOAD_SAVE_NV12
.kernel PA_LOAD_SAVE_NV12 
.code

#include "SetupVPKernel.asm"
#include "Multiple_Loop_Head.asm"
#include "PA_Load_8x8.asm"   
#include "PL8x8_PL8x4.asm"     
#include "PL8x4_Save_NV12.asm"
#include "Multiple_Loop.asm"

END_THREAD  // End of Thread

.end_code  

.end_kernel

// end of nv12_load_save_pl1.asm
