/* rwsem.h: R/W semaphores implemented using XADD/CMPXCHG for i486+
 *
 * Written by David Howells (dhowells@redhat.com).
 *
 * Derived from asm-x86/semaphore.h
 *
 *
 * The MSW of the count is the negated number of active writers and waiting
 * lockers, and the LSW is the total number of active locks
 *
 * The lock count is initialized to 0 (no active and no waiting lockers).
 *
 * When a writer subtracts WRITE_BIAS, it'll get 0xffff0001 for the case of an
 * uncontended lock. This can be determined because XADD returns the old value.
 * Readers increment by 1 and see a positive value when uncontended, negative
 * if there are writers (and maybe) readers waiting (in which case it goes to
 * sleep).
 *
 * The value of WAITING_BIAS supports up to 32766 waiting processes. This can
 * be extended to 65534 by manually checking the whole MSW rather than relying
 * on the S flag.
 *
 * The value of ACTIVE_BIAS supports up to 65535 active processes.
 *
 * This should be totally fair - if anything is waiting, a process that wants a
 * lock will go to the back of the queue. When the currently active lock is
 * released, if there's a writer at the front of the queue, then that and only
 * that will be woken up; if there's a bunch of consequtive readers at the
 * front, then they'll all be woken up, but no other readers will be.
 */

#ifndef _ASM_X86_RWSEM_H
#define _ASM_X86_RWSEM_H

#ifndef _LINUX_RWSEM_H
#error "please don't include asm/rwsem.h directly, use linux/rwsem.h instead"
#endif

#ifdef __KERNEL__
#include <asm/asm.h>

#define FASTCALL __attribute__ ((fastcall)) __attribute__ ((dllimport))

void  FASTCALL DownRead(struct rw_semaphore *sem)__asm__("DownRead");
void  FASTCALL DownWrite(struct rw_semaphore *sem)__asm__("DownWrite");
void  FASTCALL UpRead(struct rw_semaphore *sem)__asm__("UpRead");
void  FASTCALL UpWrite(struct rw_semaphore *sem)__asm__("UpWrite");

/*
 * lock for reading
 */
static inline void __down_read(struct rw_semaphore *sem)
{
    DownRead(sem);
}

static inline void __down_write(struct rw_semaphore *sem)
{
    DownWrite(sem);
}

/*
 * unlock after reading
 */
static inline void __up_read(struct rw_semaphore *sem)
{
    UpRead(sem);
}

/*
 * unlock after writing
 */
static inline void __up_write(struct rw_semaphore *sem)
{
    UpWrite(sem);
}

#endif /* __KERNEL__ */
#endif /* _ASM_X86_RWSEM_H */
