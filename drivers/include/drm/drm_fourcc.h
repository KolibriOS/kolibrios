#ifndef _DRM_FOURCC_SHIM_H
#define _DRM_FOURCC_SHIM_H
/*
 * The tree keeps the format list in uapi/ and the in-kernel helpers in
 * drm_crtc.h.  Upstream drivers include <drm/drm_fourcc.h>; give them that
 * spelling instead of patching every call site.  The guard is deliberately
 * distinct from the uapi header's own.
 */
#include <uapi/drm/drm_fourcc.h>
#endif
