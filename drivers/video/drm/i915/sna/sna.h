
#define FALSE 0
#define TRUE  1

#define DBG(x)
//#define DBG(x) dbgprintf x

#define assert(x)


#include "compiler.h"
#include <linux/kernel.h>

struct pixman_box16
{
    int16_t x1, y1, x2, y2;
};

typedef struct pixman_box16 BoxRec;
typedef unsigned int CARD32;

#include "sna_render.h"
#include "kgem.h"

#define PictOpClear             0
#define PictOpSrc               1
#define PictOpDst               2
#define PictOpOver              3
#define PictOpOverReverse       4
#define PictOpIn                5
#define PictOpInReverse         6
#define PictOpOut               7
#define PictOpOutReverse        8
#define PictOpAtop              9
#define PictOpAtopReverse       10
#define PictOpXor               11
#define PictOpAdd               12
#define PictOpSaturate          13
#define PictOpMaximum           13

struct sna {
    unsigned flags;
#define SNA_NO_THROTTLE         0x1
#define SNA_NO_DELAYED_FLUSH    0x2

//    int timer[NUM_TIMERS];

//    uint16_t timer_active;
//    uint16_t timer_ready;

//    int vblank_interval;

//    struct list deferred_free;
//    struct list dirty_pixmaps;
//    struct list active_pixmaps;
//    struct list inactive_clock[2];

    unsigned int tiling;
#define SNA_TILING_DISABLE  0x0
#define SNA_TILING_FB       0x1
#define SNA_TILING_2D       0x2
#define SNA_TILING_3D       0x4
#define SNA_TILING_ALL     (~0)

    int Chipset;
//    EntityInfoPtr pEnt;
//    struct pci_device *PciInfo;
//    struct intel_chipset chipset;

//    PicturePtr clear;
    struct {
        uint32_t fill_bo;
        uint32_t fill_pixel;
        uint32_t fill_alu;
    } blt_state;
    union {
//        struct gen2_render_state gen2;
//        struct gen3_render_state gen3;
//        struct gen4_render_state gen4;
//        struct gen5_render_state gen5;
        struct gen6_render_state gen6;
//        struct gen7_render_state gen7;
    } render_state;
    uint32_t have_render;
    uint32_t default_tiling;

//    Bool directRenderingOpen;
//    char *deviceName;

    /* Broken-out options. */
//    OptionInfoPtr Options;

    /* Driver phase/state information */
//    Bool suspended;

    struct kgem kgem;
    struct sna_render render;
};

static inline int vertex_space(struct sna *sna)
{
    return sna->render.vertex_size - sna->render.vertex_used;
}

static inline void vertex_emit(struct sna *sna, float v)
{
    assert(sna->render.vertex_used < sna->render.vertex_size);
    sna->render.vertices[sna->render.vertex_used++] = v;
}

static inline void vertex_emit_2s(struct sna *sna, int16_t x, int16_t y)
{
    int16_t *v = (int16_t *)&sna->render.vertices[sna->render.vertex_used++];
    assert(sna->render.vertex_used <= sna->render.vertex_size);
    v[0] = x;
    v[1] = y;
}

static inline void batch_emit(struct sna *sna, uint32_t dword)
{
    assert(sna->kgem.mode != KGEM_NONE);
    assert(sna->kgem.nbatch + KGEM_BATCH_RESERVED < sna->kgem.surface);
    sna->kgem.batch[sna->kgem.nbatch++] = dword;
}

