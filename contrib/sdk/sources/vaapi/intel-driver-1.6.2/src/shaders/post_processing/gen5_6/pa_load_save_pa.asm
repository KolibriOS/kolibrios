// Module name: PA_LOAD_SAVE_PA
.kernel PA_LOAD_SAVE_PA
.code

#include "SetupVPKernel.asm"
#include "Multiple_Loop_Head.asm"
#include "PA_Load_8x8.asm"
#include "PL8x8_Save_PA.asm"
#include "Multiple_Loop.asm"

END_THREAD  // End of Thread

.end_code

.end_kernel

// end of pa_load_save_pa.asm
