#ifndef __SYS_LOCK_H__
#define __SYS_LOCK_H__

#define  _LIBC  1
#define  NOT_IN_libc 1

#ifndef __USE_GNU
#define __USE_GNU 1
#endif

void __mutex_lock(volatile int *val);

typedef volatile int __libc_lock_t;
typedef struct { volatile int mutex; } __libc_lock_recursive_t;

#define __libc_lock_define(CLASS,NAME) \
  CLASS __libc_lock_t NAME;
#define __libc_rwlock_define(CLASS,NAME) \
  CLASS __libc_rwlock_t NAME;
#define __libc_lock_define_recursive(CLASS,NAME) \
  CLASS __libc_lock_recursive_t NAME;
#define __rtld_lock_define_recursive(CLASS,NAME) \
  CLASS __rtld_lock_recursive_t NAME;

typedef __libc_lock_t _LOCK_T;
typedef __libc_lock_recursive_t _LOCK_RECURSIVE_T;

#define __LOCK_INIT(class,lock) \
  __libc_lock_define_initialized(class, lock)

#define __LOCK_INIT_RECURSIVE(class, lock) \
  __libc_lock_define_initialized_recursive(class, lock)

#define __libc_lock_define_initialized(CLASS,NAME) \
  CLASS __libc_lock_t NAME;

#define __libc_lock_define_initialized_recursive(CLASS,NAME) \
  CLASS __libc_lock_recursive_t NAME = _LIBC_LOCK_RECURSIVE_INITIALIZER;

#define _LIBC_LOCK_RECURSIVE_INITIALIZER {0}

#define __lock_init(__lock) __libc_lock_init(__lock)
#define __lock_init_recursive(__lock) __libc_lock_init_recursive(__lock)
#define __lock_acquire(__lock) __libc_lock_lock(__lock)
#define __lock_acquire_recursive(__lock) __libc_lock_lock_recursive(__lock)
#define __lock_release(__lock) __libc_lock_unlock(__lock)
#define __lock_release_recursive(__lock) __libc_lock_unlock_recursive(__lock)
#define __lock_try_acquire(__lock) __libc_lock_trylock(__lock)
#define __lock_try_acquire_recursive(__lock) \
	__libc_lock_trylock_recursive(__lock)
#define __lock_close(__lock) __libc_lock_fini(__lock)
#define __lock_close_recursive(__lock) __libc_lock_fini_recursive(__lock)


#define __libc_lock_init_recursive(NAME) ((NAME).mutex=0)
#define __libc_lock_fini(NAME)

#define __libc_lock_fini_recursive(NAME) __libc_lock_fini ((NAME).mutex)


#define __libc_lock_lock(NAME) __mutex_lock (&(NAME))

/* Lock the recursive named lock variable.  */
#define __libc_lock_lock_recursive(NAME) __libc_lock_lock ((NAME).mutex)

#define __libc_lock_unlock(NAME) ((NAME)=0)
#define __libc_lock_unlock_recursive(NAME) __libc_lock_unlock ((NAME).mutex)

#endif /* __SYS_LOCK_H__ */
