#ifndef __KERNEL_PRINTK__
#define __KERNEL_PRINTK__

#include <stdarg.h>
#include <linux/init.h>
#include <linux/linkage.h>
#include <linux/cache.h>

extern const char linux_banner[];
extern const char linux_proc_banner[];

extern char *log_buf_addr_get(void);
extern u32 log_buf_len_get(void);

/* printk's without a loglevel use this.. */
#define MESSAGE_LOGLEVEL_DEFAULT CONFIG_MESSAGE_LOGLEVEL_DEFAULT

/* We show everything that is MORE important than this.. */
#define CONSOLE_LOGLEVEL_SILENT  0 /* Mum's the word */
#define CONSOLE_LOGLEVEL_MIN	 1 /* Minimum loglevel we let people use */
#define CONSOLE_LOGLEVEL_QUIET	 4 /* Shhh ..., when booted with "quiet" */
#define CONSOLE_LOGLEVEL_DEFAULT 7 /* anything MORE serious than KERN_DEBUG */
#define CONSOLE_LOGLEVEL_DEBUG	10 /* issue debug messages */
#define CONSOLE_LOGLEVEL_MOTORMOUTH 15	/* You can't shut this one up */

struct va_format {
	const char *fmt;
	va_list *va;
};

/*
 * FW_BUG
 * Add this to a message where you are sure the firmware is buggy or behaves
 * really stupid or out of spec. Be aware that the responsible BIOS developer
 * should be able to fix this issue or at least get a concrete idea of the
 * problem by reading your message without the need of looking at the kernel
 * code.
 *
 * Use it for definite and high priority BIOS bugs.
 *
 * FW_WARN
 * Use it for not that clear (e.g. could the kernel messed up things already?)
 * and medium priority BIOS bugs.
 *
 * FW_INFO
 * Use this one if you want to tell the user or vendor about something
 * suspicious, but generally harmless related to the firmware.
 *
 * Use it for information or very low priority BIOS bugs.
 */
#define FW_BUG		"[Firmware Bug]: "
#define FW_WARN		"[Firmware Warn]: "
#define FW_INFO		"[Firmware Info]: "

/*
 * HW_ERR
 * Add this to a message for hardware errors, so that user can report
 * it to hardware vendor instead of LKML or software vendor.
 */
#define HW_ERR		"[Hardware Error]: "

/*
 * DEPRECATED
 * Add this to a message whenever you want to warn user space about the use
 * of a deprecated aspect of an API so they can stop using it
 */
#define DEPRECATED	"[Deprecated]: "

/*
 * Dummy printk for disabled debugging statements to use whilst maintaining
 * gcc's format checking.
 */
#define no_printk(fmt, ...)			\
do {						\
	if (0)					\
		printk(fmt, ##__VA_ARGS__);	\
} while (0)



__printf(1, 2) int dbgprintf(const char *fmt, ...);

#define printk(fmt, arg...)    dbgprintf(fmt , ##arg)

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

#define pr_debug(fmt, ...) \
	printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)

/*
 * These can be used to print at the various log levels.
 * All of these will print unconditionally, although note that pr_debug()
 * and other debug macros are compiled out unless either DEBUG is defined
 * or CONFIG_DYNAMIC_DEBUG is set.
 */
#define pr_emerg(fmt, ...) \
	printk(KERN_EMERG pr_fmt(fmt), ##__VA_ARGS__)
#define pr_alert(fmt, ...) \
	printk(KERN_ALERT pr_fmt(fmt), ##__VA_ARGS__)
#define pr_crit(fmt, ...) \
	printk(KERN_CRIT pr_fmt(fmt), ##__VA_ARGS__)
#define pr_err(fmt, ...) \
	printk(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__)
#define pr_warning(fmt, ...) \
	printk(KERN_WARNING pr_fmt(fmt), ##__VA_ARGS__)
#define pr_warn pr_warning
#define pr_notice(fmt, ...) \
	printk(KERN_NOTICE pr_fmt(fmt), ##__VA_ARGS__)
#define pr_info(fmt, ...) \
	printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)
/*
 * Like KERN_CONT, pr_cont() should only be used when continuing
 * a line with no newline ('\n') enclosed. Otherwise it defaults
 * back to KERN_DEFAULT.
 */
#define pr_cont(fmt, ...) \
	printk(KERN_CONT fmt, ##__VA_ARGS__)

/* pr_devel() should produce zero code unless DEBUG is defined */
#ifdef DEBUG
#define pr_devel(fmt, ...) \
	printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#else
#define pr_devel(fmt, ...) \
	no_printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#endif

/*
 * Print a one-time message (analogous to WARN_ONCE() et al):
 */

#ifdef CONFIG_PRINTK
#define printk_once(fmt, ...)					\
({								\
	static bool __print_once __read_mostly;			\
								\
	if (!__print_once) {					\
		__print_once = true;				\
		printk(fmt, ##__VA_ARGS__);			\
	}							\
})
#define printk_deferred_once(fmt, ...)				\
({								\
	static bool __print_once __read_mostly;			\
								\
	if (!__print_once) {					\
		__print_once = true;				\
		printk_deferred(fmt, ##__VA_ARGS__);		\
	}							\
})
#else
#define printk_once(fmt, ...)					\
	no_printk(fmt, ##__VA_ARGS__)
#define printk_deferred_once(fmt, ...)				\
	no_printk(fmt, ##__VA_ARGS__)
#endif

#define pr_emerg_once(fmt, ...)					\
	printk_once(KERN_EMERG pr_fmt(fmt), ##__VA_ARGS__)
#define pr_alert_once(fmt, ...)					\
	printk_once(KERN_ALERT pr_fmt(fmt), ##__VA_ARGS__)
#define pr_crit_once(fmt, ...)					\
	printk_once(KERN_CRIT pr_fmt(fmt), ##__VA_ARGS__)
#define pr_err_once(fmt, ...)					\
	printk_once(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__)
#define pr_warn_once(fmt, ...)					\
	printk_once(KERN_WARNING pr_fmt(fmt), ##__VA_ARGS__)
#define pr_notice_once(fmt, ...)				\
	printk_once(KERN_NOTICE pr_fmt(fmt), ##__VA_ARGS__)
#define pr_info_once(fmt, ...)					\
	printk_once(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)
#define pr_cont_once(fmt, ...)					\
	printk_once(KERN_CONT pr_fmt(fmt), ##__VA_ARGS__)

#if defined(DEBUG)
#define pr_devel_once(fmt, ...)					\
	printk_once(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#else
#define pr_devel_once(fmt, ...)					\
	no_printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#endif

/* If you are writing a driver, please use dev_dbg instead */
#if defined(DEBUG)
#define pr_debug_once(fmt, ...)					\
	printk_once(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#else
#define pr_debug_once(fmt, ...)					\
	no_printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#endif

/*
 * ratelimited messages with local ratelimit_state,
 * no local ratelimit_state used in the !PRINTK case
 */
#ifdef CONFIG_PRINTK
#define printk_ratelimited(fmt, ...)					\
({									\
	static DEFINE_RATELIMIT_STATE(_rs,				\
				      DEFAULT_RATELIMIT_INTERVAL,	\
				      DEFAULT_RATELIMIT_BURST);		\
									\
	if (__ratelimit(&_rs))						\
		printk(fmt, ##__VA_ARGS__);				\
})
#else
#define printk_ratelimited(fmt, ...)					\
	no_printk(fmt, ##__VA_ARGS__)
#endif

#define pr_emerg_ratelimited(fmt, ...)					\
	printk_ratelimited(KERN_EMERG pr_fmt(fmt), ##__VA_ARGS__)
#define pr_alert_ratelimited(fmt, ...)					\
	printk_ratelimited(KERN_ALERT pr_fmt(fmt), ##__VA_ARGS__)
#define pr_crit_ratelimited(fmt, ...)					\
	printk_ratelimited(KERN_CRIT pr_fmt(fmt), ##__VA_ARGS__)
#define pr_err_ratelimited(fmt, ...)					\
	printk_ratelimited(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__)
#define pr_warn_ratelimited(fmt, ...)					\
	printk_ratelimited(KERN_WARNING pr_fmt(fmt), ##__VA_ARGS__)
#define pr_notice_ratelimited(fmt, ...)					\
	printk_ratelimited(KERN_NOTICE pr_fmt(fmt), ##__VA_ARGS__)
#define pr_info_ratelimited(fmt, ...)					\
	printk_ratelimited(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)
/* no pr_cont_ratelimited, don't do that... */

#if defined(DEBUG)
#define pr_devel_ratelimited(fmt, ...)					\
	printk_ratelimited(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#else
#define pr_devel_ratelimited(fmt, ...)					\
	no_printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#endif

/* If you are writing a driver, please use dev_dbg instead */
#if defined(CONFIG_DYNAMIC_DEBUG)
/* descriptor check is first to prevent flooding with "callbacks suppressed" */
#define pr_debug_ratelimited(fmt, ...)					\
do {									\
	static DEFINE_RATELIMIT_STATE(_rs,				\
				      DEFAULT_RATELIMIT_INTERVAL,	\
				      DEFAULT_RATELIMIT_BURST);		\
	DEFINE_DYNAMIC_DEBUG_METADATA(descriptor, pr_fmt(fmt));		\
	if (unlikely(descriptor.flags & _DPRINTK_FLAGS_PRINT) &&	\
	    __ratelimit(&_rs))						\
		__dynamic_pr_debug(&descriptor, pr_fmt(fmt), ##__VA_ARGS__);	\
} while (0)
#elif defined(DEBUG)
#define pr_debug_ratelimited(fmt, ...)					\
	printk_ratelimited(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#else
#define pr_debug_ratelimited(fmt, ...) \
	no_printk(KERN_DEBUG pr_fmt(fmt), ##__VA_ARGS__)
#endif

extern const struct file_operations kmsg_fops;

enum {
	DUMP_PREFIX_NONE,
	DUMP_PREFIX_ADDRESS,
	DUMP_PREFIX_OFFSET
};
extern int hex_dump_to_buffer(const void *buf, size_t len, int rowsize,
			      int groupsize, char *linebuf, size_t linebuflen,
			      bool ascii);

extern void print_hex_dump(const char *level, const char *prefix_str,
			   int prefix_type, int rowsize, int groupsize,
			   const void *buf, size_t len, bool ascii);
extern void print_hex_dump_bytes(const char *prefix_str, int prefix_type,
				 const void *buf, size_t len);


#endif
