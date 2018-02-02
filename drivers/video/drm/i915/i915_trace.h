#if !defined(_I915_TRACE_H_) || defined(TRACE_HEADER_MULTI_READ)
#define _I915_TRACE_H_

//#include <linux/stringify.h>
#include <linux/types.h>

#include <drm/drmP.h>
#include "i915_drv.h"
#include "intel_drv.h"
#include "intel_ringbuffer.h"

#define trace_i915_gem_object_create(x)
#define trace_i915_gem_object_destroy(x)
#define trace_i915_gem_object_change_domain(a, b, c)
#define trace_i915_gem_object_unbind(x)
#define trace_i915_gem_ring_flush(a, b, c)
#define trace_i915_gem_object_bind(a, b)
#define trace_i915_ring_wait_end(x)
#define trace_i915_gem_request_add(req)
#define trace_i915_gem_request_retire(a)
#define trace_i915_gem_request_wait_begin(req)
#define trace_i915_gem_request_wait_end(req)
#define trace_i915_gem_request_complete(a)
#define trace_intel_gpu_freq_change(a)
#define trace_i915_reg_rw(a, b, c, d, e)
#define trace_i915_ring_wait_begin(a)
#define trace_i915_gem_object_pwrite(a, b, c)
#define trace_i915_gem_ring_dispatch(a, b)
#define trace_i915_gem_ring_sync_to(a, b, c)
#define trace_i915_vma_bind(a, b)
#define trace_i915_vma_unbind(a)
#define trace_i915_gem_object_clflush(a)
#define trace_i915_gem_evict(dev, min_size, alignment, flags)
#define trace_i915_gem_evict_vm(vm)
#define trace_i915_gem_evict_everything(dev)
#define trace_i915_context_free(ctx)
#define trace_i915_context_create(ctx)
#define trace_switch_mm(ring, to)
#define trace_i915_ppgtt_create(base)
#define trace_i915_ppgtt_release(base)
#define trace_i915_pipe_update_start(crtc)
#define trace_i915_pipe_update_vblank_evaded(crtc)
#define trace_i915_pipe_update_end(crtc, end_vbl_count, scanline_end)
#define trace_i915_page_directory_pointer_entry_alloc(vm,pml4e,start,GEN8_PML4E_SHIFT)
#define trace_i915_page_directory_entry_alloc(vm, pdpe, start, GEN8_PDPE_SHIFT)
#define trace_i915_page_table_entry_alloc(vm, pde, start, GEN6_PDE_SHIFT)
#define trace_i915_page_table_entry_map(vm, pde, pt, index, count, GEN6_PTES)
#define trace_i915_va_alloc(vm,start,size,name)
#define trace_i915_gem_request_notify(ring)
#define trace_i915_gem_object_pread(obj, offset, size)
#define trace_i915_flip_complete(plane, pending_flip_obj)
#define trace_i915_flip_request(plane, obj)

#endif
