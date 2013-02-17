#ifndef SNA_REG_H
#define SNA_REG_H

/* Flush */
#define MI_FLUSH			(0x04<<23)
#define MI_FLUSH_DW			(0x26<<23)

#define MI_WRITE_DIRTY_STATE		(1<<4)
#define MI_END_SCENE			(1<<3)
#define MI_GLOBAL_SNAPSHOT_COUNT_RESET	(1<<3)
#define MI_INHIBIT_RENDER_CACHE_FLUSH	(1<<2)
#define MI_STATE_INSTRUCTION_CACHE_FLUSH (1<<1)
#define MI_INVALIDATE_MAP_CACHE		(1<<0)
/* broadwater flush bits */
#define BRW_MI_GLOBAL_SNAPSHOT_RESET   (1 << 3)

#define MI_BATCH_BUFFER_END	(0xA << 23)

/* Noop */
#define MI_NOOP				0x00
#define MI_NOOP_WRITE_ID		(1<<22)
#define MI_NOOP_ID_MASK			(1<<22 - 1)

/* Wait for Events */
#define MI_WAIT_FOR_EVENT			(0x03<<23)
#define MI_WAIT_FOR_PIPEB_SVBLANK		(1<<18)
#define MI_WAIT_FOR_PIPEA_SVBLANK		(1<<17)
#define MI_WAIT_FOR_OVERLAY_FLIP		(1<<16)
#define MI_WAIT_FOR_PIPEB_VBLANK		(1<<7)
#define MI_WAIT_FOR_PIPEB_SCAN_LINE_WINDOW	(1<<5)
#define MI_WAIT_FOR_PIPEA_VBLANK		(1<<3)
#define MI_WAIT_FOR_PIPEA_SCAN_LINE_WINDOW	(1<<1)

/* Set the scan line for MI_WAIT_FOR_PIPE?_SCAN_LINE_WINDOW */
#define MI_LOAD_SCAN_LINES_INCL			(0x12<<23)
#define MI_LOAD_SCAN_LINES_DISPLAY_PIPEA	(0)
#define MI_LOAD_SCAN_LINES_DISPLAY_PIPEB	(0x1<<20)

/* BLT commands */
#define BLT_WRITE_ALPHA		(1<<21)
#define BLT_WRITE_RGB		(1<<20)
#define BLT_SRC_TILED		(1<<15)
#define BLT_DST_TILED		(1<<11)

#define COLOR_BLT_CMD			((2<<29)|(0x40<<22)|(0x3))
#define XY_COLOR_BLT			((2<<29)|(0x50<<22)|(0x4))
#define XY_SETUP_BLT			((2<<29)|(1<<22)|6)
#define XY_SETUP_MONO_PATTERN_SL_BLT	((2<<29)|(0x11<<22)|7)
#define XY_SETUP_CLIP			((2<<29)|(3<<22)|1)
#define XY_SCANLINE_BLT			((2<<29)|(0x25<<22)|1)
#define XY_TEXT_IMMEDIATE_BLT		((2<<29)|(0x31<<22)|(1<<16))
#define XY_SRC_COPY_BLT_CMD		((2<<29)|(0x53<<22)|6)
#define SRC_COPY_BLT_CMD		((2<<29)|(0x43<<22)|0x4)
#define XY_PAT_BLT_IMMEDIATE		((2<<29)|(0x72<<22))
#define XY_MONO_PAT			((0x2<<29)|(0x52<<22)|0x7)
#define XY_MONO_SRC_COPY		((0x2<<29)|(0x54<<22)|(0x6))
#define XY_MONO_SRC_COPY_IMM		((0x2<<29)|(0x71<<22))
#define XY_FULL_MONO_PATTERN_BLT	((0x2<<29)|(0x57<<22)|0xa)
#define XY_FULL_MONO_PATTERN_MONO_SRC_BLT	((0x2<<29)|(0x58<<22)|0xa)

/* FLUSH commands */
#define BRW_3D(Pipeline,Opcode,Subopcode) \
	((3 << 29) | \
	 ((Pipeline) << 27) | \
	 ((Opcode) << 24) | \
	 ((Subopcode) << 16))
#define PIPE_CONTROL		BRW_3D(3, 2, 0)
#define PIPE_CONTROL_NOWRITE       (0 << 14)
#define PIPE_CONTROL_WRITE_QWORD   (1 << 14)
#define PIPE_CONTROL_WRITE_DEPTH   (2 << 14)
#define PIPE_CONTROL_WRITE_TIME    (3 << 14)
#define PIPE_CONTROL_DEPTH_STALL   (1 << 13)
#define PIPE_CONTROL_WC_FLUSH      (1 << 12)
#define PIPE_CONTROL_IS_FLUSH      (1 << 11)
#define PIPE_CONTROL_TC_FLUSH      (1 << 10)
#define PIPE_CONTROL_NOTIFY_ENABLE (1 << 8)
#define PIPE_CONTROL_GLOBAL_GTT    (1 << 2)
#define PIPE_CONTROL_LOCAL_PGTT    (0 << 2)
#define PIPE_CONTROL_DEPTH_CACHE_FLUSH	(1 << 0)

#endif
