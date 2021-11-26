#ifndef TRACE_H_INCLUDED
#define TRACE_H_INCLUDED

#include <inttypes.h>

extern uint32_t coverage;

#define COVERAGE_ON() do { trace_resume(coverage); } while (0)

#define COVERAGE_OFF() do { coverage = trace_pause(); } while (0)

void trace_begin(void);
void trace_end(void);
uint32_t trace_pause(void);
void trace_resume(uint32_t value);

#endif
