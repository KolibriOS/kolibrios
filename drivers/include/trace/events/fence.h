#ifndef _TRACE_EVENTS_FENCE_H
#define _TRACE_EVENTS_FENCE_H
/* Tracepoints are compiled out; nouveau_fence.c is the only includer. */
struct fence;
static inline void trace_fence_emit(struct fence *fence) { }
#endif
