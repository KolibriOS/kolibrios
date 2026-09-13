#ifndef _DRM_MODE_SHIM_H
#define _DRM_MODE_SHIM_H
/*
 * Same reason as drm_fourcc.h: upstream drivers spell this <drm/drm_mode.h>
 * while the tree keeps it under uapi/.  The guard deliberately does not match
 * the uapi header's own _DRM_MODE_H, or including this first would suppress it.
 */
#include <uapi/drm/drm_mode.h>
#endif
