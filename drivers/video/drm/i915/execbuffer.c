#include "drmP.h"
#include "drm.h"
#include "i915_drm.h"
#include "i915_drv.h"
#include "intel_drv.h"
//#include

#undef mb
#undef rmb
#undef wmb
#define mb() asm volatile("mfence")
#define rmb() asm volatile ("lfence")
#define wmb() asm volatile ("sfence")


typedef struct
{
    struct drm_i915_gem_object *batch;
    struct list_head  objects;
    u32    exec_start;
    u32    exec_len;

}batchbuffer_t;

struct change_domains {
    uint32_t invalidate_domains;
    uint32_t flush_domains;
    uint32_t flush_rings;
    uint32_t flips;
};

/*
 * Set the next domain for the specified object. This
 * may not actually perform the necessary flushing/invaliding though,
 * as that may want to be batched with other set_domain operations
 *
 * This is (we hope) the only really tricky part of gem. The goal
 * is fairly simple -- track which caches hold bits of the object
 * and make sure they remain coherent. A few concrete examples may
 * help to explain how it works. For shorthand, we use the notation
 * (read_domains, write_domain), e.g. (CPU, CPU) to indicate the
 * a pair of read and write domain masks.
 *
 * Case 1: the batch buffer
 *
 *  1. Allocated
 *  2. Written by CPU
 *  3. Mapped to GTT
 *  4. Read by GPU
 *  5. Unmapped from GTT
 *  6. Freed
 *
 *  Let's take these a step at a time
 *
 *  1. Allocated
 *      Pages allocated from the kernel may still have
 *      cache contents, so we set them to (CPU, CPU) always.
 *  2. Written by CPU (using pwrite)
 *      The pwrite function calls set_domain (CPU, CPU) and
 *      this function does nothing (as nothing changes)
 *  3. Mapped by GTT
 *      This function asserts that the object is not
 *      currently in any GPU-based read or write domains
 *  4. Read by GPU
 *      i915_gem_execbuffer calls set_domain (COMMAND, 0).
 *      As write_domain is zero, this function adds in the
 *      current read domains (CPU+COMMAND, 0).
 *      flush_domains is set to CPU.
 *      invalidate_domains is set to COMMAND
 *      clflush is run to get data out of the CPU caches
 *      then i915_dev_set_domain calls i915_gem_flush to
 *      emit an MI_FLUSH and drm_agp_chipset_flush
 *  5. Unmapped from GTT
 *      i915_gem_object_unbind calls set_domain (CPU, CPU)
 *      flush_domains and invalidate_domains end up both zero
 *      so no flushing/invalidating happens
 *  6. Freed
 *      yay, done
 *
 * Case 2: The shared render buffer
 *
 *  1. Allocated
 *  2. Mapped to GTT
 *  3. Read/written by GPU
 *  4. set_domain to (CPU,CPU)
 *  5. Read/written by CPU
 *  6. Read/written by GPU
 *
 *  1. Allocated
 *      Same as last example, (CPU, CPU)
 *  2. Mapped to GTT
 *      Nothing changes (assertions find that it is not in the GPU)
 *  3. Read/written by GPU
 *      execbuffer calls set_domain (RENDER, RENDER)
 *      flush_domains gets CPU
 *      invalidate_domains gets GPU
 *      clflush (obj)
 *      MI_FLUSH and drm_agp_chipset_flush
 *  4. set_domain (CPU, CPU)
 *      flush_domains gets GPU
 *      invalidate_domains gets CPU
 *      wait_rendering (obj) to make sure all drawing is complete.
 *      This will include an MI_FLUSH to get the data from GPU
 *      to memory
 *      clflush (obj) to invalidate the CPU cache
 *      Another MI_FLUSH in i915_gem_flush (eliminate this somehow?)
 *  5. Read/written by CPU
 *      cache lines are loaded and dirtied
 *  6. Read written by GPU
 *      Same as last GPU access
 *
 * Case 3: The constant buffer
 *
 *  1. Allocated
 *  2. Written by CPU
 *  3. Read by GPU
 *  4. Updated (written) by CPU again
 *  5. Read by GPU
 *
 *  1. Allocated
 *      (CPU, CPU)
 *  2. Written by CPU
 *      (CPU, CPU)
 *  3. Read by GPU
 *      (CPU+RENDER, 0)
 *      flush_domains = CPU
 *      invalidate_domains = RENDER
 *      clflush (obj)
 *      MI_FLUSH
 *      drm_agp_chipset_flush
 *  4. Updated (written) by CPU again
 *      (CPU, CPU)
 *      flush_domains = 0 (no previous write domain)
 *      invalidate_domains = 0 (no new read domains)
 *  5. Read by GPU
 *      (CPU+RENDER, 0)
 *      flush_domains = CPU
 *      invalidate_domains = RENDER
 *      clflush (obj)
 *      MI_FLUSH
 *      drm_agp_chipset_flush
 */
static void
i915_gem_object_set_to_gpu_domain(struct drm_i915_gem_object *obj,
                  struct intel_ring_buffer *ring,
                  struct change_domains *cd)
{
    uint32_t invalidate_domains = 0, flush_domains = 0;

    /*
     * If the object isn't moving to a new write domain,
     * let the object stay in multiple read domains
     */
    if (obj->base.pending_write_domain == 0)
        obj->base.pending_read_domains |= obj->base.read_domains;

    /*
     * Flush the current write domain if
     * the new read domains don't match. Invalidate
     * any read domains which differ from the old
     * write domain
     */
    if (obj->base.write_domain &&
        (((obj->base.write_domain != obj->base.pending_read_domains ||
           obj->ring != ring)) ||
         (obj->fenced_gpu_access && !obj->pending_fenced_gpu_access))) {
        flush_domains |= obj->base.write_domain;
        invalidate_domains |=
            obj->base.pending_read_domains & ~obj->base.write_domain;
    }
    /*
     * Invalidate any read caches which may have
     * stale data. That is, any new read domains.
     */
    invalidate_domains |= obj->base.pending_read_domains & ~obj->base.read_domains;
    if ((flush_domains | invalidate_domains) & I915_GEM_DOMAIN_CPU)
        i915_gem_clflush_object(obj);

    if (obj->base.pending_write_domain)
        cd->flips |= atomic_read(&obj->pending_flip);

    /* The actual obj->write_domain will be updated with
     * pending_write_domain after we emit the accumulated flush for all
     * of our domain changes in execbuffers (which clears objects'
     * write_domains).  So if we have a current write domain that we
     * aren't changing, set pending_write_domain to that.
     */
    if (flush_domains == 0 && obj->base.pending_write_domain == 0)
        obj->base.pending_write_domain = obj->base.write_domain;

    cd->invalidate_domains |= invalidate_domains;
    cd->flush_domains |= flush_domains;
    if (flush_domains & I915_GEM_GPU_DOMAINS)
        cd->flush_rings |= obj->ring->id;
    if (invalidate_domains & I915_GEM_GPU_DOMAINS)
        cd->flush_rings |= ring->id;
}

static int
i915_gem_execbuffer_flush(struct drm_device *dev,
              uint32_t invalidate_domains,
              uint32_t flush_domains,
              uint32_t flush_rings)
{
    drm_i915_private_t *dev_priv = dev->dev_private;
    int i, ret;

    if (flush_domains & I915_GEM_DOMAIN_CPU)
        intel_gtt_chipset_flush();

    if (flush_domains & I915_GEM_DOMAIN_GTT)
        wmb();

    if ((flush_domains | invalidate_domains) & I915_GEM_GPU_DOMAINS) {
        for (i = 0; i < I915_NUM_RINGS; i++)
            if (flush_rings & (1 << i)) {
                ret = i915_gem_flush_ring(&dev_priv->ring[i],
                              invalidate_domains,
                              flush_domains);
                if (ret)
                    return ret;
            }
    }

    return 0;
}

static int
i915_gem_execbuffer_move_to_gpu(struct intel_ring_buffer *ring,
                struct list_head *objects)
{
    struct drm_i915_gem_object *obj;
    struct change_domains cd;
    int ret;

    memset(&cd, 0, sizeof(cd));
    list_for_each_entry(obj, objects, exec_list)
        i915_gem_object_set_to_gpu_domain(obj, ring, &cd);

    if (cd.invalidate_domains | cd.flush_domains) {
        ret = i915_gem_execbuffer_flush(ring->dev,
                        cd.invalidate_domains,
                        cd.flush_domains,
                        cd.flush_rings);
        if (ret)
            return ret;
    }

//    if (cd.flips) {
//        ret = i915_gem_execbuffer_wait_for_flips(ring, cd.flips);
//        if (ret)
//            return ret;
//    }

//    list_for_each_entry(obj, objects, exec_list) {
//        ret = i915_gem_execbuffer_sync_rings(obj, ring);
//        if (ret)
//            return ret;
//    }

    return 0;
}

static void
i915_gem_execbuffer_move_to_active(struct list_head *objects,
                   struct intel_ring_buffer *ring,
                   u32 seqno)
{
    struct drm_i915_gem_object *obj;

    list_for_each_entry(obj, objects, exec_list) {
          u32 old_read = obj->base.read_domains;
          u32 old_write = obj->base.write_domain;


        obj->base.read_domains = obj->base.pending_read_domains;
        obj->base.write_domain = obj->base.pending_write_domain;
        obj->fenced_gpu_access = obj->pending_fenced_gpu_access;

        i915_gem_object_move_to_active(obj, ring, seqno);
        if (obj->base.write_domain) {
            obj->dirty = 1;
            obj->pending_gpu_write = true;
            list_move_tail(&obj->gpu_write_list,
                       &ring->gpu_write_list);
//            intel_mark_busy(ring->dev, obj);
        }

//        trace_i915_gem_object_change_domain(obj, old_read, old_write);
    }
}

static void
i915_gem_execbuffer_retire_commands(struct drm_device *dev,
                    struct intel_ring_buffer *ring)
{
    struct drm_i915_gem_request *request;
    u32 invalidate;

    /*
     * Ensure that the commands in the batch buffer are
     * finished before the interrupt fires.
     *
     * The sampler always gets flushed on i965 (sigh).
     */
    invalidate = I915_GEM_DOMAIN_COMMAND;
    if (INTEL_INFO(dev)->gen >= 4)
        invalidate |= I915_GEM_DOMAIN_SAMPLER;
    if (ring->flush(ring, invalidate, 0)) {
        i915_gem_next_request_seqno(ring);
        return;
    }

    /* Add a breadcrumb for the completion of the batch buffer */
    request = kzalloc(sizeof(*request), GFP_KERNEL);
    if (request == NULL || i915_add_request(ring, NULL, request)) {
        i915_gem_next_request_seqno(ring);
        kfree(request);
    }
}


int exec_batch(struct drm_device *dev, struct intel_ring_buffer *ring,
               batchbuffer_t *exec)
{
    drm_i915_private_t *dev_priv = dev->dev_private;
    struct drm_i915_gem_object *obj;

    u32 seqno;
    int i;
    int ret;

    ring = &dev_priv->ring[RCS];

    mutex_lock(&dev->struct_mutex);

    list_for_each_entry(obj, &exec->objects, exec_list)
    {
        obj->base.pending_read_domains = 0;
        obj->base.pending_write_domain = 0;
    };

    exec->batch->base.pending_read_domains |= I915_GEM_DOMAIN_COMMAND;

    ret = i915_gem_execbuffer_move_to_gpu(ring, &exec->objects);
    if (ret)
        goto err;

    seqno = i915_gem_next_request_seqno(ring);
//    for (i = 0; i < ARRAY_SIZE(ring->sync_seqno); i++) {
//        if (seqno < ring->sync_seqno[i]) {
            /* The GPU can not handle its semaphore value wrapping,
             * so every billion or so execbuffers, we need to stall
             * the GPU in order to reset the counters.
             */
//            ret = i915_gpu_idle(dev);
//            if (ret)
//                goto err;

//            BUG_ON(ring->sync_seqno[i]);
//        }
//    };

    ret = ring->dispatch_execbuffer(ring, exec->exec_start, exec->exec_len);
    if (ret)
        goto err;

    i915_gem_execbuffer_move_to_active(&exec->objects, ring, seqno);
    i915_gem_execbuffer_retire_commands(dev, ring);

err:
    mutex_unlock(&dev->struct_mutex);

    return ret;

};
