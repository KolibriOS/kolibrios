
#ifndef R300_PUBLIC_H
#define R300_PUBLIC_H

#ifdef __cplusplus
extern "C" {
#endif

struct radeon_winsys;

struct pipe_screen* r300_screen_create(struct radeon_winsys *rws);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
