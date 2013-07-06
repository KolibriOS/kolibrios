#ifndef KOS_SW_WINSYS_H
#define KOS_SW_WINSYS_H


#include "pipe/p_compiler.h"
#include "state_tracker/sw_winsys.h"

void kos_sw_display( struct sw_winsys *winsys,
                     struct sw_displaytarget *dt);

struct sw_winsys *kos_create_sw_winsys(void);

#endif
