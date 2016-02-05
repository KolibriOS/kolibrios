#ifndef _INTEL_DRIVER_H_
#define _INTEL_DRIVER_H_

#include <stddef.h>
//#include <pthread.h>
//#include <signal.h>
#include <stdbool.h>

#include <drm.h>
#include <i915_drm.h>
#include <intel_bufmgr.h>

#include <va/va_backend.h>
#include "va_backend_compat.h"

#include "intel_compiler.h"

#define BATCH_SIZE      0x80000
#define BATCH_RESERVED  0x10

#define CMD_MI                                  (0x0 << 29)
#define CMD_2D                                  (0x2 << 29)
#define CMD_3D                                  (0x3 << 29)

#define MI_NOOP                                 (CMD_MI | 0)

#define MI_BATCH_BUFFER_END                     (CMD_MI | (0xA << 23))
#define MI_BATCH_BUFFER_START                   (CMD_MI | (0x31 << 23))

#define MI_FLUSH                                (CMD_MI | (0x4 << 23))
#define   MI_FLUSH_STATE_INSTRUCTION_CACHE_INVALIDATE   (0x1 << 0)

#define MI_FLUSH_DW                             (CMD_MI | (0x26 << 23) | 0x2)
#define   MI_FLUSH_DW_VIDEO_PIPELINE_CACHE_INVALIDATE   (0x1 << 7)

#define XY_COLOR_BLT_CMD                        (CMD_2D | (0x50 << 22) | 0x04)
#define XY_COLOR_BLT_WRITE_ALPHA                (1 << 21)
#define XY_COLOR_BLT_WRITE_RGB                  (1 << 20)
#define XY_COLOR_BLT_DST_TILED                  (1 << 11)

#define GEN8_XY_COLOR_BLT_CMD                   (CMD_2D | (0x50 << 22) | 0x05)

/* BR13 */
#define BR13_8                                  (0x0 << 24)
#define BR13_565                                (0x1 << 24)
#define BR13_1555                               (0x2 << 24)
#define BR13_8888                               (0x3 << 24)

#define CMD_PIPE_CONTROL                        (CMD_3D | (3 << 27) | (2 << 24) | (0 << 16))
#define CMD_PIPE_CONTROL_CS_STALL               (1 << 20)
#define CMD_PIPE_CONTROL_NOWRITE                (0 << 14)
#define CMD_PIPE_CONTROL_WRITE_QWORD            (1 << 14)
#define CMD_PIPE_CONTROL_WRITE_DEPTH            (2 << 14)
#define CMD_PIPE_CONTROL_WRITE_TIME             (3 << 14)
#define CMD_PIPE_CONTROL_DEPTH_STALL            (1 << 13)
#define CMD_PIPE_CONTROL_WC_FLUSH               (1 << 12)
#define CMD_PIPE_CONTROL_IS_FLUSH               (1 << 11)
#define CMD_PIPE_CONTROL_TC_FLUSH               (1 << 10)
#define CMD_PIPE_CONTROL_NOTIFY_ENABLE          (1 << 8)
#define CMD_PIPE_CONTROL_DC_FLUSH               (1 << 5)
#define CMD_PIPE_CONTROL_GLOBAL_GTT             (1 << 2)
#define CMD_PIPE_CONTROL_LOCAL_PGTT             (0 << 2)
#define CMD_PIPE_CONTROL_STALL_AT_SCOREBOARD    (1 << 1)
#define CMD_PIPE_CONTROL_DEPTH_CACHE_FLUSH      (1 << 0)


struct intel_batchbuffer;

#define ALIGN(i, n)    (((i) + (n) - 1) & ~((n) - 1))
#define IS_ALIGNED(i, n) (((i) & ((n)-1)) == 0)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))
#define CLAMP(min, max, a) ((a) < (min) ? (min) : ((a) > (max) ? (max) : (a)))

#define Bool int
#define True 1
#define False 0

extern uint32_t g_intel_debug_option_flags;
#define VA_INTEL_DEBUG_OPTION_ASSERT    (1 << 0)
#define VA_INTEL_DEBUG_OPTION_BENCH     (1 << 1)
#define VA_INTEL_DEBUG_OPTION_DUMP_AUB  (1 << 2)

#define ASSERT_RET(value, fail_ret) do {    \
        if (!(value)) {                     \
            if (g_intel_debug_option_flags & VA_INTEL_DEBUG_OPTION_ASSERT)       \
                assert(value);              \
            return fail_ret;                \
        }                                   \
    } while (0)

#define SET_BLOCKED_SIGSET()   do {     \
        sigset_t bl_mask;               \
        sigfillset(&bl_mask);           \
        sigdelset(&bl_mask, SIGFPE);    \
        sigdelset(&bl_mask, SIGILL);    \
        sigdelset(&bl_mask, SIGSEGV);   \
        sigdelset(&bl_mask, SIGBUS);    \
        sigdelset(&bl_mask, SIGKILL);   \
        pthread_sigmask(SIG_SETMASK, &bl_mask, &intel->sa_mask); \
    } while (0)

#define RESTORE_BLOCKED_SIGSET() do {    \
        pthread_sigmask(SIG_SETMASK, &intel->sa_mask, NULL); \
    } while (0)

#define PPTHREAD_MUTEX_LOCK() do {             \
        SET_BLOCKED_SIGSET();                  \
        pthread_mutex_lock(&intel->ctxmutex);       \
    } while (0)

#define PPTHREAD_MUTEX_UNLOCK() do {           \
        pthread_mutex_unlock(&intel->ctxmutex);     \
        RESTORE_BLOCKED_SIGSET();              \
    } while (0)

#define WARN_ONCE(...) do {                     \
        static int g_once = 1;                  \
        if (g_once) {                           \
            g_once = 0;                         \
            fprintf(stderr, "WARNING: " __VA_ARGS__);    \
        }                                       \
    } while (0)

struct intel_device_info
{
    int gen;
    int gt;

    unsigned int urb_size;
    unsigned int max_wm_threads;

    unsigned int is_g4x         : 1; /* gen4 */
    unsigned int is_ivybridge   : 1; /* gen7 */
    unsigned int is_baytrail    : 1; /* gen7 */
    unsigned int is_haswell     : 1; /* gen7 */
    unsigned int is_cherryview  : 1; /* gen8 */
    unsigned int is_broxton     : 1; /* gen9 */
};

struct intel_driver_data 
{
    int fd;
    int device_id;
    int revision;

    int dri2Enabled;

//    sigset_t sa_mask;
//    pthread_mutex_t ctxmutex;
    int locked;

    dri_bufmgr *bufmgr;

    unsigned int has_exec2  : 1; /* Flag: has execbuffer2? */
    unsigned int has_bsd    : 1; /* Flag: has bitstream decoder for H.264? */
    unsigned int has_blt    : 1; /* Flag: has BLT unit? */
    unsigned int has_vebox  : 1; /* Flag: has VEBOX unit */
    unsigned int has_bsd2   : 1; /* Flag: has the second BSD video ring unit */

    const struct intel_device_info *device_info;
};

bool intel_driver_init(VADriverContextP ctx);
void intel_driver_terminate(VADriverContextP ctx);

static INLINE struct intel_driver_data *
intel_driver_data(VADriverContextP ctx)
{
    return (struct intel_driver_data *)ctx->pDriverData;
}

struct intel_region
{
    int x;
    int y;
    unsigned int width;
    unsigned int height;
    unsigned int cpp;
    unsigned int pitch;
    unsigned int tiling;
    unsigned int swizzle;
    dri_bo *bo;
};

#define IS_G4X(device_info)             (device_info->is_g4x)

#define IS_IRONLAKE(device_info)        (device_info->gen == 5)

#define IS_GEN6(device_info)            (device_info->gen == 6)

#define IS_HASWELL(device_info)         (device_info->is_haswell)
#define IS_GEN7(device_info)            (device_info->gen == 7)

#define IS_CHERRYVIEW(device_info)      (device_info->is_cherryview)
#define IS_GEN8(device_info)            (device_info->gen == 8)

#define IS_GEN9(device_info)            (device_info->gen == 9)

#endif /* _INTEL_DRIVER_H_ */
