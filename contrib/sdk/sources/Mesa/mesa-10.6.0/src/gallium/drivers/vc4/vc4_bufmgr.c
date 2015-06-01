/*
 * Copyright © 2014 Broadcom
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <errno.h>
#include <err.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "util/u_memory.h"
#include "util/ralloc.h"

#include "vc4_context.h"
#include "vc4_screen.h"

#define container_of(ptr, type, field) \
   (type*)((char*)ptr - offsetof(type, field))

static struct vc4_bo *
vc4_bo_from_cache(struct vc4_screen *screen, uint32_t size, const char *name)
{
        struct vc4_bo_cache *cache = &screen->bo_cache;
        uint32_t page_index = size / 4096 - 1;

        if (cache->size_list_size <= page_index)
                return NULL;

        struct vc4_bo *bo = NULL;
        pipe_mutex_lock(cache->lock);
        if (!is_empty_list(&cache->size_list[page_index])) {
                struct simple_node *node = last_elem(&cache->size_list[page_index]);
                bo = container_of(node, struct vc4_bo, size_list);
                pipe_reference_init(&bo->reference, 1);
                remove_from_list(&bo->time_list);
                remove_from_list(&bo->size_list);

                bo->name = name;
        }
        pipe_mutex_unlock(cache->lock);
        return bo;
}

struct vc4_bo *
vc4_bo_alloc(struct vc4_screen *screen, uint32_t size, const char *name)
{
        struct vc4_bo *bo;
        int ret;

        size = align(size, 4096);

        bo = vc4_bo_from_cache(screen, size, name);
        if (bo)
                return bo;

        bo = CALLOC_STRUCT(vc4_bo);
        if (!bo)
                return NULL;

        pipe_reference_init(&bo->reference, 1);
        bo->screen = screen;
        bo->size = size;
        bo->name = name;
        bo->private = true;

        if (!using_vc4_simulator) {
                struct drm_vc4_create_bo create;
                memset(&create, 0, sizeof(create));

                create.size = size;

                ret = drmIoctl(screen->fd, DRM_IOCTL_VC4_CREATE_BO, &create);
                bo->handle = create.handle;
        } else {
                struct drm_mode_create_dumb create;
                memset(&create, 0, sizeof(create));

                create.width = 128;
                create.bpp = 8;
                create.height = (size + 127) / 128;

                ret = drmIoctl(screen->fd, DRM_IOCTL_MODE_CREATE_DUMB, &create);
                bo->handle = create.handle;
                assert(create.size >= size);
        }
        if (ret != 0) {
                fprintf(stderr, "create ioctl failure\n");
                abort();
        }

        return bo;
}

void
vc4_bo_last_unreference(struct vc4_bo *bo)
{
        struct vc4_screen *screen = bo->screen;

        struct timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        pipe_mutex_lock(screen->bo_cache.lock);
        vc4_bo_last_unreference_locked_timed(bo, time.tv_sec);
        pipe_mutex_unlock(screen->bo_cache.lock);
}

static void
vc4_bo_free(struct vc4_bo *bo)
{
        struct vc4_screen *screen = bo->screen;

        if (bo->map) {
#ifdef USE_VC4_SIMULATOR
                if (bo->simulator_winsys_map) {
                        free(bo->map);
                        bo->map = bo->simulator_winsys_map;
                }
#endif
                munmap(bo->map, bo->size);
        }

        struct drm_gem_close c;
        memset(&c, 0, sizeof(c));
        c.handle = bo->handle;
        int ret = drmIoctl(screen->fd, DRM_IOCTL_GEM_CLOSE, &c);
        if (ret != 0)
                fprintf(stderr, "close object %d: %s\n", bo->handle, strerror(errno));

        free(bo);
}

static void
free_stale_bos(struct vc4_screen *screen, time_t time)
{
        while (!is_empty_list(&screen->bo_cache.time_list)) {
                struct simple_node *node =
                        first_elem(&screen->bo_cache.time_list);
                struct vc4_bo *bo = container_of(node, struct vc4_bo, time_list);

                /* If it's more than a second old, free it. */
                if (time - bo->free_time > 2) {
                        remove_from_list(&bo->time_list);
                        remove_from_list(&bo->size_list);
                        vc4_bo_free(bo);
                } else {
                        break;
                }
        }
}

void
vc4_bo_last_unreference_locked_timed(struct vc4_bo *bo, time_t time)
{
        struct vc4_screen *screen = bo->screen;
        struct vc4_bo_cache *cache = &screen->bo_cache;
        uint32_t page_index = bo->size / 4096 - 1;

        if (!bo->private) {
                vc4_bo_free(bo);
                return;
        }

        if (cache->size_list_size <= page_index) {
                struct simple_node *new_list =
                        ralloc_array(screen, struct simple_node, page_index + 1);

                /* Move old list contents over (since the array has moved, and
                 * therefore the pointers to the list heads have to change.
                 */
                for (int i = 0; i < cache->size_list_size; i++) {
                        struct simple_node *old_head = &cache->size_list[i];
                        if (is_empty_list(old_head))
                                make_empty_list(&new_list[i]);
                        else {
                                new_list[i].next = old_head->next;
                                new_list[i].prev = old_head->prev;
                                new_list[i].next->prev = &new_list[i];
                                new_list[i].prev->next = &new_list[i];
                        }
                }
                for (int i = cache->size_list_size; i < page_index + 1; i++)
                        make_empty_list(&new_list[i]);

                cache->size_list = new_list;
                cache->size_list_size = page_index + 1;
        }

        bo->free_time = time;
        insert_at_tail(&cache->size_list[page_index], &bo->size_list);
        insert_at_tail(&cache->time_list, &bo->time_list);

        free_stale_bos(screen, time);
}

static struct vc4_bo *
vc4_bo_open_handle(struct vc4_screen *screen,
                   uint32_t winsys_stride,
                   uint32_t handle, uint32_t size)
{
        struct vc4_bo *bo = CALLOC_STRUCT(vc4_bo);

        assert(size);

        pipe_reference_init(&bo->reference, 1);
        bo->screen = screen;
        bo->handle = handle;
        bo->size = size;
        bo->name = "winsys";
        bo->private = false;

#ifdef USE_VC4_SIMULATOR
        vc4_bo_map(bo);
        bo->simulator_winsys_map = bo->map;
        bo->simulator_winsys_stride = winsys_stride;
        bo->map = malloc(bo->size);
#endif

        return bo;
}

struct vc4_bo *
vc4_bo_open_name(struct vc4_screen *screen, uint32_t name,
                 uint32_t winsys_stride)
{
        struct drm_gem_open o = {
                .name = name
        };
        int ret = drmIoctl(screen->fd, DRM_IOCTL_GEM_OPEN, &o);
        if (ret) {
                fprintf(stderr, "Failed to open bo %d: %s\n",
                        name, strerror(errno));
                return NULL;
        }

        return vc4_bo_open_handle(screen, winsys_stride, o.handle, o.size);
}

struct vc4_bo *
vc4_bo_open_dmabuf(struct vc4_screen *screen, int fd, uint32_t winsys_stride)
{
        uint32_t handle;
        int ret = drmPrimeFDToHandle(screen->fd, fd, &handle);
        int size;
        if (ret) {
                fprintf(stderr, "Failed to get vc4 handle for dmabuf %d\n", fd);
                return NULL;
        }

        /* Determine the size of the bo we were handed. */
        size = lseek(fd, 0, SEEK_END);
        if (size == -1) {
                fprintf(stderr, "Couldn't get size of dmabuf fd %d.\n", fd);
                return NULL;
        }

        return vc4_bo_open_handle(screen, winsys_stride, handle, size);
}

int
vc4_bo_get_dmabuf(struct vc4_bo *bo)
{
        int fd;
        int ret = drmPrimeHandleToFD(bo->screen->fd, bo->handle,
                                     O_CLOEXEC, &fd);
        if (ret != 0) {
                fprintf(stderr, "Failed to export gem bo %d to dmabuf\n",
                        bo->handle);
                return -1;
        }

        return fd;
}

struct vc4_bo *
vc4_bo_alloc_mem(struct vc4_screen *screen, const void *data, uint32_t size,
                 const char *name)
{
        void *map;
        struct vc4_bo *bo;

        bo = vc4_bo_alloc(screen, size, name);
        map = vc4_bo_map(bo);
        memcpy(map, data, size);
        return bo;
}

bool
vc4_bo_flink(struct vc4_bo *bo, uint32_t *name)
{
        struct drm_gem_flink flink = {
                .handle = bo->handle,
        };
        int ret = drmIoctl(bo->screen->fd, DRM_IOCTL_GEM_FLINK, &flink);
        if (ret) {
                fprintf(stderr, "Failed to flink bo %d: %s\n",
                        bo->handle, strerror(errno));
                free(bo);
                return false;
        }

        bo->private = false;
        *name = flink.name;

        return true;
}

bool
vc4_wait_seqno(struct vc4_screen *screen, uint64_t seqno, uint64_t timeout_ns)
{
        if (screen->finished_seqno >= seqno)
                return true;

        struct drm_vc4_wait_seqno wait;
        memset(&wait, 0, sizeof(wait));
        wait.seqno = seqno;
        wait.timeout_ns = timeout_ns;

        int ret;
        if (!using_vc4_simulator)
                ret = drmIoctl(screen->fd, DRM_IOCTL_VC4_WAIT_SEQNO, &wait);
        else {
                wait.seqno = screen->finished_seqno;
                ret = 0;
        }

        if (ret == -ETIME) {
                return false;
        } else if (ret != 0) {
                fprintf(stderr, "wait failed\n");
                abort();
        } else {
                screen->finished_seqno = wait.seqno;
                return true;
        }
}

bool
vc4_bo_wait(struct vc4_bo *bo, uint64_t timeout_ns)
{
        struct vc4_screen *screen = bo->screen;

        struct drm_vc4_wait_bo wait;
        memset(&wait, 0, sizeof(wait));
        wait.handle = bo->handle;
        wait.timeout_ns = timeout_ns;

        int ret;
        if (!using_vc4_simulator)
                ret = drmIoctl(screen->fd, DRM_IOCTL_VC4_WAIT_BO, &wait);
        else
                ret = 0;

        if (ret == -ETIME) {
                return false;
        } else if (ret != 0) {
                fprintf(stderr, "wait failed\n");
                abort();
        } else {
                return true;
        }
}

void *
vc4_bo_map_unsynchronized(struct vc4_bo *bo)
{
        uint64_t offset;
        int ret;

        if (bo->map)
                return bo->map;

        if (!using_vc4_simulator) {
                struct drm_vc4_mmap_bo map;
                memset(&map, 0, sizeof(map));
                map.handle = bo->handle;
                ret = drmIoctl(bo->screen->fd, DRM_IOCTL_VC4_MMAP_BO, &map);
                offset = map.offset;
        } else {
                struct drm_mode_map_dumb map;
                memset(&map, 0, sizeof(map));
                map.handle = bo->handle;
                ret = drmIoctl(bo->screen->fd, DRM_IOCTL_MODE_MAP_DUMB, &map);
                offset = map.offset;
        }
        if (ret != 0) {
                fprintf(stderr, "map ioctl failure\n");
                abort();
        }

        bo->map = mmap(NULL, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED,
                       bo->screen->fd, offset);
        if (bo->map == MAP_FAILED) {
                fprintf(stderr, "mmap of bo %d (offset 0x%016llx, size %d) failed\n",
                        bo->handle, (long long)offset, bo->size);
                abort();
        }

        return bo->map;
}

void *
vc4_bo_map(struct vc4_bo *bo)
{
        void *map = vc4_bo_map_unsynchronized(bo);

        bool ok = vc4_bo_wait(bo, PIPE_TIMEOUT_INFINITE);
        if (!ok) {
                fprintf(stderr, "BO wait for map failed\n");
                abort();
        }

        return map;
}

void
vc4_bufmgr_destroy(struct pipe_screen *pscreen)
{
        struct vc4_screen *screen = vc4_screen(pscreen);
        struct vc4_bo_cache *cache = &screen->bo_cache;

        while (!is_empty_list(&cache->time_list)) {
                struct simple_node *node = first_elem(&cache->time_list);
                struct vc4_bo *bo = container_of(node, struct vc4_bo, time_list);

                remove_from_list(&bo->time_list);
                remove_from_list(&bo->size_list);
                vc4_bo_free(bo);
        }
}
