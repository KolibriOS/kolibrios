

/** enumeration of 3d consumers so some can maintain invariant state. */
enum last_3d {
    LAST_3D_OTHER,
    LAST_3D_VIDEO,
    LAST_3D_RENDER,
    LAST_3D_ROTATION
};



typedef struct intel_screen_private {
    int cpp;

#define RENDER_BATCH                I915_EXEC_RENDER
#define BLT_BATCH                   I915_EXEC_BLT

    unsigned int current_batch;

    dri_bufmgr *bufmgr;

    uint32_t batch_ptr[4096];
    /** Byte offset in batch_ptr for the next dword to be emitted. */
    unsigned int batch_used;
    /** Position in batch_ptr at the start of the current BEGIN_BATCH */
    unsigned int batch_emit_start;
    /** Number of bytes to be emitted in the current BEGIN_BATCH. */
    uint32_t batch_emitting;
    dri_bo *batch_bo, *last_batch_bo[2];
    /** Whether we're in a section of code that can't tolerate flushing */
    Bool in_batch_atomic;
    /** Ending batch_used that was verified by intel_start_batch_atomic() */
    int batch_atomic_limit;
    struct list batch_pixmaps;
    drm_intel_bo *wa_scratch_bo;

    unsigned int tiling;

#define INTEL_TILING_FB     0x1
#define INTEL_TILING_2D     0x2
#define INTEL_TILING_3D     0x4
#define INTEL_TILING_ALL   (~0)

    Bool has_relaxed_fencing;

    int Chipset;

    unsigned int BR[20];

    void (*vertex_flush) (struct intel_screen_private *intel);
    void (*batch_flush) (struct intel_screen_private *intel);
    void (*batch_commit_notify) (struct intel_screen_private *intel);

    Bool need_sync;

    int accel_pixmap_offset_alignment;
    int accel_max_x;
    int accel_max_y;
    int max_bo_size;
    int max_gtt_map_size;
    int max_tiling_size;

    struct {
        drm_intel_bo *gen4_vs_bo;
        drm_intel_bo *gen4_sf_bo;
        drm_intel_bo *gen4_wm_packed_bo;
        drm_intel_bo *gen4_wm_planar_bo;
        drm_intel_bo *gen4_cc_bo;
        drm_intel_bo *gen4_cc_vp_bo;
        drm_intel_bo *gen4_sampler_bo;
        drm_intel_bo *gen4_sip_kernel_bo;
        drm_intel_bo *wm_prog_packed_bo;
        drm_intel_bo *wm_prog_planar_bo;
        drm_intel_bo *gen6_blend_bo;
        drm_intel_bo *gen6_depth_stencil_bo;
    } video;

    /* Render accel state */
    float scale_units[2][2];
    /** Transform pointers for src/mask, or NULL if identity */
    PictTransform *transform[2];

    PixmapPtr render_source, render_mask, render_dest;
    PicturePtr render_source_picture, render_mask_picture, render_dest_picture;
    Bool needs_3d_invariant;
    Bool needs_render_state_emit;
    Bool needs_render_vertex_emit;

    /* i830 render accel state */
    uint32_t render_dest_format;
    uint32_t cblend, ablend, s8_blendctl;

    /* i915 render accel state */
    PixmapPtr texture[2];
    uint32_t mapstate[6];
    uint32_t samplerstate[6];

    struct {
        int op;
        uint32_t dst_format;
    } i915_render_state;

    struct {
        int num_sf_outputs;
        int drawrect;
        uint32_t blend;
        dri_bo *samplers;
        dri_bo *kernel;
    } gen6_render_state;

    uint32_t prim_offset;
    void (*prim_emit)(struct intel_screen_private *intel,
              int srcX, int srcY,
              int maskX, int maskY,
              int dstX, int dstY,
              int w, int h);
    int floats_per_vertex;
    int last_floats_per_vertex;
    uint16_t vertex_offset;
    uint16_t vertex_count;
    uint16_t vertex_index;
    uint16_t vertex_used;
    uint32_t vertex_id;
    float vertex_ptr[4*1024];
    dri_bo *vertex_bo;

    uint8_t surface_data[16*1024];
    uint16_t surface_used;
    uint16_t surface_table;
    uint32_t surface_reloc;
    dri_bo *surface_bo;

    /* 965 render acceleration state */
    struct gen4_render_state *gen4_render_state;

    Bool use_pageflipping;
    Bool use_triple_buffer;
    Bool force_fallback;
    Bool has_kernel_flush;
    Bool needs_flush;

    enum last_3d last_3d;

    /**
     * User option to print acceleration fallback info to the server log.
     */
    Bool fallback_debug;
    unsigned debug_flush;
    Bool has_prime_vmap_flush;
} intel_screen_private;

