// DGen
// Tooling to help debug.
// (C) 2012 Edd Barrett <vext01@gmail.com>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#ifdef WITH_MUSA
extern "C" {
#include "musa/m68k.h"
}
#endif

#ifdef WITH_DZ80
#include "dz80/types.h"
#include "dz80/dissz80.h"
#endif

#include "pd.h"
#include "md.h"
#include "system.h"
#include "debug.h"
#include "linenoise/linenoise.h"

static const char *debug_context_names[] = {
	"M68K", "Z80", "YM2612", "SN76489"
};

/**
 * Aliases for the various cores.
 * @{
 */
/** Aliases for SN76489 core. */
const char	*psg_aliases[] = { "sn", "sn76489", "psg", NULL };
/** Aliases for YM2612 core. */
const char	*fm_aliases[] = { "fm", "ym", "ym2612", NULL };
/** Aliases for Z80 core. */
const char	*z80_aliases[] = { "z80", "z", NULL};
/** Aliases for M68K core. */
const char	*m68k_aliases[] = { "m68k", "m", "68000", "m68000", NULL};
/** @} */

#define CURRENT_DEBUG_CONTEXT_NAME	debug_context_names[debug_context]

// ===[ C Functions ]=========================================================

/**
 * Linenoise completion callback.
 *
 * @param buf String so far.
 * @param lc List of linenoise completions.
 */
#ifndef NO_COMPLETION
void completion(const char *buf, linenoiseCompletions *lc) {

	const struct md::dgen_debugger_cmd	*cmd = md::debug_cmd_list;

	while (cmd->cmd != NULL) {
		if (strlen(cmd->cmd) != 1) {
			if (buf[0] == cmd->cmd[0]) {
				linenoiseAddCompletion(lc, cmd->cmd);
			}
		}
		cmd++;
	}
}
#endif

#ifdef WITH_MUSA
/** @{ Callbacks for Musashi. */
uint32_t m68k_read_disassembler_8(unsigned int addr)
{
	    return m68k_read_memory_8(addr);
}

uint32_t m68k_read_disassembler_16(unsigned int addr)
{
	    return m68k_read_memory_16(addr);
}

uint32_t m68k_read_disassembler_32(unsigned int addr)
{
	    return m68k_read_memory_32(addr);
}
/** @} */
#endif

/**
 * A simple wrapper around strtoul() with error check.
 *
 * @param[in] str String to convert to number.
 * @param[out] ret Number "str" represents.
 * @return -1 on error.
 */
static int debug_strtou32(const char *str, uint32_t *ret)
{
	char			*end = NULL;

	errno = 0;
	*ret = (uint32_t)strtoul(str, &end, 0);

	if (errno) {
		perror("strtoul");
		return (-1);
	}

	if (end == str)
		return (-1);

	return (0);
}

/**
 * Check if at least one M68K breakpoint is set.
 *
 * @return 1 if true, 0 otherwise.
 */
bool md::debug_is_m68k_bp_set()
{
	if (debug_bp_m68k[0].flags & BP_FLAG_USED)
		return (1);

	return (0);
}

/**
 * Check if at least one Z80 breakpoint is set.
 *
 * @return 1 if true, 0 otherwise.
 */
bool md::debug_is_z80_bp_set()
{
	if (debug_bp_z80[0].flags & BP_FLAG_USED)
		return 1;
	return 0;
}

/**
 * Check if at least one M68K watchpoint is set.
 *
 * @return 1 if true, 0 otherwise.
 */
bool md::debug_is_m68k_wp_set()
{
	if (debug_wp_m68k[0].flags & BP_FLAG_USED)
		return (1);

	return (0);
}

/**
 * Check if at least one Z80 watchpoint is set.
 *
 * @return 1 if true, 0 otherwise.
 */
bool md::debug_is_z80_wp_set()
{
	if (debug_wp_z80[0].flags & BP_FLAG_USED)
		return 1;
	return 0;
}

/**
 * Get the ID of the next free M68K watchpoint.
 *
 * @return ID or -1 if none free.
 */
int md::debug_next_free_wp_m68k()
{
	int			i;

	for (i = 0; i < MAX_WATCHPOINTS; i++) {
		if (!(debug_wp_m68k[i].flags & WP_FLAG_USED))
			return (i);
	}

	return (-1); // no free slots
}

/**
 * Get the ID of the next free Z80 watchpoint.
 *
 * @return ID or -1 if none free.
 */
int md::debug_next_free_wp_z80()
{
	unsigned int i;

	for (i = 0; (i < MAX_WATCHPOINTS); ++i)
		if (!(debug_wp_z80[i].flags & WP_FLAG_USED))
			return i;
	return -1;
}

/**
 * Get the ID of the next free M68K breakpoint.
 *
 * @return ID or -1 if none free.
 */
int md::debug_next_free_bp_m68k()
{
	int			i;

	for (i = 0; i < MAX_BREAKPOINTS; i++) {
		if (!(debug_bp_m68k[i].flags & BP_FLAG_USED))
			return (i);
	}

	return (-1); // no free slots
}

#ifdef WITH_DZ80

static BYTE disz80_read(void *ctx, WORD addr)
{
	class md *md = (class md *)ctx;

	return md->z80_read(addr);
}

#endif

/**
 * Get the ID of the next free Z80 breakpoint.
 *
 * @return ID or -1 if none free.
 */
int md::debug_next_free_bp_z80()
{
	unsigned int i;

	for (i = 0; (i < MAX_BREAKPOINTS); ++i)
		if (!(debug_bp_z80[i].flags & BP_FLAG_USED))
			return i;
	return -1;
}

/**
 * Initialise the debugger.
 *
 * All breakpoints are disabled by default.
 */
void md::debug_init()
{
	// start with all breakpoints and watchpoints disabled
	memset(debug_bp_m68k, 0, sizeof(debug_bp_m68k));
	memset(debug_wp_m68k, 0, sizeof(debug_wp_m68k));
	memset(debug_bp_z80, 0, sizeof(debug_bp_z80));
	memset(debug_wp_z80, 0, sizeof(debug_wp_z80));

#ifndef NO_COMPLETION
	linenoiseSetCompletionCallback(completion);
#endif

	debug_step_m68k = 0;
	debug_trace_m68k = 0;
	debug_step_z80 = 0;
	debug_trace_z80 = 0;
	debug_context = DBG_CONTEXT_M68K;
	debug_trap = false;
	debug_m68k_instr_count = 0;
	debug_z80_instr_count = 0;
	debug_instr_count_enabled = false;

#ifdef WITH_DZ80
	memset(&disz80, 0, sizeof(disz80));
	dZ80_SetDefaultOptions(&disz80);
	disz80.cpuType = DCPU_Z80;
	disz80.flags = (DISFLAG_SINGLE | DISFLAG_CALLBACK);
	disz80.mem0Start = (BYTE *)this;
	disz80.memCB = disz80_read;
#endif
}

/**
 * Find the index of a M68K breakpoint.
 *
 * @param addr Address to look for.
 * @return -1 if no breakpoint is found, otherwise its index in
 * debug_bp_m68k[].
 */
int md::debug_find_bp_m68k(uint32_t addr)
{
	int			i;

	for (i = 0; i < MAX_BREAKPOINTS; i++) {

		if (!(debug_bp_m68k[i].flags & BP_FLAG_USED))
			break;

		if (debug_bp_m68k[i].addr == addr)
			return (i);
	}

	return (-1); // not found
}

/**
 * Find the index of a Z80 breakpoint.
 *
 * @param addr Address to look for.
 * @return -1 if no breakpoint is found, otherwise its index in
 * debug_bp_z80[].
 */
int md::debug_find_bp_z80(uint16_t addr)
{
	unsigned int i;

	for (i = 0; (i < MAX_BREAKPOINTS); ++i) {
		if (!(debug_bp_z80[i].flags & BP_FLAG_USED))
			break;
		if (debug_bp_z80[i].addr == addr)
			return i;
	}
	return -1;
}

/**
 * Find the index of a M68K watchpoint by its start address.
 *
 * @param addr Address to look for.
 * @return -1 if no watchpoint at the given address, otherwise its index in
 * debug_wp_m68k[].
 */
int md::debug_find_wp_m68k(uint32_t addr)
{
	int			i;

	for (i = 0; i < MAX_WATCHPOINTS; i++) {

		if (!(debug_wp_m68k[i].flags & WP_FLAG_USED))
			break;

		if (debug_wp_m68k[i].start_addr == addr)
			return (i);
	}

	return (-1); // not found
}

/**
 * Find the index of a Z80 watchpoint by its start address.
 *
 * @param addr Address to look for.
 * @return -1 if no watchpoint at the given address, otherwise its index in
 * debug_wp_z80[].
 */
int md::debug_find_wp_z80(uint16_t addr)
{
	unsigned int i;

	for (i = 0; (i < MAX_WATCHPOINTS); ++i) {
		if (!(debug_wp_z80[i].flags & WP_FLAG_USED))
			break;
		if (debug_wp_z80[i].start_addr == addr)
			return i;
	}
	return -1;
}

/**
 * Pretty prints hex dump.
 *
 * @param[in] buf Buffer to pretty print.
 * @param len Number of bytes to print.
 * @param addr_label_start The address of the first byte.
 */
static void debug_print_hex_buf(
    unsigned char *buf, size_t len, size_t addr_label_start)
{
	uint32_t		i, done = 0;
	unsigned char		byte;
	char			ascii[17], *ap, *hp;
	char			hdr[60] = "            ";
	char			hex[] = "0123456789abcdef";

	// header
	for (i = 0; i < 16; i++) {
		hp = strchr(hdr, '\0');
		hp[0] = hex[(addr_label_start + i) % 16];
		hp[1] = ' ';
		hp[2] = ' ';
		hp[3] = '\0';
	}
	printf("%s\n", hdr);

	// process lines of 16 bytes
	ap = ascii;
	for (i = 0; i < len; i++, done++, ap++) {

		if (i % 16 == 0) {
			ascii[16] = '\0';

			if (i > 0)
				printf(" |%s|\n", ascii);

			printf("0x%08x: ", (uint32_t) addr_label_start + i);
			ap = ascii;
		}

		byte = buf[i];
		// 0x20 to 0x7e is printable ascii
		if ((byte >= 0x20) && (byte <= 0x7e))
			*ap = byte;
		else
			*ap = '.';

		printf("%02x ",  byte);
	}

	// make it all line up
	if (i % 16) {
		ascii[(i % 16)] = '\0';
		i = i % 16;
		while (i <= 15) {
			printf("   ");
			i++;
		}
	}

	// print rest of ascii
	printf(" |%s|\n", ascii);
	fflush(stdout);
}

/**
 * Print a M68K watchpoint in a human-readable form.
 *
 * @param idx Index of watchpoint to print.
 */
void md::debug_print_m68k_wp(int idx)
{
	struct dgen_wp		*w = &(debug_wp_m68k[idx]);

	printf("#%0d:\t0x%08x-%08x (%u bytes)\n", idx,
	    w->start_addr, w->end_addr, w->end_addr - w->start_addr + 1);

	debug_print_hex_buf(w->bytes, w->end_addr - w->start_addr + 1, w->start_addr);
	fflush(stdout);
}

/**
 * Print a Z80 watchpoint in a human-readable form.
 *
 * @param idx Index of watchpoint to print.
 */
void md::debug_print_z80_wp(int idx)
{
	struct dgen_wp *w = &debug_wp_z80[idx];

	printf("#%0d:\t0x%04x-%04x (%u bytes)\n", idx, w->start_addr,
	       w->end_addr, (w->end_addr - w->start_addr + 1));
	debug_print_hex_buf(w->bytes, (w->end_addr - w->start_addr + 1),
			    w->start_addr);
	fflush(stdout);
}

/**
 * Check the given M68K watchpoint against cached memory to see if it should
 * fire.
 *
 * @param[in] w Watch point to check.
 * @return 1 if true, else 0.
 */
int md::debug_should_m68k_wp_fire(struct dgen_wp *w)
{
	unsigned int		i;
	unsigned char		*p;

	for (i = w->start_addr, p = w->bytes; i <= w->end_addr; i++, p++) {
		if (misc_readbyte(i) != *p)
			return (1); // hit
	}

	return (0);
}

/**
 * Check the given Z80 watchpoint against cached memory to see if it should
 * fire.
 *
 * @param[in] w Watch point to check.
 * @return 1 if true, else 0.
 */
int md::debug_should_z80_wp_fire(struct dgen_wp *w)
{
	unsigned int i = w->start_addr;
	unsigned char *p = w->bytes;

	while (i <= w->end_addr)
		if (z80_read(i++) != *p++)
			return 1;
	return 0;
}

/**
 * Get M68K PC.
 *
 * @return Current PC.
 */
uint32_t md::m68k_get_pc()
{
	m68k_state_dump();
	return le2h32(m68k_state.pc);
}

/**
 * Get Z80 PC.
 *
 * @return Current PC.
 */
uint16_t md::z80_get_pc()
{
	z80_state_dump();
	return le2h16(z80_state.pc);
}

/**
 * Breakpoint handler fired before every M68K instruction.
 */
bool md::debug_m68k_check_bps()
{
	uint32_t pc = m68k_get_pc();
	unsigned int i;
	bool bp = false;

	if (debug_step_m68k) {
		if ((--debug_step_m68k) == 0) {
			debug_enter();
			bp = true;
		}
		goto trace;
	}
	for (i = 0; (i < MAX_BREAKPOINTS); i++) {
		if (!(debug_bp_m68k[i].flags & BP_FLAG_USED))
			break; // no bps after first disabled one
		if (pc == debug_bp_m68k[i].addr) {
			if (debug_bp_m68k[i].flags & BP_FLAG_FIRED) {
				debug_bp_m68k[i].flags &= ~BP_FLAG_FIRED;
				continue;
			}
			debug_bp_m68k[i].flags |= BP_FLAG_FIRED;
			printf("m68k breakpoint hit @ 0x%08x\n", pc);
			debug_enter();
			bp = true;
			break;
		}
	}
trace:
	if (debug_trace_m68k) {
		if (!bp)
			debug_print_m68k_disassemble(pc, 1);
		--debug_trace_m68k;
	}
	fflush(stdout);
	return bp;
}

/**
 * Watchpoint handler fired after every M68K instruction.
 */
bool md::debug_m68k_check_wps()
{
	unsigned int i;
	bool wp = false;

	for (i = 0; (i < MAX_WATCHPOINTS); i++) {
		if (!(debug_wp_m68k[i].flags & BP_FLAG_USED))
			break; // no wps after first disabled one
		if (debug_should_m68k_wp_fire(&(debug_wp_m68k[i]))) {
			printf("m68k watchpoint #%d fired\n", i);
			debug_wp_m68k[i].flags |= WP_FLAG_FIRED;
			debug_print_m68k_wp(i);
			debug_enter();
			debug_update_fired_m68k_wps();
			wp = true;
			break;
		}
	}
	fflush(stdout);
	return wp;
}

/**
 * Breakpoint handler fired before every Z80 instruction.
 */
bool md::debug_z80_check_bps()
{
	uint16_t pc = z80_get_pc();
	unsigned int i;
	bool bp = false;

	if (debug_step_z80) {
		if ((--debug_step_z80) == 0) {
			debug_enter();
			bp = true;
		}
		goto trace;
	}
	for (i = 0; (i < MAX_BREAKPOINTS); i++) {
		if (!(debug_bp_z80[i].flags & BP_FLAG_USED))
			break; // no bps after first disabled one
		if (pc == debug_bp_z80[i].addr) {
			if (debug_bp_z80[i].flags & BP_FLAG_FIRED) {
				debug_bp_z80[i].flags &= ~BP_FLAG_FIRED;
				continue;
			}
			debug_bp_z80[i].flags |= BP_FLAG_FIRED;
			printf("z80 breakpoint hit @ 0x%04x\n", pc);
			debug_enter();
			bp = true;
			break;
		}
	}
trace:
	if (debug_trace_z80) {
		if (!bp)
			debug_print_z80_disassemble(pc, 1);
		--debug_trace_z80;
	}
	fflush(stdout);
	return bp;
}

/**
 * Watchpoint handler fired after every Z80 instruction.
 */
bool md::debug_z80_check_wps()
{
	unsigned int i;
	bool wp = false;

	for (i = 0; (i < MAX_WATCHPOINTS); i++) {
		if (!(debug_wp_z80[i].flags & BP_FLAG_USED))
			break;
		if (debug_should_z80_wp_fire(&(debug_wp_z80[i]))) {
			printf("z80 watchpoint #%d fired\n", i);
			debug_wp_z80[i].flags |= WP_FLAG_FIRED;
			debug_print_z80_wp(i);
			debug_enter();
			debug_update_fired_z80_wps();
			wp = true;
			break;
		}
	}
	fflush(stdout);
	return wp;
}

/**
 * Remove a M68K breakpoint.
 *
 * @param index Index of breakpoint to remove.
 */
void md::debug_rm_bp_m68k(int index)
{
	if (!(debug_bp_m68k[index].flags & BP_FLAG_USED)) {
		printf("breakpoint not set\n");
		fflush(stdout);
		return;
	}

	// shift everything down one
	if (index == MAX_BREAKPOINTS - 1) {
		debug_bp_m68k[index].addr = 0;
		debug_bp_m68k[index].flags = 0;
	} else {
		memmove(&(debug_bp_m68k[index]),
		    &(debug_bp_m68k[index+1]),
		    sizeof(struct dgen_bp) * (MAX_BREAKPOINTS - index - 1));
		// disable last slot
		debug_bp_m68k[MAX_BREAKPOINTS - 1].addr = 0;
		debug_bp_m68k[MAX_BREAKPOINTS - 1].flags = 0;
	}
}

/**
 * Remove a Z80 breakpoint.
 *
 * @param index Index of breakpoint to remove.
 */
void md::debug_rm_bp_z80(int index)
{
	if (!(debug_bp_z80[index].flags & BP_FLAG_USED)) {
		printf("breakpoint not set\n");
		fflush(stdout);
		return;
	}
	if (index == (MAX_BREAKPOINTS - 1)) {
		debug_bp_z80[index].addr = 0;
		debug_bp_z80[index].flags = 0;
	}
	else {
		memmove(&debug_bp_z80[index],
			&debug_bp_z80[index + 1],
			(sizeof(struct dgen_bp) *
			 (MAX_BREAKPOINTS - index - 1)));
		debug_bp_z80[MAX_BREAKPOINTS - 1].addr = 0;
		debug_bp_z80[MAX_BREAKPOINTS - 1].flags = 0;
	}
}

/**
 * Remove a M68K watchpoint.
 *
 * @param index Index of watchpoint to remove.
 */
void md::debug_rm_wp_m68k(int index)
{
	if (!(debug_wp_m68k[index].flags & WP_FLAG_USED)) {
		printf("watchpoint not set\n");
		fflush(stdout);
		return;
	}

	free(debug_wp_m68k[index].bytes);

	// shift everything down one
	if (index == MAX_WATCHPOINTS - 1) {
		debug_wp_m68k[index].start_addr = 0;
		debug_wp_m68k[index].flags = 0;
	} else {
		memmove(&(debug_wp_m68k[index]),
		    &(debug_wp_m68k[index+1]),
		    sizeof(struct dgen_bp) * (MAX_WATCHPOINTS - index - 1));
		// disable last slot
		debug_wp_m68k[MAX_WATCHPOINTS - 1].start_addr = 0;
		debug_wp_m68k[MAX_WATCHPOINTS - 1].flags = 0;
	}
}

/**
 * Remove a Z80 watchpoint.
 *
 * @param index Index of watchpoint to remove.
 */
void md::debug_rm_wp_z80(int index)
{
	if (!(debug_wp_z80[index].flags & WP_FLAG_USED)) {
		printf("watchpoint not set\n");
		fflush(stdout);
		return;
	}
	free(debug_wp_z80[index].bytes);
	if (index == (MAX_WATCHPOINTS - 1)) {
		debug_wp_z80[index].start_addr = 0;
		debug_wp_z80[index].flags = 0;
	}
	else {
		memmove(&debug_wp_z80[index],
			&debug_wp_z80[index + 1],
			(sizeof(struct dgen_bp) *
			 (MAX_WATCHPOINTS - index - 1)));
		debug_wp_z80[MAX_WATCHPOINTS - 1].start_addr = 0;
		debug_wp_z80[MAX_WATCHPOINTS - 1].flags = 0;
	}
}

/**
 * Pretty print M68K breakpoints.
 */
void md::debug_list_bps_m68k()
{
	int			i;

	printf("m68k breakpoints:\n");
	for (i = 0; i < MAX_BREAKPOINTS; i++) {
		if (!(debug_bp_m68k[i].flags & BP_FLAG_USED))
			break; // can be no more after first disabled bp

		printf("#%0d:\t0x%08x\n", i, debug_bp_m68k[i].addr);
	}

	if (i == 0)
		printf("\tno m68k breakpoints set\n");
	fflush(stdout);
}

/**
 * Pretty print Z80 breakpoints.
 */
void md::debug_list_bps_z80()
{
	unsigned int i;

	printf("z80 breakpoints:\n");
	for (i = 0; (i < MAX_BREAKPOINTS); i++) {
		if (!(debug_bp_z80[i].flags & BP_FLAG_USED))
			break;
		printf("#%0d:\t0x%04x\n", i, debug_bp_z80[i].addr);
	}
	if (i == 0)
		printf("\tno z80 breakpoints set\n");
	fflush(stdout);
}

/**
 * Pretty print M68K watchpoints.
 */
void md::debug_list_wps_m68k()
{
	int			i;

	printf("m68k watchpoints:\n");
	for (i = 0; i < MAX_WATCHPOINTS; i++) {
		if (!(debug_wp_m68k[i].flags & WP_FLAG_USED))
			break; // can be no more after first disabled
		debug_print_m68k_wp(i);
	}

	if (i == 0)
		printf("\tno m68k watchpoints set\n");
	fflush(stdout);
}

/**
 * Pretty print Z80 watchpoints.
 */
void md::debug_list_wps_z80()
{
	unsigned int i;

	printf("z80 watchpoints:\n");
	for (i = 0; (i < MAX_WATCHPOINTS); i++) {
		if (!(debug_wp_z80[i].flags & WP_FLAG_USED))
			break;
		debug_print_z80_wp(i);
	}
	if (i == 0)
		printf("\tno z80 watchpoints set\n");
	fflush(stdout);
}

/**
 * Add a M68K breakpoint.
 *
 * @param addr Address to break on.
 * @return Always 1.
 */
int md::debug_set_bp_m68k(uint32_t addr)
{
	int		slot;

	if ((debug_find_bp_m68k(addr)) != -1) {
		printf("breakpoint already set at this address\n");
		goto out;
	}

	slot = debug_next_free_bp_m68k();
	if (slot == -1) {
		printf("No space for another break point\n");
		goto out;
	}

	debug_bp_m68k[slot].addr = addr;
	debug_bp_m68k[slot].flags = BP_FLAG_USED;
	printf("m68k breakpoint #%d set @ 0x%08x\n", slot, addr);
out:
	fflush(stdout);
	return (1);
}

/**
 * Add a Z80 breakpoint.
 *
 * @param addr Address to break on.
 * @return Always 1.
 */
int md::debug_set_bp_z80(uint16_t addr)
{
	int slot;

	if ((debug_find_bp_z80(addr)) != -1) {
		printf("breakpoint already set at this address\n");
		goto out;
	}
	slot = debug_next_free_bp_z80();
	if (slot == -1) {
		printf("No space for another break point\n");
		goto out;
	}
	debug_bp_z80[slot].addr = addr;
	debug_bp_z80[slot].flags = BP_FLAG_USED;
	printf("z80 breakpoint #%d set @ 0x%04x\n", slot, addr);
out:
	fflush(stdout);
	return 1;
}

/**
 * Convert a core name to a context ID.
 *
 * @param[in] arg NUL-terminated core name.
 * @return Core context ID or -1 on error.
 */
static int debug_parse_cpu(char *arg)
{
	uint32_t	  num;
	const char	**p;

	/* by name */
	for (p = z80_aliases; *p != NULL; p++) {
		if (strcmp(arg, *p) == 0)
			return (DBG_CONTEXT_Z80);
	}

	for (p = m68k_aliases; *p != NULL; p++) {
		if (strcmp(arg, *p) == 0)
			return (DBG_CONTEXT_M68K);
	}

	for (p = fm_aliases; *p != NULL; p++) {
		if (strcmp(arg, *p) == 0)
			return (DBG_CONTEXT_YM2612);
	}

	for (p = psg_aliases; *p != NULL; p++) {
		if (strcmp(arg, *p) == 0)
			return (DBG_CONTEXT_SN76489);
	}

	/* by index */
	if ((debug_strtou32(arg, &num)) < 0)
		return (-1);

	if (num > DBG_CONTEXT_SN76489)
		return (-1);

	return ((int) num);
}

// ===[ C++ Methods ]=========================================================

/**
 * Add a M68K watchpoint to a range of addresses.
 *
 * @param start_addr Start address of watchpoint range.
 * @param end_addr End address of watchpoint range.
 */
void md::debug_set_wp_m68k(uint32_t start_addr, uint32_t end_addr)
{
	int		slot;

	slot = debug_next_free_wp_m68k();
	if (slot == -1) {
		printf("No space for another watch point\n");
		goto out;
	}

	debug_wp_m68k[slot].start_addr = start_addr;
	debug_wp_m68k[slot].end_addr = end_addr;
	debug_wp_m68k[slot].flags = WP_FLAG_USED;
	debug_wp_m68k[slot].bytes = (unsigned char *) malloc(end_addr - start_addr + 1);
	if (debug_wp_m68k[slot].bytes == NULL) {
		perror("malloc");
		goto out;
	}

	debug_update_m68k_wp_cache(&(debug_wp_m68k[slot]));

	printf("m68k watchpoint #%d set @ 0x%08x-0x%08x (%u bytes)\n",
	    slot, start_addr, end_addr, end_addr - start_addr + 1);
out:
	fflush(stdout);
}

/**
 * Update the data pointer of a single M68K watchpoint.
 *
 * @param w Watchpoint to update.
 */
void md::debug_update_m68k_wp_cache(struct dgen_wp *w)
{
	unsigned int		 addr;
	unsigned char		*p;

	p = w->bytes;
	for (addr = w->start_addr; addr <= w->end_addr; addr++) {
		*(p++) = misc_readbyte(addr);
	}
}

/**
 * Resynchronise all M68K watchpoints based on actual data.
 */
void md::debug_update_fired_m68k_wps()
{
	int			i;

	for (i = 0; i < MAX_WATCHPOINTS; i++) {

		if (!(debug_wp_m68k[i].flags & WP_FLAG_USED))
			return;

		if (!(debug_wp_m68k[i].flags & WP_FLAG_FIRED))
			continue;

		debug_update_m68k_wp_cache(&(debug_wp_m68k[i]));
		debug_wp_m68k[i].flags &= ~WP_FLAG_FIRED;
	}
}

/**
 * Add a Z80 watchpoint to a range of addresses.
 *
 * @param start_addr Start address of watchpoint range.
 * @param end_addr End address of watchpoint range.
 */
void md::debug_set_wp_z80(uint16_t start_addr, uint16_t end_addr)
{
	int slot;

	slot = debug_next_free_wp_z80();
	if (slot == -1) {
		printf("No space for another watch point\n");
		goto out;
	}
	debug_wp_z80[slot].start_addr = start_addr;
	debug_wp_z80[slot].end_addr = end_addr;
	debug_wp_z80[slot].flags = WP_FLAG_USED;
	debug_wp_z80[slot].bytes =
		(unsigned char *)malloc(end_addr - start_addr + 1);
	if (debug_wp_z80[slot].bytes == NULL) {
		perror("malloc");
		goto out;
	}
	debug_update_z80_wp_cache(&(debug_wp_z80[slot]));
	printf("z80 watchpoint #%d set @ 0x%04x-0x%04x (%u bytes)\n",
	       slot, start_addr, end_addr, (end_addr - start_addr + 1));
out:
	fflush(stdout);
}

/**
 * Update the data pointer of a single Z80 watchpoint.
 *
 * @param w Watchpoint to update.
 */
void md::debug_update_z80_wp_cache(struct dgen_wp *w)
{
	unsigned int addr;
	unsigned char *p = w->bytes;

	for (addr = w->start_addr; (addr <= w->end_addr); addr++)
		*(p++) = z80_read(addr);
}

/**
 * Resynchronise all Z80 watchpoints based on actual data.
 */
void md::debug_update_fired_z80_wps()
{
	int i;

	for (i = 0; (i < MAX_WATCHPOINTS); i++) {
		if (!(debug_wp_z80[i].flags & WP_FLAG_USED))
			return;
		if (!(debug_wp_z80[i].flags & WP_FLAG_FIRED))
			continue;
		debug_update_z80_wp_cache(&(debug_wp_z80[i]));
		debug_wp_z80[i].flags &= ~WP_FLAG_FIRED;
	}
}

/**
 * Watchpoints (watch) command handler.
 *
 * - If n_args == 0 then list watchpoints.
 * - If n_args == 1 then add a watchpoint with length 1 with start address
 *   defined in args[0].
 * - If n_args == 2 then add a watch point with start address args[0] and
 *   length args[1].
 *
 * @param n_args Number of arguments.
 * @param args Arguments, see above.
 * @return Always 1.
 */
int md::debug_cmd_watch(int n_args, char **args)
{
	uint32_t		start, len = 1;

	if (debug_context != DBG_CONTEXT_M68K){
		printf("watchpoints not supported on %s core\n",
		    CURRENT_DEBUG_CONTEXT_NAME);
		goto out;
	}

	switch (n_args) {
	case 2:
		if ((debug_strtou32(args[1], &len)) < 0) {
			printf("length malformed: %s\n", args[1]);
			goto out;
		}
		// fallthru
	case 1:
		if ((debug_strtou32(args[0], &start)) < 0) {
			printf("address malformed: %s\n", args[0]);
			goto out;
		}
		debug_set_wp_m68k(start, start+len-1); // one byte
		break;
	case 0:
		// listing wps
		debug_list_wps_m68k();
		break;
	};
out:
	fflush(stdout);
	return (1);
}

/**
 * Pretty print a block of memory as a hex dump.
 *
 * @param addr Start address.
 * @param len Length (in bytes) to dump.
 */
void md::debug_dump_mem(uint32_t addr, uint32_t len)
{
	uint32_t		 i;
	unsigned char		*buf;

	// we have to make a buffer to pass down
	buf = (unsigned char *) malloc(len);
	if (buf == NULL) {
		perror("malloc");
		return;
	}

	for (i = 0; i < len; i++) {
	    buf[i] = misc_readbyte(addr + i);
	}

	debug_print_hex_buf(buf, len, addr);
	free(buf);
}

/**
 * Memory dump (mem) command handler.
 *
 * - If n_args == 1 then args[0] is start address to dump from for
 *   DEBUG_DFLT_MEMDUMP_LEN bytes.
 * - If n_args == 2 then args[0] is start address and args[1] is number of
 *   bytes to dump.
 *
 * @param n_args Number of arguments.
 * @param args Arguments, see above.
 * @return Always 1.
 */
int md::debug_cmd_mem(int n_args, char **args)
{
	uint32_t		addr, len = DEBUG_DFLT_MEMDUMP_LEN;

	if (debug_context != DBG_CONTEXT_M68K) {
		printf("memory dumping not implemented on %s core\n",
		    CURRENT_DEBUG_CONTEXT_NAME);
		goto out;
	}

	switch (n_args) {
	case 2: /* specified length */
		if ((debug_strtou32(args[1], &len)) < 0) {
			printf("length malformed: %s\n", args[1]);
			goto out;
		}
		/* FALLTHRU */
	case 1: /* default length */
		if ((debug_strtou32(args[0], &addr)) < 0) {
			printf("addr malformed: %s\n", args[0]);
			goto out;
		}

		debug_dump_mem(addr, len);
		break;
	};
out:
	fflush(stdout);
	return (1);
}

/**
 * Memory/registers write (setb/setw/setl/setr) commands handler.
 *
 * - args[1] is the numerical value to write to registers/memory.
 * - If type == ~0u, args[0] is a register name.
 * - If type == 1, args[0] is the address of a byte.
 * - If type == 2, args[0] is the address of a word.
 * - If type == 4, args[0] is the address of a long word.
 *
 * @param n_args Number of arguments (ignored, always 2).
 * @param args Arguments, see above.
 * @param type Data type to write, see above.
 * @return Always 1.
 */
int md::debug_cmd_setbwlr(int n_args, char **args, unsigned int type)
{
	uint32_t		addr = 0;
	union {
		uint32_t *r32;
		uint16_t *r16;
		uint8_t *r8;
		void *ptr;
	} reg_ptr = { NULL };
	unsigned int		reg = -1;
	uint32_t		val;

	(void)n_args;
	assert(n_args == 2);
	if ((debug_context != DBG_CONTEXT_M68K) &&
	    (debug_context != DBG_CONTEXT_Z80)) {
		printf("memory setting not implemented on %s core\n",
		       CURRENT_DEBUG_CONTEXT_NAME);
		goto out;
	}
	if (type == ~0u) {
		static const struct {

#define REG0(id, idx) { \
	# id # idx, \
	((uintptr_t)&m68k_state.id[idx] - (uintptr_t)&m68k_state), \
	sizeof(m68k_state.id[idx]) \
}

#define REG1(id) { # id, offsetof(m68k_state_t, id), sizeof(m68k_state.id) }

#define REG2(id, idx, name) { \
	name, \
	((uintptr_t)&z80_state.alt[idx].id - (uintptr_t)&z80_state), \
	sizeof(z80_state.alt[idx].id) \
}

#define REG3(id) { # id, offsetof(z80_state_t, id), sizeof(z80_state.id) }

			const char *name;
			size_t offset;
			size_t size;
		} regid[2][32] = {
			{
				// M68K
				REG0(a, 0), REG0(a, 1), REG0(a, 2), REG0(a, 3),
				REG0(a, 4), REG0(a, 5), REG0(a, 6), REG0(a, 7),
				REG0(d, 0), REG0(d, 1), REG0(d, 2), REG0(d, 3),
				REG0(d, 4), REG0(d, 5), REG0(d, 6), REG0(d, 7),
				REG1(pc), REG1(sr)
			},
			{
				// Z80
				REG2(fa, 0, "af"), REG2(fa, 1, "af'"),
				REG2(cb, 0, "bc"), REG2(cb, 1, "bc'"),
				REG2(ed, 0, "de"), REG2(ed, 1, "de'"),
				REG2(lh, 0, "hl"), REG2(lh, 1, "hl'"),
				REG3(ix), REG3(iy), REG3(sp), REG3(pc),
				REG3(r), REG3(i), REG3(iff), REG3(im)
			}
		};
		unsigned int idx;
		uint8_t *state;

		if (debug_context == DBG_CONTEXT_M68K) {
			idx = 0;
			state = (uint8_t *)&m68k_state;
		}
		else if (debug_context == DBG_CONTEXT_Z80) {
			idx = 1;
			state = (uint8_t *)&z80_state;
		}
		else
			goto out;
		for (addr = 0;
		     (addr != (sizeof(regid[idx]) / sizeof(regid[idx][0])));
		     ++addr) {
			if (regid[idx][addr].name == NULL)
				break;
			if (strcasecmp(regid[idx][addr].name, args[0]))
				continue;
			reg_ptr.ptr =
				(void *)(state + regid[idx][addr].offset);
			reg = regid[idx][addr].size;
			break;
		}
		if (reg == -1u) {
			printf("unknown register %s\n", args[0]);
			goto out;
		}
	}
	else if (debug_strtou32(args[0], &addr) < 0) {
		printf("addr malformed: %s\n", args[0]);
		goto out;
	}
	if (debug_strtou32(args[1], &val) < 0) {
		printf("value malformed: %s\n", args[1]);
		goto out;
	}
	switch (type) {
	case 1:
		/* byte */
		misc_writebyte(addr, val);
		break;
	case 2:
		/* word */
		misc_writeword(addr, val);
		break;
	case 4:
		/* long */
		misc_writeword(addr, ((val >> 16) & 0xffff));
		misc_writeword((addr + 2), (val & 0xffff));
		break;
	case ~0u:
		/* register */
		m68k_state_dump();
		z80_state_dump();
		if (reg == 2)
			*reg_ptr.r16 = le2h16(val);
		else if (reg == 4)
			*reg_ptr.r32 = le2h32(val);
		else if (reg == 1)
			*reg_ptr.r8 = val;
		m68k_state_restore();
		z80_state_restore();
		break;
	default:
		printf("unknown type size %u\n", type);
	}
out:
	fflush(stdout);
	return 1;
}

/**
 * Set byte (setb) command handler, see debug_cmd_setbwlr().
 *
 * @return Always 1.
 */
int md::debug_cmd_setb(int n_args, char **args)
{
	return debug_cmd_setbwlr(n_args, args, 1);
}

/**
 * Set word (setw) command handler, see debug_cmd_setbwlr().
 *
 * @return Always 1.
 */
int md::debug_cmd_setw(int n_args, char **args)
{
	return debug_cmd_setbwlr(n_args, args, 2);
}

/**
 * Set long word (setl) command handler, see debug_cmd_setbwlr().
 *
 * @return Always 1.
 */
int md::debug_cmd_setl(int n_args, char **args)
{
	return debug_cmd_setbwlr(n_args, args, 4);
}

/**
 * Set register (setr) command handler, see debug_cmd_setbwlr().
 *
 * @return Always 1.
 */
int md::debug_cmd_setr(int n_args, char **args)
{
	return debug_cmd_setbwlr(n_args, args, ~0u);
}

/**
 * Disassemble (dis) command handler.
 *
 * - If n_args == 0, do nothing.
 * - If n_args == 1 start disassembling from address args[0] for
 *   DEBUG_DFLT_DASM_LEN instructions.
 * - If n_args == 2 start disassembling from address args[0] for args[1]
 *   instructions.
 *
 * @param n_args Number of arguments.
 * @param[in] args Arguments list.
 * @return Always 1.
 */
int md::debug_cmd_dis(int n_args, char **args)
{
	uint32_t		addr;
	uint32_t		length = DEBUG_DFLT_DASM_LEN;

	if (debug_context == DBG_CONTEXT_M68K)
		addr = m68k_get_pc();
	else if (debug_context == DBG_CONTEXT_Z80)
		addr = z80_get_pc();
	else {
		printf("disassembly is not implemented on %s core\n",
		    CURRENT_DEBUG_CONTEXT_NAME);
		goto out;
	}

	switch (n_args) {
		case 0:
			// nothing :)
			break;
		case 2:
			if ((debug_strtou32(args[1], &length)) < 0) {
				printf("length malformed: %s\n", args[1]);
				goto out;
			}
			// fallthru
		case 1:
			if ((debug_strtou32(args[0], &addr)) < 0) {
				printf("address malformed: %s\n", args[0]);
				goto out;
			}
			break;
	};

	if (debug_context == DBG_CONTEXT_M68K)
		debug_print_m68k_disassemble(addr, length);
	else if (debug_context == DBG_CONTEXT_Z80)
		debug_print_z80_disassemble(addr, length);
out:
	fflush(stdout);
	return (1);
}

/**
 * Continue (cont) command handler. This command drops out of debug mode.
 *
 * @param n_args Number of arguments (ignored).
 * @param args Arguments (ignored).
 * @return Always 0 (debugger should exit).
 */
int md::debug_cmd_cont(int n_args, char **args)
{
	(void) n_args;
	(void) args;

	debug_trap = false;
	return (0); // causes debugger to exit
}

/**
 * Pretty print the M68K registers using m68k_state from class md.
 */
void md::debug_show_m68k_regs()
{
	int			i;
	uint16_t		sr;

	m68k_state_dump();
	sr = le2h16(m68k_state.sr);

	if (debug_instr_count_enabled)
		printf("m68k (%lu instructions):\n", debug_m68k_instr_count);
	else
		printf("m68k:\n");

	printf("\tpc:\t0x%08x\n\tsr:\t0x%08x\n", le2h32(m68k_state.pc), sr);

	// print SR register - user byte
	printf("\t  user sr  : <X=%u, N=%u, Z=%u, V=%u, C=%u>\n",
	       (sr & M68K_SR_EXTEND) ? 1 : 0,
	       (sr & M68K_SR_NEGATIVE) ? 1 : 0,
	       (sr & M68K_SR_ZERO) ? 1 : 0,
	       (sr & M68K_SR_OVERFLOW) ? 1 : 0,
	       (sr & M68K_SR_CARRY) ? 1 : 0);

	// print SR register - system byte
	printf("\t  sys sr   : <TE=%u%u, SUS=%u, MIS=%u, IPM=%u%u%u>\n",
	       (sr & M68K_SR_TRACE_EN1) ? 1 : 0,
	       (sr & M68K_SR_TRACE_EN2) ? 1 : 0,
	       (sr & M68K_SR_SUP_STATE) ? 1 : 0,
	       (sr & M68K_SR_MI_STATE) ? 1 : 0,
	       (sr & M68K_SR_IP_MASK1) ? 1 : 0,
	       (sr & M68K_SR_IP_MASK2) ? 1 : 0,
	       (sr & M68K_SR_IP_MASK3) ? 1 : 0);

	// d*
	for (i =  0; i < 8; i++)
		printf("\td%d:\t0x%08x\n", i, le2h32(m68k_state.d[i]));
	// a*
	for (i =  0; i < 8; i++)
		printf("\ta%d:\t0x%08x\n", i, le2h32(m68k_state.a[i]));
	fflush(stdout);
}

#define PRINT_Z80_FLAGS(x)						\
	printf("\t   <S=%u, Z= %u, H=%u, P/V=%u, N=%u, C=%u>\n",	\
	    (x & Z80_SR_SIGN) ? 1 : 0,					\
	    (x & Z80_SR_ZERO) ? 1 : 0,					\
	    (x & Z80_SR_HALF_CARRY) ? 1 : 0,				\
	    (x & Z80_SR_PARITY_OVERFLOW) ? 1 : 0,			\
	    (x & Z80_SR_ADD_SUB) ? 1 : 0,				\
	    (x & Z80_SR_CARRY) ? 1 : 0);
/**
 * Pretty print Z80 registers using z80_state from class md.
 */
void md::debug_show_z80_regs()
{
	int			i;

	z80_state_dump();
	printf("z80:\n");

	for (i = 0; i < 2; i++) {
		const char *alt = (i ? "'" : "");

		printf("\t af%s:\t0x%04x\n",
		       alt, le2h16(z80_state.alt[i].fa));
		PRINT_Z80_FLAGS(z80_state.alt[i].fa >> 1);
		printf("\t bc%s:\t0x%04x\n"
		       "\t de%s:\t0x%04x\n"
		       "\t hl%s:\t0x%04x\n",
		       alt, le2h16(z80_state.alt[i].cb),
		       alt, le2h16(z80_state.alt[i].ed),
		       alt, le2h16(z80_state.alt[i].lh));
	}

	printf("\t ix:\t0x%04x\n"
	    "\t iy:\t0x%04x\n"
	    "\t sp:\t0x%04x\n"
	    "\t pc:\t0x%04x\n"
	    "\t  r:\t0x%02x\n"
	    "\t  i:\t0x%02x\n"
	    "\tiff:\t0x%02x\n"
	    "\t im:\t0x%02x\n",
	    le2h16(z80_state.ix),
	    le2h16(z80_state.iy),
	    le2h16(z80_state.sp),
	    le2h16(z80_state.pc),
	    z80_state.r,
	    z80_state.i,
	    z80_state.iff,
	    z80_state.im);
	fflush(stdout);
}

/**
 * Help (help) command handler.
 *
 * @param n_args Number of arguments (ignored).
 * @param args List of arguments (ignored).
 * @return Always 1.
 */
int md::debug_cmd_help(int n_args, char **args)
{
	(void) n_args;
	(void) args;

	printf("commands:\n"
	    "\tC/cpu <cpu>\t\tswitch to cpu context\n"
	    "\t-b/-break <#num/addr>\tremove breakpoint for current cpu\n"
	    "\tb/break <addr>\t\tset breakpoint for current cpu\n"
	    "\tb/break\t\t\tshow breakpoints for current cpu\n"
	    "\tc/cont\t\t\texit debugger and continue execution\n"
	    "\td/dis <addr> <num>\tdisasm 'num' instrs starting at 'addr'\n"
	    "\td/dis <addr>\t\tdisasm %u instrs starting at 'addr'\n"
	    "\td/dis\t\t\tdisasm %u instrs starting at the current instr\n"
	    "\tm/mem <addr> <len>\tdump 'len' bytes of memory at 'addr'\n"
	    "\tm/mem <addr>\t\tdump %u bytes of memory at 'addr'\n"
	    "\tset/setb <addr> <val>\twrite byte 'val' to memory at 'addr'\n"
	    "\tsetw <addr> <val>\twrite word 'val' to memory at 'addr'\n"
	    "\tsetl <addr> <val>\twrite long 'val' to memory at 'addr'\n"
	    "\tsetr <reg> <val>\twrite 'val' to register 'reg'\n"
	    "\th/help/?\t\tshow this message\n"
	    "\tr/reg\t\t\tshow registers of current cpu\n"
	    "\tcount\t\t\ttoggle instructions counters\n"
	    "\ts/step\t\t\tstep one instruction\n"
	    "\ts/step <num>\t\tstep 'num' instructions\n"
	    "\tt/trace [bool|num]\ttoggle instructions tracing\n"
	    "\t-w/-watch <#num/addr>\tremove watchpoint for current cpu\n"
	    "\tw/watch <addr> <len>\tset multi-byte watchpoint for current cpu\n"
	    "\tw/watch <addr>\t\tset 1-byte watchpoint for current cpu\n"
	    "\tw/watch\t\t\tshow watchpoints for current cpu\n"
	    "\ncpu names/numbers:\n"
	    "\t'm68k', 'm', '68000', 'm68000' or '%d' refers to the main m68000 chip\n"
	    "\t'z80', 'z' or '%d' refers to the secondary z80 chip\n"
	    "\t'ym', 'fm', 'ym2612' or '%d' refers to the fm2616 sound chip\n"
	    "\t'sn', 'sn76489', 'psg'  or '%d' refers to the sn76489 sound chip\n",
	    DEBUG_DFLT_DASM_LEN, DEBUG_DFLT_DASM_LEN, DEBUG_DFLT_MEMDUMP_LEN,
	    DBG_CONTEXT_M68K, DBG_CONTEXT_Z80, DBG_CONTEXT_YM2612, DBG_CONTEXT_SN76489);
	fflush(stdout);
	return (1);
}

/**
 * Dump registers (reg) command handler.
 * This command pretty prints registers for the current context.
 *
 * @param n_args Number of arguments (ignored).
 * @param[in] args List of arguments (ignored).
 * @return Always 1.
 */
int md::debug_cmd_reg(int n_args, char **args)
{
	(void) n_args;
	(void) args;

	switch (debug_context) {
	case DBG_CONTEXT_M68K:
		debug_show_m68k_regs();
		break;
	case DBG_CONTEXT_Z80:
		debug_show_z80_regs();
		break;
	case DBG_CONTEXT_YM2612:
		debug_show_ym2612_regs();
		break;
	default:
		printf("register dump not implemented on %s core\n",
		    CURRENT_DEBUG_CONTEXT_NAME);
		break;
	};
	fflush(stdout);
	return (1);
}

/**
 * Instructions counters toggle (count) command handler.
 *
 * - If n_args == 0, toggle (enable/disable) instructions counters.
 * - If n_args == 1 and args[0] is a boolean string, toggle instructions
 *   counters accordingly.
 *
 * @param n_args Number of arguments.
 * @param args Arguments.
 * @return Always 1.
 */
int md::debug_cmd_count(int n_args, char **args)
{
	static const struct {
		const char *param;
		bool value;
	} opt[] = {
		{ "true", true }, { "false", false },
		{ "yes", true }, { "no", false },
		{ "on", true }, { "off", false },
		{ "enable", true }, { "disable", false }
	};

	if (n_args == 1) {
		uint32_t i;

		for (i = 0; (i != (sizeof(opt) / sizeof(opt[0]))); ++i)
			if (!strcasecmp(args[0], opt[i].param)) {
				debug_instr_count_enabled = opt[i].value;
				break;
			}
		if (i == (sizeof(opt) / sizeof(opt[0]))) {
			if (debug_strtou32(args[0], &i) == -1) {
				printf("invalid argument: %s\n", args[0]);
				goto out;
			}
			debug_instr_count_enabled = !!i;
		}
	}
	else
		debug_instr_count_enabled = !debug_instr_count_enabled;
	printf("instructions counters ");
	if (!debug_instr_count_enabled)
		printf("disabled.\n");
	else {
		debug_m68k_instr_count = 0;
		debug_z80_instr_count = 0;
		printf("enabled.\n");
	}
out:
	fflush(stdout);
	return 1;
}

/**
 * Breakpoint (break) command handler.
 *
 * - If n_args == 0, list breakpoints.
 * - If n_args == 1, set a breakpoint at address args[0].
 *
 * @param n_args Number of arguments.
 * @param[in] args List of arguments.
 * @return Always 1.
 */
int md::debug_cmd_break(int n_args, char **args)
{
	uint32_t		num;

	if ((debug_context != DBG_CONTEXT_M68K) &&
	    (debug_context != DBG_CONTEXT_Z80)) {
		printf("breakpoints are not supported on %s\n",
		       CURRENT_DEBUG_CONTEXT_NAME);
		goto out;
	}

	switch (n_args) {
	case 1:
		// setting a bp
		if ((debug_strtou32(args[0], &num)) < 0) {
			printf("address malformed: %s\n", args[0]);
			goto out;
		}
		if (debug_context == DBG_CONTEXT_M68K)
			debug_set_bp_m68k(num);
		else if (debug_context == DBG_CONTEXT_Z80)
			debug_set_bp_z80(num);
		break;
	case 0:
		// listing bps
		if (debug_context == DBG_CONTEXT_M68K)
			debug_list_bps_m68k();
		else if (debug_context == DBG_CONTEXT_Z80)
			debug_list_bps_z80();
	};

out:
	fflush(stdout);
	return (1);
}

/**
 * Quit (quit) command handler.
 * This command makes DGen/SDL quit.
 *
 * @param n_args Number of arguments (ignored).
 * @param args List of arguments (ignored).
 * @return Always -1.
 */
int md::debug_cmd_quit(int n_args, char **args)
{
	(void) n_args;
	(void) args;
	debug_leave();
	return -1;
}

/**
 * Core/CPU selection (cpu) command.
 * Switch to the core/CPU given in args[0].
 *
 * @param n_args Number of arguments (should always be 1).
 * @param args List of arguments.
 * @return Always 1.
 */
int md::debug_cmd_cpu(int n_args, char **args)
{
	(void) n_args;

	int			ctx;

	ctx = debug_parse_cpu(args[0]);
	if (ctx < 0) {
		printf("unknown cpu: %s\n", args[0]);
		goto out;
	}

	debug_context = ctx;
out:
	fflush(stdout);
	return (1);
}

/**
 * Step (step) command handler for the current core.
 *
 * - If n_args == 0, step one instruction.
 * - If n_args == 1, step args[0] instructions.
 *
 * @param n_args Number of arguments.
 * @param args List of arguments.
 * @return 0 if execution should continue, 1 otherwise.
 */
int md::debug_cmd_step(int n_args, char **args)
{
	uint32_t		num = 1;

	if ((n_args >= 1) && (debug_strtou32(args[0], &num) < 0)) {
		printf("malformed number: %s\n", args[0]);
		goto out;
	}
	if (debug_context == DBG_CONTEXT_M68K)
		debug_step_m68k = (num + 1); /* triggered when 0 */
	else if (debug_context == DBG_CONTEXT_Z80)
		debug_step_z80 = (num + 1);
	else {
		printf("stepping not supported on %s\n",
		       CURRENT_DEBUG_CONTEXT_NAME);
		goto out;
	}
	fflush(stdout);
	debug_trap = false;
	return (0); // continue executing
out:
	fflush(stdout);
	return (1);
}

/**
 * Trace toggle (trace) command handler.
 *
 * - If n_args == 0, toggle (enable/disable) instructions tracing permanently.
 * - If n_args == 1 and args[0] is a number, enable instructions tracing for
 *   this number of instructions.
 * - If n_args == 1 and args[0] is a boolean string, toggle instructions
 *   tracing accordingly.
 *
 * @param n_args Number of arguments.
 * @param args Arguments.
 * @return Always 1.
 */
int md::debug_cmd_trace(int n_args, char **args)
{
	static const struct {
		const char *param;
		unsigned int value;
	} opt[] = {
		{ "true", ~0u }, { "false", 0 },
		{ "yes", ~0u }, { "no", 0 },
		{ "on", ~0u }, { "off", 0 },
		{ "enable", ~0u }, { "disable", 0 }
	};
	unsigned int *which;

	if ((which = &debug_trace_m68k, debug_context != DBG_CONTEXT_M68K) &&
	    (which = &debug_trace_z80, debug_context != DBG_CONTEXT_Z80)) {
		printf("instructions tracing not supported on %s\n",
		       CURRENT_DEBUG_CONTEXT_NAME);
		goto out;
	}
	if (n_args == 1) {
		uint32_t i;

		for (i = 0; (i != (sizeof(opt) / sizeof(opt[0]))); ++i)
			if (!strcasecmp(args[0], opt[i].param)) {
				*which = opt[i].value;
				break;
			}
		if (i == (sizeof(opt) / sizeof(opt[0]))) {
			if (debug_strtou32(args[0], &i) == -1) {
				printf("invalid argument: %s\n", args[0]);
				goto out;
			}
			*which = i;
		}
	}
	else
		*which = (!*which * ~0u);
	printf("instructions tracing ");
	switch (*which) {
	case 0:
		printf("disabled.\n");
		break;
	case ~0u:
		printf("enabled permanently.\n");
		break;
	default:
		printf("enabled for the next %u instruction(s).\n", *which);
		break;
	}
out:
	fflush(stdout);
	return 1;
}

/**
 * Watchpoint removal (-watch) command handler.
 *
 * If args[0] starts with a #, remove a watchpoint by ID. Otherwise, remove
 * it by address.
 *
 * @param n_args Number of arguments (always 1).
 * @param[in] args List of arguments.
 * @return Always 1.
 */
int md::debug_cmd_minus_watch(int n_args, char **args)
{
	int			index = -1;
	uint32_t		num;

	(void) n_args;

	if ((debug_context != DBG_CONTEXT_M68K) &&
	    (debug_context != DBG_CONTEXT_Z80)) {
		printf("watchpoints not supported on %s\n",
		       CURRENT_DEBUG_CONTEXT_NAME);
		goto out;
	}

	if (args[0][0] == '#') { // remove by index

		if (strlen(args[0]) < 2) {
			printf("parse error\n");
			goto out;
		}

		if ((debug_strtou32(args[0]+1, &num)) < 0) {
			printf("address malformed: %s\n", args[0]);
			goto out;
		}
		index = num;

		if ((index < 0) || (index >= MAX_BREAKPOINTS)) {
			printf("breakpoint out of range\n");
			goto out;
		}
	} else { // remove by address
		if ((debug_strtou32(args[0], &num)) < 0) {
			printf("address malformed: %s\n", args[0]);
			goto out;
		}
		if (debug_context == DBG_CONTEXT_M68K)
			index = debug_find_wp_m68k(num);
		else if (debug_context == DBG_CONTEXT_Z80)
			index = debug_find_wp_z80(num);
	}
	if (debug_context == DBG_CONTEXT_M68K)
		debug_rm_wp_m68k(index);
	else if (debug_context == DBG_CONTEXT_Z80)
		debug_rm_wp_z80(index);
out:
	fflush(stdout);
	return (1);
}


/**
 * Breakpoint removal (-break) command handler.
 *
 * If args[0] starts with a #, remove a breakpoint by ID. Otherwise, remove
 * it by address.
 *
 * @param n_args Number of arguments (always 1).
 * @param args List of arguments.
 * @return Always 1.
 */
int md::debug_cmd_minus_break(int n_args, char **args)
{
	int			index = -1;
	uint32_t		num;

	(void) n_args;

	if ((debug_context != DBG_CONTEXT_M68K) &&
	    (debug_context != DBG_CONTEXT_Z80)) {
		printf("breakpoints not supported on %s\n",
		       CURRENT_DEBUG_CONTEXT_NAME);
		goto out;
	}

	if (args[0][0] == '#') { // remove by index

		if (strlen(args[0]) < 2) {
		    printf("parse error\n");
		    goto out;
		}

		if ((debug_strtou32(args[0]+1, &num)) < 0) {
			printf("address malformed: %s\n", args[0]);
			goto out;
		}
		index = num;

		if ((index < 0) || (index >= MAX_BREAKPOINTS)) {
			printf("breakpoint out of range\n");
			goto out;
		}

	} else { // remove by address
		if ((debug_strtou32(args[0], &num)) < 0) {
			printf("address malformed: %s\n", args[0]);
			goto out;
		}
		if (debug_context == DBG_CONTEXT_M68K)
			index = debug_find_bp_m68k(num);
		else if (debug_context == DBG_CONTEXT_Z80)
			index = debug_find_bp_z80(num);
		if (index < 0) {
			printf("no breakpoint here: %s\n", args[0]);
			goto out;
		}
	}
	// we now have an index into our bp array
	if (debug_context == DBG_CONTEXT_M68K)
		debug_rm_bp_m68k(index);
	else if (debug_context == DBG_CONTEXT_Z80)
		debug_rm_bp_z80(index);
out:
	fflush(stdout);
	return (1);
}

/**
 * Dispatch a command to the relevant handler method.
 *
 * @param n_toks Number of tokens (arguments).
 * @param toks List of tokens (arguments).
 * @return 1 if n_toks is 0, otherwise the command handler's return value.
 */
int md::debug_despatch_cmd(int n_toks, char **toks)
{

	struct md::dgen_debugger_cmd	 *d = (struct md::dgen_debugger_cmd *)
	    md::debug_cmd_list, *found = NULL;

	if (n_toks == 0)
		return 1;
	while (d->cmd != NULL) {
		if ((strcmp(toks[0], d->cmd) == 0) && (n_toks-1 == d->n_args)) {
			found = d;
			break;
		}
		d++;
	}

	if (found == NULL) {
		printf("unknown command/wrong argument count (type '?' for help)\n");
		fflush(stdout);
		return (1);
	}

	// all is well, call
	return ((this->*found->handler)(n_toks-1, &toks[1]));
}

#define MAX_DISASM		128
/**
 * Pretty print a M68K disassembly.
 *
 * @param from Address to start disassembling from.
 * @param len Number of instructions to disassemble.
 */
void md::debug_print_m68k_disassemble(uint32_t from, int len)
{
#ifdef WITH_MUSA
	int			i;
	char			disasm[MAX_DISASM];

	md_set_musa(1);	// assign static in md:: so C can get at it (HACK)
	for (i = 0; i < len; i++) {
		unsigned int sz;

		sz = m68k_disassemble(disasm, from, M68K_CPU_TYPE_68040);
		printf("   0x%06x: %s\n", from, disasm);
		from += sz;
	}
	md_set_musa(0);
#else
	(void)len;
	printf("   0x%06x: %02x %02x [...]\n", from,
	       misc_readbyte(from), misc_readbyte(from + 1));
#endif
	fflush(stdout);
}

/**
 * Pretty print a Z80 disassembly.
 *
 * @param from Address to start disassembling from.
 * @param num Number of instructions to disassemble.
 */
void md::debug_print_z80_disassemble(uint16_t from, unsigned int num)
{
#ifdef WITH_DZ80
	unsigned int i;

	for (i = 0; (i < num); ++i) {
		int err;

		disz80.start = from;
		disz80.end = from;
		err = dZ80_Disassemble(&disz80);
		if (err != DERR_NONE)
			printf("   0x%04x: ??? error: %s\n",
			       from, dZ80_GetErrorText(err));
		else
			printf("   0x%04x: %8s %s\n",
			       from, disz80.hexDisBuf, disz80.disBuf);
		from += disz80.bytesProcessed;
	}
#else
	(void)num;
	printf("   0x%04x: %02x [...]\n", from, z80_read(from));
#endif
	fflush(stdout);
}

/**
 * Leave debugger.
 */
void md::debug_leave()
{
	if (debug_trap == false)
		return;
	linenoise_nb_clean();
	debug_trap = false;
}

/**
 * Enter debugger and show command prompt.
 */
int md::debug_enter()
{
	char				*cmd, prompt[32];
	char				*p, *next;
	char				*toks[MAX_DEBUG_TOKS];
	int				 n_toks = 0;
	uint32_t			 m68k_pc = m68k_get_pc();
	uint16_t			 z80_pc = z80_get_pc();
	int				 ret;

	if (debug_trap == false) {
		pd_message("Debug trap.");
		debug_trap = true;

		if (debug_context == DBG_CONTEXT_M68K)
			debug_print_m68k_disassemble(m68k_pc, 1);
		else if (debug_context == DBG_CONTEXT_Z80)
			debug_print_z80_disassemble(z80_pc, 1);
	}

	switch (debug_context) {
	case DBG_CONTEXT_M68K:
		snprintf(prompt, sizeof(prompt), "m68k:0x%08x> ", m68k_pc);
		break;
	case DBG_CONTEXT_Z80:
		snprintf(prompt, sizeof(prompt), "z80:0x%04x> ", z80_pc);
		break;
	case DBG_CONTEXT_YM2612:
		snprintf(prompt, sizeof(prompt), "ym2612> ");
		break;
	case DBG_CONTEXT_SN76489:
		snprintf(prompt, sizeof(prompt), "sn76489> ");
		break;
	default:
		printf("unknown cpu. should not happen\n");
		fflush(stdout);
		return 0;
	};

	if ((cmd = linenoise_nb(prompt)) == NULL) {
		if (!linenoise_nb_eol())
			return 0;
		linenoise_nb_clean();
		return 0;
	}

	linenoiseHistoryAdd((const char *)cmd);

	// tokenise
	next = p = cmd;
	n_toks = 0;
	while ((n_toks < MAX_DEBUG_TOKS) &&
	       ((p = strtok(next, " \t")) != NULL)) {
		toks[n_toks++] = p;
		next = NULL;
	}

	ret = debug_despatch_cmd(n_toks,  toks);
	free(cmd);
	return ret;
}


/**
 * List of commands.
 */
const struct md::dgen_debugger_cmd md::debug_cmd_list[] = {
		// breakpoints
		{(char *) "break",	1,	&md::debug_cmd_break},
		{(char *) "b",		1,	&md::debug_cmd_break},
		{(char *) "break",	0,	&md::debug_cmd_break},
		{(char *) "b",		0,	&md::debug_cmd_break},
		{(char *) "-break",	1,	&md::debug_cmd_minus_break},
		{(char *) "-b",		1,	&md::debug_cmd_minus_break},
		// switch debug context
		{(char *) "C",		1,	&md::debug_cmd_cpu},
		{(char *) "cpu",	1,	&md::debug_cmd_cpu},
		// continue emulation
		{(char *) "cont",	0,	&md::debug_cmd_cont},
		{(char *) "c",		0,	&md::debug_cmd_cont},
		// disassemble
		{(char *) "dis",	0,	&md::debug_cmd_dis},
		{(char *) "d",		0,	&md::debug_cmd_dis},
		{(char *) "dis",	1,	&md::debug_cmd_dis},
		{(char *) "d",		1,	&md::debug_cmd_dis},
		{(char *) "dis",	2,	&md::debug_cmd_dis},
		{(char *) "d",		2,	&md::debug_cmd_dis},
		// help
		{(char *) "help",	0,	&md::debug_cmd_help},
		{(char *) "h",		0,	&md::debug_cmd_help},
		{(char *) "?",		0,	&md::debug_cmd_help},
		// memory
		{(char *) "m",		1,	&md::debug_cmd_mem},
		{(char *) "mem",	1,	&md::debug_cmd_mem},
		{(char *) "m",		2,	&md::debug_cmd_mem},
		{(char *) "mem",	2,	&md::debug_cmd_mem},
		{(char *) "set",	2,	&md::debug_cmd_setb},
		{(char *) "setb",	2,	&md::debug_cmd_setb},
		{(char *) "setw",	2,	&md::debug_cmd_setw},
		{(char *) "setl",	2,	&md::debug_cmd_setl},
		{(char *) "setr",	2,	&md::debug_cmd_setr},
		// quit
		{(char *) "quit",	0,	&md::debug_cmd_quit},
		{(char *) "q",		0,	&md::debug_cmd_quit},
		{(char *) "exit",	0,	&md::debug_cmd_quit},
		{(char *) "e",		0,	&md::debug_cmd_quit},
		// dump registers
		{(char *) "reg",	0,	&md::debug_cmd_reg},
		{(char *) "r",		0,	&md::debug_cmd_reg},
		{(char *) "count",	1,	&md::debug_cmd_count},
		{(char *) "count",	0,	&md::debug_cmd_count},
		// step
		{(char *) "step",	1,	&md::debug_cmd_step},
		{(char *) "s",		1,	&md::debug_cmd_step},
		{(char *) "step",	0,	&md::debug_cmd_step},
		{(char *) "s",		0,	&md::debug_cmd_step},
		{(char *) "trace",	1,	&md::debug_cmd_trace},
		{(char *) "t",		1,	&md::debug_cmd_trace},
		{(char *) "trace",	0,	&md::debug_cmd_trace},
		{(char *) "t",		0,	&md::debug_cmd_trace},
		// watch points
		{(char *) "watch",	2,	&md::debug_cmd_watch},
		{(char *) "w",		2,	&md::debug_cmd_watch},
		{(char *) "watch",	1,	&md::debug_cmd_watch},
		{(char *) "w",		1,	&md::debug_cmd_watch},
		{(char *) "watch",	0,	&md::debug_cmd_watch},
		{(char *) "w",		0,	&md::debug_cmd_watch},
		{(char *) "-watch",	1,	&md::debug_cmd_minus_watch},
		{(char *) "-w",		1,	&md::debug_cmd_minus_watch},
		// sentinal
		{NULL,                  0,      NULL}
	};
