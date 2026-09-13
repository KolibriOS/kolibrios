#ifndef _LINUX_KTHREAD_H
#define _LINUX_KTHREAD_H
/*
 * Minimal kthread emulation for KolibriOS.
 *
 * The GPU scheduler (drivers/video/drm/amdgpu/scheduler) is the only user in
 * the DRM tree.  KolibriOS kernel threads are created with CreateKernelThread(),
 * which takes no argument, so kthread_run() stashes the payload in a small
 * pid-indexed table and the trampoline picks it up.  Park/stop are cooperative
 * flags polled by the thread body, matching how the scheduler uses them.
 */

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/rwsem.h>

/*
 * Placeholder address space.  Drivers reach it as current->mm on their userptr
 * paths only, which KolibriOS never enters (no ioctl surface for user code).
 */
struct mm_struct {
	struct rw_semaphore mmap_sem;
};

struct task_struct {
	int	(*threadfn)(void *data);
	void	 *data;
	int	  pid;
	volatile int should_stop;
	volatile int should_park;
	volatile int parked;
	char	  comm[16];
	struct mm_struct *mm;
};

struct task_struct *kthread_create_kos(int (*threadfn)(void *data),
				       void *data, const char *name);
int  kthread_stop(struct task_struct *k);
bool kthread_should_stop(void);
bool kthread_should_park(void);
void kthread_parkme(void);
int  kthread_park(struct task_struct *k);
void kthread_unpark(struct task_struct *k);
struct task_struct *kthread_current(void);

#define kthread_run(threadfn, data, namefmt, ...) \
	kthread_create_kos((threadfn), (data), (namefmt))

#define kthread_create(threadfn, data, namefmt, ...) \
	kthread_create_kos((threadfn), (data), (namefmt))

static inline void wake_up_process(struct task_struct *k) { }
static inline void get_task_struct(struct task_struct *k) { }
static inline void put_task_struct(struct task_struct *k) { }

#endif /* _LINUX_KTHREAD_H */
