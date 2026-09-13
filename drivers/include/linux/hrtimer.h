#ifndef _LINUX_HRTIMER_H
#define _LINUX_HRTIMER_H
/*
 * amdgpu_mode.h only embeds a struct hrtimer in the virtual-DCE crtc and never
 * arms it on this port, so a structural stub is enough.
 */
#include <linux/types.h>
#include <linux/ktime.h>

enum hrtimer_restart { HRTIMER_NORESTART, HRTIMER_RESTART };
enum hrtimer_mode {
	HRTIMER_MODE_ABS	= 0x0,
	HRTIMER_MODE_REL	= 0x1,
	HRTIMER_MODE_PINNED	= 0x2,
};
enum hrtimer_base_type { HRTIMER_BASE_MONOTONIC, HRTIMER_MAX_CLOCK_BASES };

struct hrtimer {
	ktime_t	_softexpires;
	enum hrtimer_restart (*function)(struct hrtimer *);
	int	state;
};

static inline void hrtimer_init(struct hrtimer *timer, int which, enum hrtimer_mode mode)
	{ timer->function = NULL; timer->state = 0; }
static inline int  hrtimer_start(struct hrtimer *t, ktime_t tim, enum hrtimer_mode m) { return 0; }
static inline int  hrtimer_cancel(struct hrtimer *t) { return 0; }
static inline int  hrtimer_try_to_cancel(struct hrtimer *t) { return 0; }
static inline bool hrtimer_active(const struct hrtimer *t) { return false; }
static inline void hrtimer_start_range_ns(struct hrtimer *t, ktime_t tim,
					  u64 range_ns, enum hrtimer_mode m) { }

/* Sleeps through the DDK's delay(); see the driver's kos_* glue. */
extern int schedule_hrtimeout(ktime_t *expires, const enum hrtimer_mode mode);
#endif
