// Module name: AVS
.kernel RGBX_TO_NV12
.code

#include "VP_Setup.g4a"
#include "Set_Layer_0.g4a"
#include "Set_AVS_Buf_0123_BGRA.g4a"
#include "PA_AVS_Buf_0.g4a"
#include "PA_AVS_Buf_1.g4a"
#include "PA_AVS_Buf_2.g4a"
#include "PA_AVS_Buf_3.g4a"
#include "RGB_to_YUV.g4a"
#include "Save_AVS_NV12.g4a"        
#include "EOT.g4a"

.end_code  

.end_kernel
