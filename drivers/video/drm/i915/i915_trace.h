#if !defined(_I915_TRACE_H_) || defined(TRACE_HEADER_MULTI_READ)
#define _I915_TRACE_H_

//#include <linux/stringify.h>
#include <linux/types.h>
//#include <linux/tracepoint.h>


#define trace_i915_gem_object_create(x)
#define trace_i915_gem_object_destroy(x)
#define trace_i915_gem_object_change_domain(a, b, c)
#define trace_i915_gem_object_unbind(x)
#define trace_i915_gem_ring_flush(a, b, c)
#define trace_i915_gem_object_bind(a, b)
#define trace_i915_ring_wait_end(x)

#endif
