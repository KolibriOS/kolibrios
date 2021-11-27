#ifndef TRACE_H_INCLUDED
#define TRACE_H_INCLUDED

#include <inttypes.h>

extern uint32_t coverage;

#define COVERAGE_ON() 

#define COVERAGE_OFF()

void trace_begin(void);
void trace_end(void);
uint32_t trace_pause(void);
void trace_resume(uint32_t value);

#endif
