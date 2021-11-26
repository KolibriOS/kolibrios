#include "trace_lbr.h"

uint32_t coverage;

void trace_begin() {
    trace_lbr_begin();
}

void trace_end() {
    trace_lbr_end();
}

uint32_t trace_pause(void) {
    return trace_lbr_pause();
}

void trace_resume(uint32_t value) {
    trace_lbr_resume(value);
}
