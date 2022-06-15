// DGen/SDL v1.29+
// Megadrive 1 Frame module
// Many, many thanks to John Stiles for the new structure of this module! :)
// And kudos to Gens (Gens/GS) and Genplus (GX) authors -- zamaz

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#ifdef HAVE_MEMCPY_H
#include "memcpy.h"
#endif
#include "md.h"
#include "debug.h"
#include "rc-vars.h"

// Set and unset contexts (Musashi, StarScream, MZ80)

#ifdef WITH_MUSA
class md* md::md_musa(0);

bool md::md_set_musa(bool set)
{
	if (set) {
		++md_musa_ref;
		if (md_musa == this)
			return true;
		md_musa_prev = md_musa;
		md_musa = this;
		md_set_musa_sync(true);
		return false;
	}
	else {
		if (md_musa != this)
			abort();
		if (--md_musa_ref != 0)
			return true;
		md_set_musa_sync(false);
		md_musa = md_musa_prev;
		md_musa_prev = 0;
		return true;
	}
}

void md::md_set_musa_sync(bool push)
{
	unsigned int i, j;

	if (push) {
		m68k_set_context(ctx_musa);
		for (i = M68K_REG_D0, j = 0; (i <= M68K_REG_D7); ++i, ++j)
			m68k_set_reg((m68k_register_t)i,
				     le2h32(m68k_state.d[j]));
		for (i = M68K_REG_A0, j = 0; (i <= M68K_REG_A7); ++i, ++j)
			m68k_set_reg((m68k_register_t)i,
				     le2h32(m68k_state.a[j]));
		m68k_set_reg(M68K_REG_PC, le2h32(m68k_state.pc));
		m68k_set_reg(M68K_REG_SR, le2h16(m68k_state.sr));
	}
	else {
		for (i = M68K_REG_D0, j = 0; (i <= M68K_REG_D7); ++i, ++j)
			m68k_state.d[j] =
				h2le32(m68k_get_reg(NULL, (m68k_register_t)i));
		for (i = M68K_REG_A0, j = 0; (i <= M68K_REG_A7); ++i, ++j)
			m68k_state.a[j] =
				h2le32(m68k_get_reg(NULL, (m68k_register_t)i));
		m68k_state.pc = h2le32(m68k_get_reg(NULL, M68K_REG_PC));
		m68k_state.sr = h2le16(m68k_get_reg(NULL, M68K_REG_SR));
		m68k_get_context(ctx_musa);
	}
}
#endif // WITH_MUSA

#ifdef WITH_STAR
class md* md::md_star(0);

bool md::md_set_star(bool set)
{
	if (set) {
		++md_star_ref;
		if (md_star == this)
			return true;
		md_star_prev = md_star;
		md_star = this;
		md_set_star_sync(true);
		return false;
	}
	else {
		if (md_star != this)
			abort();
		if (--md_star_ref != 0)
			return true;
		md_set_star_sync(false);
		md_star = md_star_prev;
		md_star_prev = 0;
		return true;
	}
}

void md::md_set_star_sync(bool push)
{
	unsigned int i;

	if (push) {
		for (i = 0; (i < 8); ++i) {
			cpu.dreg[i] = le2h32(m68k_state.d[i]);
			cpu.areg[i] = le2h32(m68k_state.a[i]);
		}
		cpu.pc = le2h32(m68k_state.pc);
		cpu.sr = le2h16(m68k_state.sr);
		s68000SetContext(&cpu);
	}
	else {
		s68000GetContext(&cpu);
		for (i = 0; (i < 8); ++i) {
			m68k_state.d[i] = h2le32(cpu.dreg[i]);
			m68k_state.a[i] = h2le32(cpu.areg[i]);
		}
		m68k_state.pc = h2le32(cpu.pc);
		m68k_state.sr = h2le16(cpu.sr);
	}
}
#endif // WITH_STAR

#ifdef WITH_CYCLONE
class md* md::md_cyclone(0);

bool md::md_set_cyclone(bool set)
{
	if (set) {
		++md_cyclone_ref;
		if (md_cyclone == this)
			return true;
		md_cyclone_prev = md_cyclone;
		md_cyclone = this;
		md_set_cyclone_sync(true);
		return false;
	}
	else {
		if (md_cyclone != this)
			abort();
		if (--md_cyclone_ref != 0)
			return true;
		md_set_cyclone_sync(false);
		md_cyclone = md_cyclone_prev;
		md_cyclone_prev = 0;
		return true;
	}
}

void md::md_set_cyclone_sync(bool push)
{
	unsigned int i;

	if (push) {
		for (i = 0; (i < 8); ++i) {
			cyclonecpu.d[i] = le2h32(m68k_state.d[i]);
			cyclonecpu.a[i] = le2h32(m68k_state.a[i]);
		}
		cyclonecpu.membase = 0;
		cyclonecpu.pc = cyclonecpu.checkpc(le2h32(m68k_state.pc));
		CycloneSetSr(&cyclonecpu, le2h16(m68k_state.sr));
	}
	else {
		for (i = 0; (i < 8); ++i) {
			m68k_state.d[i] = h2le32(cyclonecpu.d[i]);
			m68k_state.a[i] = h2le32(cyclonecpu.a[i]);
		}
		m68k_state.pc = h2le32(cyclonecpu.pc-cyclonecpu.membase);
		m68k_state.sr = h2le16(CycloneGetSr(&cyclonecpu));
	}
}
#endif // WITH_CYCLONE

#ifdef WITH_CZ80
bool md::md_set_cz80(bool set)
{
	if (set) {
		if (md_cz80_ref++)
			return true;
		md_set_cz80_sync(true);
		return false;
	}
	else {
		if (md_cz80_ref == 0)
			abort();
		if (--md_cz80_ref)
			return true;
		md_set_cz80_sync(false);
		return true;
	}
}

void md::md_set_cz80_sync(bool push)
{
	if (push) {
		Cz80_Set_AF(&cz80, le2h16(z80_state.alt[0].fa));
		Cz80_Set_BC(&cz80, le2h16(z80_state.alt[0].cb));
		Cz80_Set_DE(&cz80, le2h16(z80_state.alt[0].ed));
		Cz80_Set_HL(&cz80, le2h16(z80_state.alt[0].lh));
		Cz80_Set_AF2(&cz80, le2h16(z80_state.alt[1].fa));
		Cz80_Set_BC2(&cz80, le2h16(z80_state.alt[1].cb));
		Cz80_Set_DE2(&cz80, le2h16(z80_state.alt[1].ed));
		Cz80_Set_HL2(&cz80, le2h16(z80_state.alt[1].lh));
		Cz80_Set_IX(&cz80, le2h16(z80_state.ix));
		Cz80_Set_IY(&cz80, le2h16(z80_state.iy));
		Cz80_Set_SP(&cz80, le2h16(z80_state.sp));
		Cz80_Set_PC(&cz80, le2h16(z80_state.pc));
		Cz80_Set_R(&cz80, z80_state.r);
		Cz80_Set_I(&cz80, z80_state.i);
		Cz80_Set_IFF(&cz80, z80_state.iff);
		Cz80_Set_IM(&cz80, z80_state.im);
	}
	else {
		z80_state.alt[0].fa = h2le16(Cz80_Get_AF(&cz80));
		z80_state.alt[0].cb = h2le16(Cz80_Get_BC(&cz80));
		z80_state.alt[0].ed = h2le16(Cz80_Get_DE(&cz80));
		z80_state.alt[0].lh = h2le16(Cz80_Get_HL(&cz80));
		z80_state.alt[1].fa = h2le16(Cz80_Get_AF2(&cz80));
		z80_state.alt[1].cb = h2le16(Cz80_Get_BC2(&cz80));
		z80_state.alt[1].ed = h2le16(Cz80_Get_DE2(&cz80));
		z80_state.alt[1].lh = h2le16(Cz80_Get_HL2(&cz80));
		z80_state.ix = h2le16(Cz80_Get_IX(&cz80));
		z80_state.iy = h2le16(Cz80_Get_IY(&cz80));
		z80_state.sp = h2le16(Cz80_Get_SP(&cz80));
		z80_state.pc = h2le16(Cz80_Get_PC(&cz80));
		z80_state.r = Cz80_Get_R(&cz80);
		z80_state.i = Cz80_Get_I(&cz80);
		z80_state.iff = Cz80_Get_IFF(&cz80);
		z80_state.im = Cz80_Get_IM(&cz80);
	}
}
#endif // WITH_CZ80

#ifdef WITH_MZ80
class md* md::md_mz80(0);

bool md::md_set_mz80(bool set)
{
	if (set) {
		++md_mz80_ref;
		if (md_mz80 == this)
			return true;
		md_mz80_prev = md_mz80;
		md_mz80 = this;
		md_set_mz80_sync(true);
		return false;
	}
	else {
		if (md_mz80 != this)
			abort();
		if (--md_mz80_ref != 0)
			return true;
		md_set_mz80_sync(false);
		md_mz80 = md_mz80_prev;
		md_mz80_prev = 0;
		return true;
	}
}

void md::md_set_mz80_sync(bool push)
{
	if (push) {
		z80.z80AF = le2h16(z80_state.alt[0].fa);
		z80.z80BC = le2h16(z80_state.alt[0].cb);
		z80.z80DE = le2h16(z80_state.alt[0].ed);
		z80.z80HL = le2h16(z80_state.alt[0].lh);
		z80.z80afprime = le2h16(z80_state.alt[1].fa);
		z80.z80bcprime = le2h16(z80_state.alt[1].cb);
		z80.z80deprime = le2h16(z80_state.alt[1].ed);
		z80.z80hlprime = le2h16(z80_state.alt[1].lh);
		z80.z80IX = le2h16(z80_state.ix);
		z80.z80IY = le2h16(z80_state.iy);
		z80.z80sp = le2h16(z80_state.sp);
		z80.z80pc = le2h16(z80_state.pc);
		z80.z80r = z80_state.r;
		z80.z80i = z80_state.i;
		z80.z80iff = z80_state.iff;
		z80.z80interruptMode = z80_state.im;
		mz80SetContext(&z80);
	}
	else {
		mz80GetContext(&z80);
		z80_state.alt[0].fa = h2le16(z80.z80AF);
		z80_state.alt[0].cb = h2le16(z80.z80BC);
		z80_state.alt[0].ed = h2le16(z80.z80DE);
		z80_state.alt[0].lh = h2le16(z80.z80HL);
		z80_state.alt[1].fa = h2le16(z80.z80afprime);
		z80_state.alt[1].cb = h2le16(z80.z80bcprime);
		z80_state.alt[1].ed = h2le16(z80.z80deprime);
		z80_state.alt[1].lh = h2le16(z80.z80hlprime);
		z80_state.ix = h2le16(z80.z80IX);
		z80_state.iy = h2le16(z80.z80IY);
		z80_state.sp = h2le16(z80.z80sp);
		z80_state.pc = h2le16(z80.z80pc);
		z80_state.r = z80.z80r;
		z80_state.i = z80.z80i;
		z80_state.iff = z80.z80iff;
		z80_state.im = z80.z80interruptMode;
	}
}
#endif // WITH_MZ80

#ifdef WITH_DRZ80
class md* md::md_drz80(0);

bool md::md_set_drz80(bool set)
{
	if (set) {
		++md_drz80_ref;
		if (md_drz80 == this)
			return true;
		md_drz80_prev = md_drz80;
		md_drz80 = this;
		md_set_drz80_sync(true);
		return false;
	}
	else {
		if (md_drz80 != this)
			abort();
		if (--md_drz80_ref != 0)
			return true;
		md_set_drz80_sync(false);
		md_drz80 = md_drz80_prev;
		md_drz80_prev = 0;
		return true;
	}
}

void md::md_set_drz80_sync(bool push)
{
	if (push) {
		drz80.Z80A = ((le2h16(z80_state.alt[0].fa) & 0xff00) << 16);
		drz80.Z80F = (le2h16(z80_state.alt[0].fa) & 0x00ff);
		drz80.Z80BC = (le2h16(z80_state.alt[0].cb) << 16);
		drz80.Z80DE = (le2h16(z80_state.alt[0].ed) << 16);
		drz80.Z80HL = (le2h16(z80_state.alt[0].lh) << 16);
		drz80.Z80A2 = ((le2h16(z80_state.alt[1].fa) & 0xff00) << 16);
		drz80.Z80F2 = ((le2h16(z80_state.alt[1].fa) & 0x00ff) << 24);
		drz80.Z80BC2 = (le2h16(z80_state.alt[1].cb) << 16);
		drz80.Z80DE2 = (le2h16(z80_state.alt[1].ed) << 16);
		drz80.Z80HL2 = (le2h16(z80_state.alt[1].lh) << 16);
		drz80.Z80IX = (le2h16(z80_state.ix) << 16);
		drz80.Z80IY = (le2h16(z80_state.iy) << 16);
		drz80.Z80SP_BASE = (uintptr_t)z80ram;
		drz80.Z80PC_BASE = (uintptr_t)z80ram;
		drz80.Z80SP = (drz80.Z80SP_BASE + le2h16(z80_state.sp));
		drz80.Z80PC = (drz80.Z80PC_BASE + le2h16(z80_state.pc));
		drz80.Z80R = z80_state.r;
		drz80.Z80I = z80_state.i;
		drz80.Z80IF = z80_state.iff;
		drz80.Z80IM = z80_state.im;
	}
	else {
		z80_state.alt[0].fa = h2le16(((drz80.Z80A >> 16) & 0xff00) |
					     (drz80.Z80F & 0x00ff));
		z80_state.alt[0].cb = h2le16(drz80.Z80BC >> 16);
		z80_state.alt[0].ed = h2le16(drz80.Z80DE >> 16);
		z80_state.alt[0].lh = h2le16(drz80.Z80HL >> 16);
		z80_state.alt[1].fa = h2le16(((drz80.Z80A2 >> 16) & 0xff00) |
					     ((drz80.Z80F2 >> 24) & 0x00ff));
		z80_state.alt[1].cb = h2le16(drz80.Z80BC2 >> 16);
		z80_state.alt[1].ed = h2le16(drz80.Z80DE2 >> 16);
		z80_state.alt[1].lh = h2le16(drz80.Z80HL2 >> 16);
		z80_state.ix = h2le16(drz80.Z80IX >> 16);
		z80_state.iy = h2le16(drz80.Z80IY >> 16);
		z80_state.sp = h2le16(drz80.Z80SP - drz80.Z80SP_BASE);
		z80_state.pc = h2le16(drz80.Z80PC - drz80.Z80PC_BASE);
		z80_state.r = drz80.Z80R;
		z80_state.i = drz80.Z80I;
		z80_state.iff = drz80.Z80IF;
		z80_state.im = drz80.Z80IM;
	}
}
#endif // WITH_DRZ80

// Set/unset contexts
void md::md_set(bool set)
{
#ifdef WITH_MUSA
	if (cpu_emu == CPU_EMU_MUSA)
		md_set_musa(set);
	else
#endif
#ifdef WITH_CYCLONE
	if (cpu_emu == CPU_EMU_CYCLONE)
		md_set_cyclone(set);
	else
#endif
#ifdef WITH_STAR
	if (cpu_emu == CPU_EMU_STAR)
		md_set_star(set);
	else
#endif
		(void)0;
#ifdef WITH_CZ80
	if (z80_core == Z80_CORE_CZ80)
		md_set_cz80(set);
	else
#endif
#ifdef WITH_MZ80
	if (z80_core == Z80_CORE_MZ80)
		md_set_mz80(set);
	else
#endif
#ifdef WITH_DRZ80
	if (z80_core == Z80_CORE_DRZ80)
		md_set_drz80(set);
	else
#endif
		(void)0;
}

// Return PC data.
unsigned int md::m68k_read_pc()
{
	static bool rec = false;
	unsigned int pc;

	// Forbid recursion.
	if (rec)
		return h2be16(0xdead);
	rec = true;
#ifdef WITH_MUSA
	if (cpu_emu == CPU_EMU_MUSA) {
		md_set_musa(1);
		pc = m68k_get_reg(NULL, M68K_REG_PC);
		md_set_musa(0);
	}
	else
#endif
#ifdef WITH_CYCLONE
	if (cpu_emu == CPU_EMU_CYCLONE) {
		md_set_cyclone(1);
		pc = (cyclonecpu.pc - cyclonecpu.membase);
		md_set_cyclone(0);
	}
	else
#endif
#ifdef WITH_STAR
	if (cpu_emu == CPU_EMU_STAR) {
		md_set_star(1);
		pc = cpu.pc;
		md_set_star(0);
	}
	else
#endif
		pc = 0;
	pc = misc_readword(pc & 0xffffff);
	rec = false;
	return pc;
}

// Return current M68K odometer
int md::m68k_odo()
{
	if (m68k_st_running) {
#ifdef WITH_MUSA
		if (cpu_emu == CPU_EMU_MUSA)
			return (odo.m68k + m68k_cycles_run());
#endif
#ifdef WITH_CYCLONE
		if (cpu_emu == CPU_EMU_CYCLONE)
			return (odo.m68k +
				((odo.m68k_max - odo.m68k) -
				 cyclonecpu.cycles));
#endif
#ifdef WITH_STAR
		if (cpu_emu == CPU_EMU_STAR)
			return (odo.m68k + s68000readOdometer());
#endif
	}
	return odo.m68k;
}

// Run M68K to odo.m68k_max
void md::m68k_run()
{
	int cycles = (odo.m68k_max - odo.m68k);
#ifdef WITH_DEBUGGER
	int cycles_to_debug = 0;
	int prev_odo = 0;
	bool debug_m68k;
#endif

	if (cycles <= 0)
		return;
	m68k_st_running = 1;
#ifdef WITH_DEBUGGER
	if (debug_trap)
		goto cpu_stalled;
	debug_m68k = (debug_step_m68k ||
		      debug_trace_m68k ||
		      debug_instr_count_enabled ||
		      debug_is_m68k_bp_set() ||
		      debug_is_m68k_wp_set());
	if (debug_m68k) {
		prev_odo = odo.m68k;
		cycles_to_debug = cycles;
		cycles = 1;
	debug_next_instruction:
		if (debug_m68k_check_bps())
			goto cpu_stalled;
	}
#endif
#ifdef WITH_MUSA
	if (cpu_emu == CPU_EMU_MUSA)
		odo.m68k += m68k_execute(cycles);
	else
#endif
#ifdef WITH_STAR
	if (cpu_emu == CPU_EMU_STAR) {
		s68000tripOdometer();
		s68000exec(cycles);
		odo.m68k += s68000readOdometer();
	}
	else
#endif
#ifdef WITH_CYCLONE
	if (cpu_emu == CPU_EMU_CYCLONE) {
		cyclonecpu.cycles = cycles;
		CycloneRun(&cyclonecpu);
		odo.m68k += (cycles - cyclonecpu.cycles);
	}
	else
#endif
		odo.m68k += cycles;
#ifdef WITH_DEBUGGER
	if (debug_m68k) {
		++debug_m68k_instr_count;
		if (debug_m68k_check_wps())
			goto cpu_stalled;
		cycles_to_debug -= (odo.m68k - prev_odo);
		if (cycles_to_debug > 0) {
			prev_odo = odo.m68k;
			goto debug_next_instruction;
		}
	}
cpu_stalled:
#endif
	m68k_st_running = 0;
}

// Issue BUSREQ
void md::m68k_busreq_request()
{
	if (z80_st_busreq)
		return;
	z80_st_busreq = 1;
	if (z80_st_reset)
		return;
	z80_sync(0);
}

// Cancel BUSREQ
void md::m68k_busreq_cancel()
{
	if (!z80_st_busreq)
		return;
	z80_st_busreq = 0;
	z80_sync(1);
}

// Trigger M68K IRQ
void md::m68k_irq(int i)
{
#ifdef WITH_DEBUGGER
	// Ignore interrupts while debugger is active or stepping by a
	// small amount (XXX should be configurable).
	if (debug_trap || (debug_step_m68k && debug_step_m68k < 10000))
		return;
#endif
#ifdef WITH_MUSA
	if (cpu_emu == CPU_EMU_MUSA)
		m68k_set_irq(i);
	else
#endif
#ifdef WITH_CYCLONE
	if (cpu_emu == CPU_EMU_CYCLONE)
		cyclonecpu.irq = i;
	else
#endif
#ifdef WITH_STAR
	if (cpu_emu == CPU_EMU_STAR) {
		if (i)
			s68000interrupt(i, -1);
		else {
			s68000GetContext(&cpu);
			memset(cpu.interrupts, 0, sizeof(cpu.interrupts));
			s68000SetContext(&cpu);
		}
	}
	else
#endif
		(void)i;
}

// Trigger M68K IRQ or disable them according to VDP status.
void md::m68k_vdp_irq_trigger()
{
	if ((vdp.vint_pending) && (vdp.reg[1] & 0x20))
		m68k_irq(6);
	else if ((vdp.hint_pending) && (vdp.reg[0] & 0x10))
		m68k_irq(4);
	else
		m68k_irq(0);
}

// Called whenever M68K acknowledges an interrupt.
void md::m68k_vdp_irq_handler()
{
	if ((vdp.vint_pending) && (vdp.reg[1] & 0x20)) {
		vdp.vint_pending = false;
		coo5 &= ~0x80;
		if ((vdp.hint_pending) && (vdp.reg[0] & 0x10))
			m68k_irq(4);
		else {
			vdp.hint_pending = false;
			m68k_irq(0);
		}
	}
	else {
		vdp.hint_pending = false;
		m68k_irq(0);
	}
}

// Return current Z80 odometer
int md::z80_odo()
{
	if (z80_st_running) {
#ifdef WITH_CZ80
		if (z80_core == Z80_CORE_CZ80)
			return (odo.z80 + Cz80_Get_CycleRemaining(&cz80));
#endif
#ifdef WITH_MZ80
		if (z80_core == Z80_CORE_MZ80)
			return (odo.z80 + mz80GetElapsedTicks(0));
#endif
#ifdef WITH_DRZ80
		if (z80_core == Z80_CORE_DRZ80)
			return (odo.z80 +
				((odo.z80_max - odo.z80) - drz80.cycles));
#endif
	}
	return odo.z80;
}

// Run Z80 to odo.z80_max
void md::z80_run()
{
	int cycles = (odo.z80_max - odo.z80);
#ifdef WITH_DEBUGGER
	int cycles_to_debug = 0;
	int prev_odo = 0;
	bool debug_z80;
#endif

	if (cycles <= 0)
		return;
	z80_st_running = 1;
#ifdef WITH_DEBUGGER
	if (debug_trap)
		goto cpu_stalled;
	debug_z80 = (debug_step_z80 ||
		     debug_trace_z80 ||
		     debug_instr_count_enabled ||
		     debug_is_z80_bp_set() ||
		     debug_is_z80_wp_set());
	if (debug_z80) {
		prev_odo = odo.z80;
		cycles_to_debug = cycles;
		cycles = 1;
	debug_next_instruction:
		if (debug_z80_check_bps())
			goto cpu_stalled;
	}
#endif
	if (z80_st_busreq | z80_st_reset)
		odo.z80 += cycles;
	else {
#ifdef WITH_CZ80
		if (z80_core == Z80_CORE_CZ80)
			odo.z80 += Cz80_Exec(&cz80, cycles);
		else
#endif
#ifdef WITH_MZ80
		if (z80_core == Z80_CORE_MZ80) {
			mz80exec(cycles);
			odo.z80 += mz80GetElapsedTicks(1);
		}
		else
#endif
#ifdef WITH_DRZ80
		if (z80_core == Z80_CORE_DRZ80) {
			int rem = DrZ80Run(&drz80, cycles);

			// drz80.cycles is the number of cycles remaining,
			// so it must be either 0 or a negative value.
			// z80_odo() relies on this.
			// This value is also returned by DrZ80Run().
			assert(drz80.cycles <= 0);
			assert(drz80.cycles == rem);
			odo.z80 += (cycles - rem);
		}
		else
#endif
			odo.z80 += cycles;
	}
#ifdef WITH_DEBUGGER
	if (debug_z80) {
		++debug_z80_instr_count;
		if (debug_z80_check_wps())
			goto cpu_stalled;
		cycles_to_debug -= (odo.z80 - prev_odo);
		if (cycles_to_debug > 0) {
			prev_odo = odo.z80;
			goto debug_next_instruction;
		}
	}
cpu_stalled:
#endif
	z80_st_running = 0;
}

// Synchronize Z80 with M68K, don't execute code if fake is nonzero
void md::z80_sync(int fake)
{
	int cycles = (m68k_odo() >> 1);
#ifdef WITH_DEBUGGER
	int cycles_to_debug = 0;
	int prev_odo = 0;
	bool debug_z80;
#endif

	if (cycles > odo.z80_max)
		cycles = odo.z80_max;
	cycles -= odo.z80;
	if (cycles <= 0)
		return;
	z80_st_running = 1;
#ifdef WITH_DEBUGGER
	if (debug_trap)
		goto cpu_stalled;
	debug_z80 = (debug_step_z80 ||
		     debug_trace_z80 ||
		     debug_instr_count_enabled ||
		     debug_is_z80_bp_set() ||
		     debug_is_z80_wp_set());
	if (debug_z80) {
		prev_odo = odo.z80;
		cycles_to_debug = cycles;
		cycles = 1;
	debug_next_instruction:
		if (debug_z80_check_bps())
			goto cpu_stalled;
	}
#endif
	if (fake)
		odo.z80 += cycles;
	else {
#ifdef WITH_CZ80
		if (z80_core == Z80_CORE_CZ80)
			odo.z80 += Cz80_Exec(&cz80, cycles);
		else
#endif
#ifdef WITH_MZ80
		if (z80_core == Z80_CORE_MZ80) {
			mz80exec(cycles);
			odo.z80 += mz80GetElapsedTicks(1);
		}
		else
#endif
#ifdef WITH_DRZ80
		if (z80_core == Z80_CORE_DRZ80) {
			int rem = DrZ80Run(&drz80, cycles);

			// drz80.cycles is the number of cycles remaining,
			// so it must be either 0 or a negative value.
			// z80_odo() relies on this.
			// This value is also returned by DrZ80Run().
			assert(drz80.cycles <= 0);
			assert(drz80.cycles == rem);
			odo.z80 += (cycles - rem);
		}
		else
#endif
			odo.z80 += cycles;
	}
#ifdef WITH_DEBUGGER
	if (debug_z80) {
		++debug_z80_instr_count;
		if (debug_z80_check_wps())
			goto cpu_stalled;
		cycles_to_debug -= (odo.z80 - prev_odo);
		if (cycles_to_debug > 0) {
			prev_odo = odo.z80;
			goto debug_next_instruction;
		}
	}
cpu_stalled:
#endif
	z80_st_running = 0;
}

// Trigger Z80 IRQ
void md::z80_irq(int vector)
{
#ifdef WITH_DEBUGGER
	// Ignore interrupts while debugger is active or stepping by a
	// small amount (XXX should be configurable).
	if (debug_trap || (debug_step_z80 && debug_step_z80 < 1000))
		return;
#endif
	z80_st_irq = 1;
	z80_irq_vector = vector;
#ifdef WITH_CZ80
	if (z80_core == Z80_CORE_CZ80)
		Cz80_Set_IRQ(&cz80, vector);
	else
#endif
#ifdef WITH_MZ80
	if (z80_core == Z80_CORE_MZ80)
		mz80int(vector);
	else
#endif
#ifdef WITH_DRZ80
	if (z80_core == Z80_CORE_DRZ80) {
		drz80.z80irqvector = vector;
		drz80.Z80_IRQ = 1;
	}
	else
#endif
		(void)0;
}

// Clear Z80 IRQ
void md::z80_irq_clear()
{
	z80_st_irq = 0;
	z80_irq_vector = 0;
#ifdef WITH_CZ80
	if (z80_core == Z80_CORE_CZ80)
		Cz80_Clear_IRQ(&cz80);
	else
#endif
#ifdef WITH_MZ80
	if (z80_core == Z80_CORE_MZ80)
		mz80ClearPendingInterrupt();
	else
#endif
#ifdef WITH_DRZ80
	if (z80_core == Z80_CORE_DRZ80) {
		drz80.z80irqvector = 0x00;
		drz80.Z80_IRQ = 0x00;
	}
	else
#endif
		(void)0;
}

// Return the number of microseconds spent in current frame
unsigned int md::frame_usecs()
{
	if (z80_st_running)
		return ((z80_odo() * 1000) / (clk0 / 1000));
	return ((m68k_odo() * 1000) / (clk1 / 1000));
}

// Return first line of vblank
unsigned int md::vblank()
{
	return (((pal) && (vdp.reg[1] & 0x08)) ? PAL_VBLANK : NTSC_VBLANK);
}

// 6-button pad status update. Should be called once per displayed line.
void md::pad_update()
{
	// The following code was originally in DGen until at least DGen v1.21
	// (Win32 version) but wasn't in DGen/SDL v1.23, preventing 6-button
	// emulation from working at all until now (v1.31 included).
	// This broke some games (no input at all).

	// Reset 6-button pad toggle after 26? lines
	if (aoo3_six_timeout > 25)
		aoo3_six = 0;
	else
		++aoo3_six_timeout;
	if (aoo5_six_timeout > 25)
		aoo5_six = 0;
	else
		++aoo5_six_timeout;
}

// Generate one frame
int md::one_frame(struct bmap *bm, unsigned char retpal[256],
		  struct sndinfo *sndi)
{
	int hints;
	int m68k_max, z80_max;
	unsigned int vblank = md::vblank();

#ifdef WITH_DEBUGGER
	if (debug_trap)
		return 0;
#endif
#ifdef WITH_DEBUG_VDP
	/*
	 * If the user is disabling planes for debugging, then we
	 * paint the screen black before blitting a new frame. This
	 * stops crap from earlier frames from junking up the display.
	 */
	if ((bm != NULL) &&
	    (dgen_vdp_hide_plane_b | dgen_vdp_hide_plane_a |
	     dgen_vdp_hide_plane_w | dgen_vdp_hide_sprites))
		memset(bm->data, 0, (bm->pitch * bm->h));
#endif
	md_set(1);
	// Reset odometers
	memset(&odo, 0, sizeof(odo));
	// Reset FM tickers
	fm_ticker[1] = 0;
	fm_ticker[3] = 0;
	// Raster zero causes special things to happen :)
	// Init status register with fifo always empty (FIXME)
	coo4 = (0x34 | 0x02); // 00110100b | 00000010b
	if (vdp.reg[12] & 0x2)
		coo5 ^= 0x10; // Toggle odd/even for interlace
	coo5 &= ~0x08; // Clear vblank
	coo5 |= !!pal;
	// Clear sprite overflow bit (d6).
	coo5 &= ~0x40;
	// Clear sprite collision bit (d5).
	coo5 &= ~0x20;
	// Is permanently set
	hints = vdp.reg[10]; // Set hint counter
	// Reset sprite overflow line
	vdp.sprite_overflow_line = INT_MIN;
	// Video display! :D
	for (ras = 0; ((unsigned int)ras < vblank); ++ras) {
		pad_update(); // Update 6-button pads
		fm_timer_callback(); // Update sound timers
		if (--hints < 0) {
			// Trigger hint
			hints = vdp.reg[10];
			vdp.hint_pending = true;
			m68k_vdp_irq_trigger();
			may_want_to_get_pic(bm, retpal, 1);
		}
		else
			may_want_to_get_pic(bm, retpal, 0);
		// Enable h-blank
		coo5 |= 0x04;
		// H-blank comes before, about 36/209 of the whole scanline
		m68k_max = (odo.m68k_max + M68K_CYCLES_PER_LINE);
		odo.m68k_max += M68K_CYCLES_HBLANK;
		z80_max = (odo.z80_max + Z80_CYCLES_PER_LINE);
		odo.z80_max += Z80_CYCLES_HBLANK;
		m68k_run();
		z80_run();
		// Disable h-blank
		coo5 &= ~0x04;
		// Do hdisplay now
		odo.m68k_max = m68k_max;
		odo.z80_max = z80_max;
		m68k_run();
		z80_run();
	}
	// Now we're in vblank, more special things happen :)
	// The following was roughly adapted from Genplus GX
	// Enable v-blank
	coo5 |= 0x08;
	if (--hints < 0) {
		// Trigger hint
		vdp.hint_pending = true;
		m68k_vdp_irq_trigger();
	}
	// Save m68k_max and z80_max
	m68k_max = (odo.m68k_max + M68K_CYCLES_PER_LINE);
	z80_max = (odo.z80_max + Z80_CYCLES_PER_LINE);
	// Delay between vint and vint flag
	odo.m68k_max += M68K_CYCLES_HBLANK;
	odo.z80_max += Z80_CYCLES_HBLANK;
	// Enable h-blank
	coo5 |= 0x04;
	m68k_run();
	z80_run();
	// Disable h-blank
	coo5 &= ~0x04;
	// Toggle vint flag
	coo5 |= 0x80;
	// Delay between v-blank and vint
	odo.m68k_max += (M68K_CYCLES_VDELAY - M68K_CYCLES_HBLANK);
	odo.z80_max += (Z80_CYCLES_VDELAY - Z80_CYCLES_HBLANK);
	m68k_run();
	z80_run();
	// Restore m68k_max and z80_max
	odo.m68k_max = m68k_max;
	odo.z80_max = z80_max;
	// Blank everything and trigger vint
	vdp.vint_pending = true;
	m68k_vdp_irq_trigger();
	if (!z80_st_reset)
		z80_irq(0);
	fm_timer_callback();
	// Run remaining cycles
	m68k_run();
	z80_run();
	++ras;
	// Run the course of vblank
	pad_update();
	fm_timer_callback();
	// Usual h-blank stuff
	coo5 |= 0x04;
	m68k_max = (odo.m68k_max + M68K_CYCLES_PER_LINE);
	odo.m68k_max += M68K_CYCLES_HBLANK;
	z80_max = (odo.z80_max + Z80_CYCLES_PER_LINE);
	odo.z80_max += Z80_CYCLES_HBLANK;
	m68k_run();
	z80_run();
	coo5 &= ~0x04;
	odo.m68k_max = m68k_max;
	odo.z80_max = z80_max;
	m68k_run();
	z80_run();
	// Clear Z80 interrupt
	if (z80_st_irq)
		z80_irq_clear();
	++ras;
	// Remaining lines
	while ((unsigned int)ras < lines) {
		pad_update();
		fm_timer_callback();
		// Enable h-blank
		coo5 |= 0x04;
		m68k_max = (odo.m68k_max + M68K_CYCLES_PER_LINE);
		odo.m68k_max += M68K_CYCLES_HBLANK;
		z80_max = (odo.z80_max + Z80_CYCLES_PER_LINE);
		odo.z80_max += Z80_CYCLES_HBLANK;
		m68k_run();
		z80_run();
		// Disable h-blank
		coo5 &= ~0x04;
		odo.m68k_max = m68k_max;
		odo.z80_max = z80_max;
		m68k_run();
		z80_run();
		++ras;
	}
	// Fill the sound buffers
	if (sndi)
		may_want_to_get_sound(sndi);
	fm_timer_callback();
	md_set(0);
#ifdef WITH_VGMDUMP
	vgm_dump_frame();
#endif
	return 0;
}

// Return V counter (Gens/GS style)
uint8_t md::calculate_coo8()
{
	unsigned int id;
	unsigned int hc, vc;
	uint8_t bl, bh;

	id = m68k_odo();
	/*
	  FIXME
	  Using "(ras - 1)" instead of "ras" here seems to solve horizon
	  issues in Road Rash and Mickey Mania (Moose Chase level).
	*/
	if (ras)
		id -= ((ras - 1) * M68K_CYCLES_PER_LINE);
	id &= 0x1ff;
	if (vdp.reg[4] & 0x81) {
		hc = hc_table[id][1];
		bl = 0xa4;
	}
	else {
		hc = hc_table[id][0];
		bl = 0x84;
	}
	bh = (hc <= 0xe0);
	bl = (hc >= bl);
	bl &= bh;
	vc = ras;
	vc += (bl != 0);
	if (pal) {
		if (vc >= 0x103)
			vc -= 56;
	}
	else {
		if (vc >= 0xeb)
			vc -= 6;
	}
	return vc;
}

// Return H counter (Gens/GS style)
uint8_t md::calculate_coo9()
{
	unsigned int id;

	id = m68k_odo();
	if (ras)
		id -= ((ras - 1) * M68K_CYCLES_PER_LINE);
	id &= 0x1ff;
	if (vdp.reg[4] & 0x81)
		return hc_table[id][1];
	return hc_table[id][0];
}

// *************************************
//       May want to get pic or sound
// *************************************

inline int md::may_want_to_get_pic(struct bmap *bm,unsigned char retpal[256],int/*mark*/)
{
  if (bm==NULL) return 0;

  if (ras>=0 && (unsigned int)ras<vblank())
    vdp.draw_scanline(bm, ras);
  if(retpal && ras == 100) get_md_palette(retpal, vdp.cram);
  return 0;
}

int md::may_want_to_get_sound(struct sndinfo *sndi)
{
  extern intptr_t dgen_volume;
  unsigned int i, len = sndi->len;

  // Get the PSG
  SN76496Update_16_2(0, sndi->lr, len);

	if (dac_len) {
		unsigned int ratio = ((sndi->len << 10) / elemof(dac_data));

		// Stretch the DAC to fit the real length.
		for (i = 0; (i != sndi->len); ++i) {
			unsigned int index = ((i << 10) / ratio);
			uint16_t data;

			if (index >= dac_len)
				data = dac_data[dac_len - 1];
			else
				data = dac_data[index];
			data = ((data - 0x80) << 6);
			sndi->lr[i << 1] += data;
			sndi->lr[(i << 1) ^ 1] += data;
		}
		// Clear the DAC for next frame.
		dac_len = 0;
#ifndef NDEBUG
		memset(dac_data, 0xff, sizeof(dac_data));
#endif
	}

  // Add in the stereo FM buffer
  YM2612UpdateOne(0, sndi->lr, len, dgen_volume, 1);
  if (dgen_mjazz) {
    YM2612UpdateOne(1, sndi->lr, len, dgen_volume, 0);
    YM2612UpdateOne(2, sndi->lr, len, dgen_volume, 0);
  }
  return 0;
}

