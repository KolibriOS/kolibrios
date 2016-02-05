// Module name: PL3_LOAD_SAVE_NV12
.kernel PL3_LOAD_SAVE_NV12
.code

#include "SetupVPKernel.asm"
#include "Multiple_Loop_Head.asm"
#include "IMC3_Load_8x4.asm"        
#include "PL8x4_Save_NV12.asm"
#include "Multiple_Loop.asm"

END_THREAD  // End of Thread

.end_code  

.end_kernel

// end of pl3_load_save_nv12.asm
