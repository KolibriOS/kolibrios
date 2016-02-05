#ifndef _INTEL_BATCHBUFFER_H_
#define _INTEL_BATCHBUFFER_H_

#include <xf86drm.h>
#include <drm.h>
#include <i915_drm.h>
#include <intel_bufmgr.h>

#include "intel_driver.h"

struct intel_batchbuffer 
{
    struct intel_driver_data *intel;
    dri_bo *buffer;
    unsigned int size;
    unsigned char *map;
    unsigned char *ptr;
    int atomic;
    int flag;

    int emit_total;
    unsigned char *emit_start;

    int (*run)(drm_intel_bo *bo, int used,
               drm_clip_rect_t *cliprects, int num_cliprects,
               int DR4, unsigned int ring_flag);

    /* Used for Sandybdrige workaround */
    dri_bo *wa_render_bo;
};

struct intel_batchbuffer *intel_batchbuffer_new(struct intel_driver_data *intel, int flag, int buffer_size);
void intel_batchbuffer_free(struct intel_batchbuffer *batch);
void intel_batchbuffer_start_atomic(struct intel_batchbuffer *batch, unsigned int size);
void intel_batchbuffer_start_atomic_bcs(struct intel_batchbuffer *batch, unsigned int size);
void intel_batchbuffer_start_atomic_blt(struct intel_batchbuffer *batch, unsigned int size);
void intel_batchbuffer_start_atomic_veb(struct intel_batchbuffer *batch, unsigned int size);
void intel_batchbuffer_end_atomic(struct intel_batchbuffer *batch);
void intel_batchbuffer_emit_dword(struct intel_batchbuffer *batch, unsigned int x);
void intel_batchbuffer_emit_reloc(struct intel_batchbuffer *batch, dri_bo *bo, 
                                  uint32_t read_domains, uint32_t write_domains, 
                                  uint32_t delta);
void intel_batchbuffer_emit_reloc64(struct intel_batchbuffer *batch, dri_bo *bo,
                                  uint32_t read_domains, uint32_t write_domains,
                                  uint32_t delta);
void intel_batchbuffer_require_space(struct intel_batchbuffer *batch, unsigned int size);
void intel_batchbuffer_data(struct intel_batchbuffer *batch, void *data, unsigned int size);
void intel_batchbuffer_emit_mi_flush(struct intel_batchbuffer *batch);
void intel_batchbuffer_flush(struct intel_batchbuffer *batch);
void intel_batchbuffer_begin_batch(struct intel_batchbuffer *batch, int total);
void intel_batchbuffer_advance_batch(struct intel_batchbuffer *batch);
void intel_batchbuffer_check_batchbuffer_flag(struct intel_batchbuffer *batch, int flag);
int intel_batchbuffer_check_free_space(struct intel_batchbuffer *batch, int size);
int intel_batchbuffer_used_size(struct intel_batchbuffer *batch);
void intel_batchbuffer_align(struct intel_batchbuffer *batch, unsigned int alignedment);

typedef enum {
    BSD_DEFAULT,
    BSD_RING0,
    BSD_RING1,
} bsd_ring_flag;

void intel_batchbuffer_start_atomic_bcs_override(struct intel_batchbuffer *batch, unsigned int size,
                                                 bsd_ring_flag override_flag);

#define __BEGIN_BATCH(batch, n, f) do {                         \
        assert(f == (batch->flag & I915_EXEC_RING_MASK));                               \
        intel_batchbuffer_check_batchbuffer_flag(batch, batch->flag);     \
        intel_batchbuffer_require_space(batch, (n) * 4);        \
        intel_batchbuffer_begin_batch(batch, (n));              \
    } while (0)

#define __OUT_BATCH(batch, d) do {              \
        intel_batchbuffer_emit_dword(batch, d); \
    } while (0)

#define __OUT_RELOC(batch, bo, read_domains, write_domain, delta) do {  \
        assert((delta) >= 0);                                           \
        intel_batchbuffer_emit_reloc(batch, bo,                         \
                                     read_domains, write_domain,        \
                                     delta);                            \
    } while (0)

/* Handle 48-bit address relocations for Gen8+ */
#define __OUT_RELOC64(batch, bo, read_domains, write_domain, delta) do { \
         intel_batchbuffer_emit_reloc64(batch, bo,                       \
         read_domains, write_domain,                                     \
         delta);                                                         \
    } while (0)

#define __ADVANCE_BATCH(batch) do {             \
        intel_batchbuffer_advance_batch(batch); \
    } while (0)

#define BEGIN_BATCH(batch, n)           __BEGIN_BATCH(batch, n, I915_EXEC_RENDER)
#define BEGIN_BLT_BATCH(batch, n)       __BEGIN_BATCH(batch, n, I915_EXEC_BLT)
#define BEGIN_BCS_BATCH(batch, n)       __BEGIN_BATCH(batch, n, I915_EXEC_BSD)
#define BEGIN_VEB_BATCH(batch, n)       __BEGIN_BATCH(batch, n, I915_EXEC_VEBOX)

#define OUT_BATCH(batch, d)             __OUT_BATCH(batch, d)
#define OUT_BLT_BATCH(batch, d)         __OUT_BATCH(batch, d)
#define OUT_BCS_BATCH(batch, d)         __OUT_BATCH(batch, d)
#define OUT_VEB_BATCH(batch, d)         __OUT_BATCH(batch, d)

#define OUT_RELOC(batch, bo, read_domains, write_domain, delta) \
    __OUT_RELOC(batch, bo, read_domains, write_domain, delta)
#define OUT_BLT_RELOC(batch, bo, read_domains, write_domain, delta)     \
    __OUT_RELOC(batch, bo, read_domains, write_domain, delta)
#define OUT_BCS_RELOC(batch, bo, read_domains, write_domain, delta)     \
    __OUT_RELOC(batch, bo, read_domains, write_domain, delta)
#define OUT_RELOC64(batch, bo, read_domains, write_domain, delta)       \
    __OUT_RELOC64(batch, bo, read_domains, write_domain, delta)

#define ADVANCE_BATCH(batch)            __ADVANCE_BATCH(batch)
#define ADVANCE_BLT_BATCH(batch)        __ADVANCE_BATCH(batch)
#define ADVANCE_BCS_BATCH(batch)        __ADVANCE_BATCH(batch)
#define ADVANCE_VEB_BATCH(batch)        __ADVANCE_BATCH(batch)

#endif /* _INTEL_BATCHBUFFER_H_ */
