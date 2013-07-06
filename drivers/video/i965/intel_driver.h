#ifndef _INTEL_DRIVER_H_
#define _INTEL_DRIVER_H_

#include <stddef.h>
#include <pthread.h>
#include <signal.h>

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

/* BR13 */
#define BR13_8                                  (0x0 << 24)
#define BR13_565                                (0x1 << 24)
#define BR13_1555                               (0x2 << 24)
#define BR13_8888                               (0x3 << 24)

#define CMD_PIPE_CONTROL                        (CMD_3D | (3 << 27) | (2 << 24) | (0 << 16))
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
#define CMD_PIPE_CONTROL_DEPTH_CACHE_FLUSH      (1 << 0)


struct intel_batchbuffer;

#define ALIGN(i, n)    (((i) + (n) - 1) & ~((n) - 1))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

#define Bool int
#define True 1
#define False 0

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
            printf("WARNING: " __VA_ARGS__);    \
        }                                       \
    } while (0)

struct intel_driver_data 
{
    int fd;
    int device_id;
    int revision;

    int dri2Enabled;

    sigset_t sa_mask;
//    pthread_mutex_t ctxmutex;
    int locked;

    dri_bufmgr *bufmgr;

    unsigned int has_exec2  : 1; /* Flag: has execbuffer2? */
    unsigned int has_bsd    : 1; /* Flag: has bitstream decoder for H.264? */
    unsigned int has_blt    : 1; /* Flag: has BLT unit? */
};

Bool intel_driver_init(VADriverContextP ctx);
Bool intel_driver_terminate(VADriverContextP ctx);

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

#define PCI_CHIP_GM45_GM                0x2A42
#define PCI_CHIP_IGD_E_G                0x2E02
#define PCI_CHIP_Q45_G                  0x2E12
#define PCI_CHIP_G45_G                  0x2E22
#define PCI_CHIP_G41_G                  0x2E32
#define PCI_CHIP_B43_G                  0x2E42
#define PCI_CHIP_B43_G1                 0x2E92

#define PCI_CHIP_IRONLAKE_D_G           0x0042
#define PCI_CHIP_IRONLAKE_M_G           0x0046

#ifndef PCI_CHIP_SANDYBRIDGE_GT1
#define PCI_CHIP_SANDYBRIDGE_GT1	0x0102  /* Desktop */
#define PCI_CHIP_SANDYBRIDGE_GT2	0x0112
#define PCI_CHIP_SANDYBRIDGE_GT2_PLUS	0x0122
#define PCI_CHIP_SANDYBRIDGE_M_GT1	0x0106  /* Mobile */
#define PCI_CHIP_SANDYBRIDGE_M_GT2	0x0116
#define PCI_CHIP_SANDYBRIDGE_M_GT2_PLUS	0x0126
#define PCI_CHIP_SANDYBRIDGE_S_GT	0x010A  /* Server */
#endif

#define PCI_CHIP_IVYBRIDGE_GT1          0x0152  /* Desktop */
#define PCI_CHIP_IVYBRIDGE_GT2          0x0162
#define PCI_CHIP_IVYBRIDGE_M_GT1        0x0156  /* Mobile */
#define PCI_CHIP_IVYBRIDGE_M_GT2        0x0166
#define PCI_CHIP_IVYBRIDGE_S_GT1        0x015a  /* Server */
#define PCI_CHIP_IVYBRIDGE_S_GT2        0x016a

#define PCI_CHIP_HASWELL_GT1            0x0402 /* Desktop */
#define PCI_CHIP_HASWELL_GT2            0x0412
#define PCI_CHIP_HASWELL_GT2_PLUS       0x0422
#define PCI_CHIP_HASWELL_M_GT1          0x0406 /* Mobile */
#define PCI_CHIP_HASWELL_M_GT2          0x0416
#define PCI_CHIP_HASWELL_M_GT2_PLUS     0x0426
#define PCI_CHIP_HASWELL_S_GT1          0x040a /* Server */
#define PCI_CHIP_HASWELL_S_GT2          0x041a
#define PCI_CHIP_HASWELL_S_GT2_PLUS     0x042a

#define	PCI_CHIP_HASWELL_SDV_GT1		0x0c02 /* Desktop */
#define	PCI_CHIP_HASWELL_SDV_GT2		0x0c12
#define	PCI_CHIP_HASWELL_SDV_GT2_PLUS		0x0c22
#define	PCI_CHIP_HASWELL_SDV_M_GT1		0x0c06 /* Mobile */
#define	PCI_CHIP_HASWELL_SDV_M_GT2		0x0c16
#define	PCI_CHIP_HASWELL_SDV_M_GT2_PLUS		0x0c26
#define	PCI_CHIP_HASWELL_SDV_S_GT1		0x0c0a /* Server */
#define	PCI_CHIP_HASWELL_SDV_S_GT2		0x0c1a
#define	PCI_CHIP_HASWELL_SDV_S_GT2_PLUS		0x0c2a

#define	PCI_CHIP_HASWELL_ULT_GT1		0x0A02 /* Desktop */
#define	PCI_CHIP_HASWELL_ULT_GT2		0x0A12
#define	PCI_CHIP_HASWELL_ULT_GT2_PLUS		0x0A22
#define	PCI_CHIP_HASWELL_ULT_M_GT1		0x0A06 /* Mobile */
#define	PCI_CHIP_HASWELL_ULT_M_GT2		0x0A16
#define	PCI_CHIP_HASWELL_ULT_M_GT2_PLUS		0x0A26
#define	PCI_CHIP_HASWELL_ULT_S_GT1		0x0A0A /* Server */
#define	PCI_CHIP_HASWELL_ULT_S_GT2		0x0A1A
#define	PCI_CHIP_HASWELL_ULT_S_GT2_PLUS		0x0A2A

#define	PCI_CHIP_HASWELL_CRW_GT1		0x0D12 /* Desktop */
#define	PCI_CHIP_HASWELL_CRW_GT2		0x0D22
#define	PCI_CHIP_HASWELL_CRW_GT2_PLUS		0x0D32
#define	PCI_CHIP_HASWELL_CRW_M_GT1		0x0D16 /* Mobile */
#define	PCI_CHIP_HASWELL_CRW_M_GT2		0x0D26
#define	PCI_CHIP_HASWELL_CRW_M_GT2_PLUS		0x0D36
#define	PCI_CHIP_HASWELL_CRW_S_GT1		0x0D1A /* Server */
#define	PCI_CHIP_HASWELL_CRW_S_GT2		0x0D2A
#define	PCI_CHIP_HASWELL_CRW_S_GT2_PLUS		0x0D3A

#define IS_G45(devid)           (devid == PCI_CHIP_IGD_E_G ||   \
                                 devid == PCI_CHIP_Q45_G ||     \
                                 devid == PCI_CHIP_G45_G ||     \
                                 devid == PCI_CHIP_G41_G ||     \
                                 devid == PCI_CHIP_B43_G ||     \
                                 devid == PCI_CHIP_B43_G1)
 
#define IS_GM45(devid)          (devid == PCI_CHIP_GM45_GM)
#define IS_G4X(devid)		(IS_G45(devid) || IS_GM45(devid))

#define IS_IRONLAKE_D(devid)    (devid == PCI_CHIP_IRONLAKE_D_G)
#define IS_IRONLAKE_M(devid)    (devid == PCI_CHIP_IRONLAKE_M_G)
#define IS_IRONLAKE(devid)      (IS_IRONLAKE_D(devid) || IS_IRONLAKE_M(devid))

#define IS_SNB_GT1(devid)       (devid == PCI_CHIP_SANDYBRIDGE_GT1 ||   \
                                 devid == PCI_CHIP_SANDYBRIDGE_M_GT1 || \
                                 devid == PCI_CHIP_SANDYBRIDGE_S_GT)

#define IS_SNB_GT2(devid)       (devid == PCI_CHIP_SANDYBRIDGE_GT2 ||   \
                                 devid == PCI_CHIP_SANDYBRIDGE_GT2_PLUS || \
                                 devid == PCI_CHIP_SANDYBRIDGE_M_GT2 || \
                                 devid == PCI_CHIP_SANDYBRIDGE_M_GT2_PLUS)

#define IS_GEN6(devid)          (IS_SNB_GT1(devid) ||   \
                                 IS_SNB_GT2(devid))

#define IS_IVB_GT1(devid)       (devid == PCI_CHIP_IVYBRIDGE_GT1 ||     \
                                 devid == PCI_CHIP_IVYBRIDGE_M_GT1 ||   \
                                 devid == PCI_CHIP_IVYBRIDGE_S_GT1)

#define IS_IVB_GT2(devid)       (devid == PCI_CHIP_IVYBRIDGE_GT2 ||     \
                                 devid == PCI_CHIP_IVYBRIDGE_M_GT2 ||   \
                                 devid == PCI_CHIP_IVYBRIDGE_S_GT2)

#define IS_IVYBRIDGE(devid)     (IS_IVB_GT1(devid) ||   \
                                 IS_IVB_GT2(devid))

#define IS_HSW_GT1(devid)   	(devid == PCI_CHIP_HASWELL_GT1		|| \
                                 devid == PCI_CHIP_HASWELL_M_GT1	|| \
                                 devid == PCI_CHIP_HASWELL_S_GT1	|| \
                                 devid == PCI_CHIP_HASWELL_SDV_GT1	|| \
                                 devid == PCI_CHIP_HASWELL_SDV_M_GT1	|| \
                                 devid == PCI_CHIP_HASWELL_SDV_S_GT1	|| \
                                 devid == PCI_CHIP_HASWELL_CRW_GT1	|| \
                                 devid == PCI_CHIP_HASWELL_CRW_M_GT1	|| \
                                 devid == PCI_CHIP_HASWELL_CRW_S_GT1    || \
                                 devid == PCI_CHIP_HASWELL_ULT_GT1	|| \
                                 devid == PCI_CHIP_HASWELL_ULT_M_GT1	|| \
                                 devid == PCI_CHIP_HASWELL_ULT_S_GT1)

#define IS_HSW_GT2(devid)   	(devid == PCI_CHIP_HASWELL_GT2||        \
                                 devid == PCI_CHIP_HASWELL_M_GT2||      \
                                 devid == PCI_CHIP_HASWELL_S_GT2||      \
                                 devid == PCI_CHIP_HASWELL_SDV_GT2||    \
                                 devid == PCI_CHIP_HASWELL_SDV_M_GT2||  \
                                 devid == PCI_CHIP_HASWELL_SDV_S_GT2||  \
                                 devid == PCI_CHIP_HASWELL_CRW_GT2||    \
                                 devid == PCI_CHIP_HASWELL_CRW_M_GT2||  \
                                 devid == PCI_CHIP_HASWELL_CRW_S_GT2||  \
                                 devid == PCI_CHIP_HASWELL_ULT_GT2||    \
                                 devid == PCI_CHIP_HASWELL_ULT_GT2_PLUS|| \
                                 devid == PCI_CHIP_HASWELL_ULT_M_GT2||  \
                                 devid == PCI_CHIP_HASWELL_ULT_M_GT2_PLUS|| \
                                 devid == PCI_CHIP_HASWELL_ULT_S_GT2    || \
                                 devid == PCI_CHIP_HASWELL_ULT_S_GT2_PLUS || \
                                 devid == PCI_CHIP_HASWELL_GT2_PLUS||   \
                                 devid == PCI_CHIP_HASWELL_M_GT2_PLUS    || \
                                 devid == PCI_CHIP_HASWELL_S_GT2_PLUS           || \
                                 devid == PCI_CHIP_HASWELL_SDV_GT2_PLUS|| \
                                 devid == PCI_CHIP_HASWELL_SDV_M_GT2_PLUS|| \
                                 devid == PCI_CHIP_HASWELL_SDV_S_GT2_PLUS|| \
                                 devid == PCI_CHIP_HASWELL_CRW_GT2_PLUS|| \
                                 devid == PCI_CHIP_HASWELL_CRW_M_GT2_PLUS|| \
                                 devid == PCI_CHIP_HASWELL_CRW_S_GT2_PLUS)

#define IS_HASWELL(devid)       (IS_HSW_GT1(devid) || \
                                 IS_HSW_GT2(devid))

#define IS_GEN7(devid)          (IS_IVYBRIDGE(devid) || \
                                 IS_HASWELL(devid))

#ifndef I915_EXEC_VEBOX
#define I915_EXEC_VEBOX         4
#endif

#endif /* _INTEL_DRIVER_H_ */
