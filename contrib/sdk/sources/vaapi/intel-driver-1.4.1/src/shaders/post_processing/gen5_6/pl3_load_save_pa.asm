// Module name: PL3_LOAD_SAVE_pa
.kernel PL3_LOAD_SAVE_PA // what's usage of it? just a name?
.code

#include "SetupVPKernel.asm"
#include "Multiple_Loop_Head.asm"
#include "IMC3_Load_8x5.asm"   
#include "PL8x5_PL8x8.asm"     
#include "PL8x8_Save_PA.asm"
#include "Multiple_Loop.asm"

END_THREAD  // End of Thread

.end_code  

.end_kernel

// end of pl3_load_save_pa.asm
