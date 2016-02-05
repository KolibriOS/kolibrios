// Module name: NV12_LOAD_SAVE_RGBX
.kernel NV12_LOAD_SAVE_RGBX
.code
#define FIX_POINT_CONVERSION
// #define FLOAT_POINT_CONVERSION

#include "SetupVPKernel.asm"
#include "YUV_to_RGBX_Coef.asm"
#include "Multiple_Loop_Head.asm"
#include "NV12_Load_8x4.asm"
#ifdef FIX_POINT_CONVERSION
  #include "YUVX_Save_RGBX_Fix.asm"
#else
  #include "YUVX_Save_RGBX_Float.asm"
#endif
#include "RGB16x8_Save_RGB.asm"
#include "Multiple_Loop.asm"

END_THREAD  // End of Thread

.end_code  

.end_kernel

// end of nv12_load_save_rgbx.asm
