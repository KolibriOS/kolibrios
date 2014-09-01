#if !defined(_RADEON_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _RADEON_TRACE_H_

#include <linux/stringify.h>
#include <linux/types.h>
#include <drm/drmP.h>


#define trace_radeon_vm_set_page(pe, addr, count, incr, flags)
#define trace_radeon_fence_emit(ddev, ring, seq)
#define trace_radeon_fence_wait_begin(ddev, i, target_seq)
#define trace_radeon_fence_wait_end(ddev, i, target_seq)
#define trace_radeon_semaphore_signale(ridx, semaphore)
#define trace_radeon_semaphore_wait(ridx, semaphore)
#define trace_radeon_vm_grab_id(id, ring)
#define trace_radeon_vm_bo_update(bo_va)
#define trace_radeon_bo_create(bo)
#define trace_radeon_cs(parser)
#define trace_radeon_vm_flush(pd_addr, ring, id)

#endif
