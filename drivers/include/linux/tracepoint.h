#ifndef _LINUX_TRACEPOINT_H
#define _LINUX_TRACEPOINT_H
/* Tracepoints compile away: amdgpu_trace.h expands to empty trace_*() calls. */
/*
 * Each event still has to yield a callable trace_<name>(), because the driver
 * code calls it unconditionally - upstream generates one per event.  Here they
 * expand to empty inlines, so the calls disappear at -O2 but the argument
 * expressions are still type-checked.
 */
#define TP_PROTO(args...)	args
#define TP_ARGS(args...)	args
#define TP_STRUCT__entry(args...)
#define TP_fast_assign(args...)
#define TP_printk(fmt, args...)
#define TP_CONDITION(args...)

#define DECLARE_EVENT_CLASS(name, proto, args, tstruct, assign, print)

#define DEFINE_EVENT(tmpl, name, proto, args)				\
	static inline void trace_##name(proto) { }			\
	static inline bool trace_##name##_enabled(void) { return false; }

#define TRACE_EVENT(name, proto, args, tstruct, assign, print)		\
	static inline void trace_##name(proto) { }			\
	static inline bool trace_##name##_enabled(void) { return false; }

#define TRACE_EVENT_CONDITION(name, proto, args, cond, tstruct, assign, print) \
	static inline void trace_##name(proto) { }			\
	static inline bool trace_##name##_enabled(void) { return false; }

#define DECLARE_TRACE(name, proto, args)				\
	static inline void trace_##name(proto) { }
#define EXPORT_TRACEPOINT_SYMBOL_GPL(name)
#define EXPORT_TRACEPOINT_SYMBOL(name)
#endif
