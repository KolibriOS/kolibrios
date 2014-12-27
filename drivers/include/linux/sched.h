#ifndef _LINUX_SCHED_H
#define _LINUX_SCHED_H


#define TASK_UNINTERRUPTIBLE    2
/* Task command name length */
#define TASK_COMM_LEN 16 

#define schedule_timeout(x)  delay(x)
#define MAX_SCHEDULE_TIMEOUT    LONG_MAX

#endif
