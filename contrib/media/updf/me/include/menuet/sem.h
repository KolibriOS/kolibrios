#ifndef __MENUETOS_SEM_H
#define __MENUETOS_SEM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef volatile struct {
 unsigned long s_magic;
 volatile unsigned long s_value;
 volatile unsigned long s_lock;
} semaphore_t;

#define SEM_MAGIC	0x1BAD2E20

#define SEM_INIT	{ SEM_MAGIC , 0 , 0 }

static inline semaphore_t * get_sem_init(void)
{
 static semaphore_t __ret=SEM_INIT;
 return &__ret;
}

static inline void fill_empty_sem(semaphore_t * sem)
{
 if(sem)
 {
  //memcpy((void *)sem,(const void *)get_sem_init(),sizeof(semaphore_t));
 }
}
  
#define DECLARE_SEMAPHORE(name)	  semaphore_t name = SEM_INIT ;
#define DECLARE_STATIC_SEM(name)  static semaphore_t name = SEM_INIT ;
#define DECLARE_SEMAPHORE_S(name) semaphore_t name

#define check_sem_magic(s)	((s)->s_magic==SEM_MAGIC)

#define SEM_VAL(s)	(s)->s_value
#define SEM_LOCK(s)	(s)->s_lock

#define sem_lock_acquire(sem) \
    { \
LOCKED: \
	if(check_sem_magic(sem)) \
	{ \
	    if(SEM_LOCK(sem)) \
	    { \
		__menuet__delay100(1); \
		goto LOCKED; \
	    } \
	} \
    }
    
#define sem_lock(sem) \
    { \
	if(check_sem_magic(sem)) \
	{ \
	    sem_lock_acquire(sem); \
	    SEM_LOCK(sem)++; \
	} \
    }
    
#define sem_unlock(sem) \
    { \
	if(check_sem_magic(sem)) \
	{ \
	    if(SEM_LOCK(sem)) SEM_LOCK(sem)--; \
	} \
    }
    
#define sem_read(sem) \
    ({ \
	volatile unsigned long __ret; \
	if(check_sem_magic(sem)) \
	{ \
	    sem_lock(sem); \
	    __ret=SEM_VAL(sem); \
	    sem_unlock(sem); \
	} else { \
	    __ret=0; \
	} \
	__ret; \
    })

#define sem_write(sem,val) \
    { \
	if(check_sem_magic(sem)) \
	{ \
	    sem_lock(sem); \
	    SEM_VAL(sem)=(val); \
	    sem_unlock(sem); \
	} \
    }
    
#define sem_up(sem) \
    { \
	if(check_sem_magic(sem)) \
	{ \
	    sem_lock(sem); \
	    SEM_VAL(sem)++; \
	    sem_unlock(sem); \
	} \
    }
    
#define sem_down(sem) \
    { \
	if(check_sem_magic(sem)) \
	{ \
	    sem_lock(sem); \
	    if(SEM_VAL(sem)) SEM_VAL(sem)--; \
	    sem_unlock(sem); \
	} \
    }
    
#define sem_isup(sem) \
    { \
	int __ret; \
	if(check_sem_magic(sem)) \
	{ \
	    sem_lock(sem); \
	    __ret=SEM_VAL(sem) ? 1 : 0; \
	    sem_unlock(sem); \
	} \
	__ret; \
    }
    
#define sem_isdown(sem) \
    { \
	int __ret; \
	if(check_sem_magic(sem)) \
	{ \
	    sem_lock(sem); \
	    __ret=SEM_VAL(sem) ? 0 : 1; \
	    sem_unlock(sem); \
	} \
	__ret; \
    }

#ifdef __cplusplus
}
#endif
    
#endif
