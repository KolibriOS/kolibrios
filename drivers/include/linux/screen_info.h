#ifndef _LINUX_SCREEN_INFO_H
#define _LINUX_SCREEN_INFO_H
/*
 * nouveau_fbcon.c only uses this to decide whether it owns the boot console.
 * KolibriOS has no boot console to hand over, so the record stays empty.
 */
#include <linux/types.h>

struct screen_info {
	__u8  orig_video_isVGA;
	__u16 lfb_width;
	__u16 lfb_height;
	__u16 lfb_depth;
	__u32 lfb_base;
	__u32 lfb_size;
};

extern struct screen_info screen_info;
#endif
