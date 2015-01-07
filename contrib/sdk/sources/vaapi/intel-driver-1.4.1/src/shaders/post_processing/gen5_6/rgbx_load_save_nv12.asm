// Module name: RGBX_LOAD_SAVE_NV12
.kernel RGBX_LOAD_SAVE_NV12 
.code
#define FIX_POINT_CONVERSION
// #define FLOAT_POINT_CONVERSION

#include "SetupVPKernel.asm"
#include "RGBX_to_YUV_Coef.asm"
#include "Multiple_Loop_Head.asm"
#include "RGBX_Load_16x8.asm"   
#ifdef FIX_POINT_CONVERSION
  #include "RGBX_Save_YUV_Fix.asm"
#else
  #include "RGBX_Save_YUV_Float.asm"
#endif
#include "PL16x8_PL8x4.asm"     
#include "PL8x4_Save_NV12.asm"
#include "Multiple_Loop.asm"

END_THREAD  // End of Thread

.end_code  

.end_kernel

// end of rgbx_load_save_nv12.asm
