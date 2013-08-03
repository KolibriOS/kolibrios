
#include "target-helpers/inline_sw_helper.h"
#include "target-helpers/inline_debug_helper.h"
#include "state_tracker/drm_driver.h"

PUBLIC struct pipe_screen *
swrast_create_screen(struct sw_winsys *ws);

PUBLIC
DRM_DRIVER_DESCRIPTOR("swrast", NULL, NULL, NULL)

struct pipe_screen *
swrast_create_screen(struct sw_winsys *ws)
{
    struct pipe_screen *screen;
   
    ws = kos_create_sw_winsys();
    if (ws == NULL)
        return NULL;
 
    screen = softpipe_create_screen(ws); 
    return screen;
}
