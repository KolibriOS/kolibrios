// Module name: NV12_DN_NV12
.kernel NV12_DN_NV12
.code

#define INC_DN
        
#include "SetupVPKernel.asm"
#include "Multiple_Loop_Head.asm"

#define LOAD_UV_ONLY
#include "NV12_Load_8x4.asm"
#undef LOAD_UV_ONLY

#include "PL_DN_ALG.asm"        
        
#include "PL8x4_Save_NV12.asm"
        
#include "Multiple_Loop.asm"

END_THREAD  // End of Thread

.end_code  

.end_kernel

// end of nv12_dn_nv12.asm
