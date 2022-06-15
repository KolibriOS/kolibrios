// DGen/SDL v1.29+
// Megadrive C++ module - misc memory

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "md.h"
#include "mem.h"

/**
 * Read one byte from the memory space.
 * @param a Address to read
 * @return byte or 0 if invalid.
 */
uint8_t md::z80_read(uint16_t a)
{
	/* 0x0000-0x3fff: Z80 RAM */
	if (a <= Z80_RAM_END)
		return z80ram[(a & 0x1fff)];
	/* 0x4000-0x5fff: YM2612 */
	if (a <= YM2612_RAM_END)
		return myfm_read(a);
	/* 0x6000-0x6fff: bank register */
	if (a <= BANK_RAM_END)
		return 0; /* invalid address */
	/* 0x7000-0x7fff: PSG/VDP */
	if (a <= PSGVDP_RAM_END)
		return 0; /* invalid address */
	/* 0x8000-0xffff: M68K bank */
	return misc_readbyte(z80_bank68k + (a & 0x7fff));
}

/**
 * Write one byte to the memory
 * @param a Address to read
 * @param d Data (byte) to write.
 */
void md::z80_write(uint16_t a, uint8_t d)
{
	/* 0x0000-0x3fff: Z80 RAM */
	if (a <= Z80_RAM_END) {
		z80ram[(a & 0x1fff)] = d;
		return;
	}
	/* 0x4000-0x5fff: YM2612 */
	if (a <= YM2612_RAM_END) {
		myfm_write(a, d, 1);
		return;
	}
	/* 0x6000-0x6fff: bank register */
	if (a <= BANK_RAM_END) {
		uint32_t tmp;

		if (a > 0x60ff)
			return; /* invalid address */
		tmp = (z80_bank68k >> 1);
		tmp |= ((d & 1) << 23);
		z80_bank68k = (tmp & 0xff8000);
		return;
	}
	/* 0x7000-0x7fff: PSG */
	if (a <= PSGVDP_RAM_END) {
		if (a == 0x7f11) {
			mysn_write(d);
			return;
		}
		return; /* invalid address */
	}
	/* 0x8000-0xffff: M68K bank */
	misc_writebyte((z80_bank68k + (a & 0x7fff)), d);
}

/**
 * Port read to Z80.
 * This is a NOP
 * @param a address (ignored)
 * @return always returns 0xff
 */
uint8_t md::z80_port_read(uint16_t a)
{
	(void)a;
	return 0xff;
}

/**
 * Port write to z80
 * This is a Nop
 * @param a address (ignored)
 * @param d data (ignored)
 */
void md::z80_port_write(uint16_t a, uint8_t d)
{
	(void)a;
	(void)d;
}

uint8_t md::m68k_ROM_read(uint32_t a)
{
	/* save RAM */
	if ((save_active) && (save_len) &&
	    (a >= save_start) && ((a - save_start) < save_len))
		return saveram[((a ^ 1) - save_start)];
	/* ROM */
	if (ROM_ADDR(a) < romlen)
		return rom[ROM_ADDR(a)];
	/* empty area */
	return 0;
}

uint8_t md::m68k_IO_read(uint32_t a)
{
	/* Z80 */
	if (a < 0xa10000) {
		if ((!z80_st_busreq) && (a < 0xa04000))
			return 0;
		return z80_read(a & 0xffff);
	}
	/* version */
	if (a == 0xa10000)
		return 0;
	if (a == 0xa10001) {
		uint8_t c = region;

		/* If region hasn't been defined, guess it. */
		if (c == '\0')
			c = region_guess();
		region_info(c, 0, 0, 0, 0, &c);
		/* Remove PAL flag if we're not in PAL mode. */
		if (!pal)
			c &= ~0x40;
		return c;
	}
	/* data 1 (pad 0) */
	if (a == 0xa10002)
		return 0;
	if (a == 0xa10003) {
		if (aoo3_six == 3) {
			/* extended pad info */
			if (aoo3_toggle == 0)
				return (((pad[0] >> 8) & 0x30) + 0x00);
			return ((pad[0] & 0x30) + 0x40 +
				((pad[0] >> 16) & 0x0f));
		}
		if (aoo3_toggle == 0) {
			if (aoo3_six == 4)
				return (((pad[0] >> 8) & 0x30) +
					0x00 + 0x0f);
			return (((pad[0] >> 8) & 0x30) +
				0x00 + (pad[0] & 0x03));
		}
		return ((pad[0] & 0x30) + 0x40 + (pad[0] & 0x0f));
	}
	/* data 2 (pad 1) */
	if (a == 0xa10004)
		return 0;
	if (a == 0xa10005) {
		if (aoo5_six == 3) {
			/* extended pad info */
			if (aoo5_toggle == 0)
				return (((pad[1] >> 8) & 0x30) + 0x00);
			return ((pad[1] & 0x30) + 0x40 +
				((pad[1] >> 16) & 0x0f));
		}
		if (aoo5_toggle == 0) {
			if (aoo5_six == 4)
				return (((pad[1] >> 8) & 0x30) +
					0x00 + 0x0f);
			return (((pad[1] >> 8) & 0x30) +
				0x00 + (pad[1] & 0x03));
		}
		return ((pad[1] & 0x30) + 0x40 + (pad[1] & 0x0f));
	}
	/* data 3 (exp) */
	if (a == 0xa10006)
		return 0;
	if (a == 0xa10007)
		return 0xff;
	/* ctrl 1 */
	if (a == 0xa10008)
		return 0;
	if (a == 0xa10009)
		return pad_com[0];
	/* ctrl 2 */
	if (a == 0xa1000a)
		return 0;
	if (a == 0xa1000b)
		return pad_com[1];
	/* ctrl 3 */
	if ((a & 0xa1000c) == 0xa1000c)
		return 0;
	/* memory mode */
	if ((a & 0xfffffe) == 0xa11000)
		return 0xff;
	/* Z80 BUSREQ */
	if ((a & 0xffff01) == 0xa11100)
		return (!z80_st_busreq | ((m68k_read_pc() >> 8) & 0xfe));
	if ((a & 0xffff01) == 0xa11101)
		return (m68k_read_pc() & 0xff);
	/* Z80 RESET */
	if ((a & 0xffff01) == 0xa11200)
		return (m68k_read_pc() >> 8);
	if ((a & 0xffff01) == 0xa11201)
		return (m68k_read_pc() & 0xff);
	return 0; /* invalid address */
}

uint8_t md::m68k_VDP_read(uint32_t a)
{
	a &= 0xe700ff;
	/* data */
	if (a < 0xc00004) {
		if (a & 0x01)
			return 0;
		vdp.cmd_pending = false;
		return vdp.readbyte();
	}
	/* control */
	if (a < 0xc00008) {
		vdp.cmd_pending = false;
		if ((a & 0x01) == 0)
			return coo4;
		return coo5;
	}
	/* HV counters */
	if (a == 0xc00008)
		return calculate_coo8();
	if (a == 0xc00009)
		return calculate_coo9();
	/* PSG */
	if (a == 0xc00011)
		return (0);
	return 0; /* invalid address */
}

/**
 * Read a byte from the m68Ks ram.
 * @param a Address to read.
 */
uint8_t md::misc_readbyte(uint32_t a)
{
	/* clip to 24-bit */
	a &= 0x00ffffff;
	/* 0x000000-0x7fffff: ROM */
	if (a <= M68K_ROM_END) {
		return m68k_ROM_read(a);
	}
#ifdef WITH_PICO
	/* 0x800000-0x80001f: Sega Pico I/O area */
	if ((pico_enabled) &&
	    ((a >= 0x800000) && (a <= 0x80001f))) {
		a &= 0x1f;
		switch(a) {
		case 1: // Version register
			switch (region) {
			case 'J': // Japan
				return 0;
			case 'E': // Europe
				return 32;
			case 'U': // USA
				return 64;
			}
			return 0;
		case 3: // Pico pad
			return pad[0];
		case 5: // MSB of X coordinate for pen
			return pico_pen_coords[0] >> 8;
		case 7: // LSB of X coordinate for pen
			return pico_pen_coords[0] & 0xff;
		case 9: // MSB of Y coordinate for pen
			return pico_pen_coords[1] >> 8;
		case 0xB: // LSB of Y coordinate for pen
			return pico_pen_coords[1] & 0xff;
		}
	}
	/* 0x800020-0xafffff: Sega Pico empty area */
	if ((pico_enabled) &&
	    (a <= M68K_IO_END))
		return 0;
#endif
	/* 0x800000-0x9fffff: empty area */
	if (a <= M68K_EMPTY1_END) {
		/*
		 * http://cgfm2.emuviews.com/txt/gen-hw.txt
		 * see section 1 point 3 for what these addresses do.
		 */
		return 0;
	}
	/* 0xa00000-0xafffff: system I/O and control */
	if (a <= M68K_IO_END) {
		return m68k_IO_read(a);
	}
	/* 0xb00000-0xbfffff: empty area */
	if (a <= M68K_EMPTY2_END)
		return 0;
	/* 0xc00000-0xdfffff: VDP/PSG */
	if (a <= M68K_VDP_END) {
		return m68k_VDP_read(a);
	}
	/* 0xe00000-0xfeffff: invalid addresses, mirror RAM */
	/* 0xff0000-0xffffff: RAM */
	return ram[((a ^ 1) & 0xffff)];
}

void md::m68k_ROM_write(uint32_t a, uint8_t d)
{
	/* save RAM */
	if ((!save_prot) && (save_len) &&
	    (a >= save_start) && ((a - save_start) < save_len))
		saveram[((a ^ 1) - save_start)] = d;
#ifdef WITH_DEBUGGER
	/* Allow debugger to write to the ROM. */
	if ((debug_trap) && (ROM_ADDR(a) < romlen))
		rom[ROM_ADDR(a)] = d;
#endif
}

void md::m68k_IO_write(uint32_t a, uint8_t d)
{
	/* Z80 */
	if (a < 0xa10000) {
		if ((!z80_st_busreq) && (a < 0xa04000))
			return;
		z80_write((a & 0xffff), d);
		return;
	}
	if (a == 0xa11100) {
		/* Z80 BUSREQ */
		if (d & 0x01)
			m68k_busreq_request();
		else
			m68k_busreq_cancel();
		return;
	}
	if (a == 0xa11101)
		return;
	/* Z80 RESET */
	if (a == 0xa11200) {
		/* cancel RESET state if nonzero */
		if (d)
			z80_st_reset = 0;
		else if (z80_st_reset == 0) {
			if (z80_st_busreq == 0)
				z80_sync(0);
			z80_st_reset = 1;
			z80_reset();
			fm_reset();
		}
		return;
	}
	if (a == 0xa11201)
		return;
	/* I/O port access */
	if (a < 0xa1000d) {
		if (a == 0xa10003) {
			if ((aoo3_six >= 0) && ((d & 0x40) == 0) &&
			    (aoo3_toggle))
				++aoo3_six;
			if (aoo3_six > 0xc00000)
				aoo3_six &= ~0x400000;
			/* keep it circling around a high value */
			if (d & 0x40)
				aoo3_toggle = 1;
			else
				aoo3_toggle = 0;
			aoo3_six_timeout = 0;
			return;
		}
		if (a == 0xa10005) {
			if ((aoo5_six >= 0) && ((d & 0x40) == 0) &&
			    (aoo5_toggle))
				++aoo5_six;
			if (aoo5_six > 0xc00000)
				aoo5_six &= ~0x400000;
			/* keep it circling around a high value */
			if (d & 0x40)
				aoo5_toggle = 1;
			else
				aoo5_toggle = 0;
			aoo5_six_timeout = 0;
			return;
		}
		return;
	}
	/* save RAM status */
	if (a == 0xa130f1) {
		/*
		  Bit 0: 0 = ROM active, 1 = SRAM active
		  Bit 1: 0 = writable protect
		*/
		save_active = (d & 1);
		save_prot = (d & 2);
		return;
	}
	return;
}

/**
 * write a byte to the m68Ks ram.
 * @param a Address to write.
 * @param d Date (byte) two write.
 */
void md::misc_writebyte(uint32_t a, uint8_t d)
{
	/* clip to 24-bit */
	a &= 0x00ffffff;
	/* 0x000000-0x7fffff: ROM */
	if (a <= M68K_ROM_END) {
		m68k_ROM_write(a, d);
		return;
	}
	/* 0x800000-0x9fffff: empty area */
	if (a <= M68K_EMPTY1_END)
		return;
	/* 0xa00000-0xafffff: system I/O and control */
	if (a <= M68K_IO_END) {
		m68k_IO_write(a, d);
		return;
	}
	/* 0xb00000-0xbfffff: empty area */
	if (a <= M68K_EMPTY2_END)
		return;
	/* 0xc00000-0xdfffff: VDP/PSG */
	if (a < M68K_VDP_END) {
		a &= 0xe700ff;
		if (a < 0xc00008) {
			misc_writeword(a, (d | (d << 8)));
			return;
		}
		/* PSG */
		if (a == 0xc00011)
			mysn_write(d);
		return;
	}
	/* 0xe00000-0xfeffff: invalid addresses, mirror RAM */
	/* 0xff0000-0xffffff: RAM */
	ram[((a ^ 1) & 0xffff)] = d;
}


/**
 * Read a word from the m68k memory.
 * There are quirks with word wide reads see section 1.2 of
 * http://cgfm2.emuviews.com/txt/gen-hw.txt
 * @param a Address to read
 * @return word from memory.
 */
uint16_t md::misc_readword(uint32_t a)
{
	uint16_t ret;

	a &= 0x00ffffff;
	/* BUSREQ */
	if ((a & 0xffff00) == 0xa11100)
		return ((!z80_st_busreq << 8) | (m68k_read_pc() & 0xfeff));
	/* RESET */
	if ((a & 0xffff00) == 0xa11200)
		return m68k_read_pc();
	/* VDP */
	if ((a >= 0xc00000) && (a < 0xe00000)) {
		a &= 0xe700ff;
		if (a < 0xc00004) {
			if (a & 0x01)
				return 0;
			vdp.cmd_pending = false;
			return vdp.readword();
		}
		if (a < 0xc00008) {
			if (a & 0x01)
				return 0;
			return (((coo4 & 0xff) << 8) | (coo5 & 0xff));
		}
		if (a == 0xc00008) {
			if (a & 0x01)
				return 0;
			return ((calculate_coo8() << 8) |
				(calculate_coo9() & 0xff));
		}
	}
	/* else pass onto readbyte */
	ret = (misc_readbyte(a) << 8);
	ret |= misc_readbyte(a + 1);
	return ret;
}

/**
 * Write a word to m68k memory
 * @param a Address to write to.
 * @param d Data to write.
 */
void md::misc_writeword(uint32_t a, uint16_t d)
{
	a &= 0x00ffffff;
	/* Z80 */
	if ((a >= 0xa00000) && (a < 0xa10000)) {
		if ((!z80_st_busreq) && (a < 0xa04000))
			return;
		z80_write((a & 0xffff), (d >> 8));
		return;
	}
	/* BUSREQ and RESET */
	if ((a == 0xa11100) ||
	    (a == 0xa11200)) {
		misc_writebyte(a, (d >> 8));
		return;
	}
	/* VDP */
	if ((a >= 0xc00000) && (a < 0xe00000)) {
		a &= 0xe700ff;
		if (a < 0xc00004) {
			if (a & 0x01)
				return;
			vdp.writeword(d);
			vdp.cmd_pending = false;
			return;
		}
		if (a < 0xc00008) {
			if (a & 0x01)
				return;
			/* second half of a command */
			if (vdp.cmd_pending) {
				vdp.command(d);
				return;
			}
			/* register write */
			if ((d & 0xc000) == 0x8000) {
				uint8_t addr = ((d >> 8) & 0x1f);
				vdp.write_reg(addr, d);
				return;
			}
			/* first half of a command */
			vdp.command(d);
			vdp.cmd_pending = true;
			return;
		}
	}
	/* else pass onto writebyte */
	misc_writebyte(a, (d >> 8));
	misc_writebyte((a + 1), (d & 0xff));
}

#ifdef WITH_MUSA

// read/write functions called by the CPU to access memory.
// while values used are 32 bits, only the appropriate number
// of bits are relevant (i.e. in write_memory_8, only the lower 8 bits
// of value should be written to memory).
// address will be a 24-bit value.

/* Read from anywhere */
extern "C" unsigned int m68k_read_memory_8(unsigned int address)
{
	return md::md_musa->misc_readbyte(address);
}

extern "C" unsigned int m68k_read_memory_16(unsigned int address)
{
	return md::md_musa->misc_readword(address);
}

extern "C" unsigned int m68k_read_memory_32(unsigned int address)
{
	return ((md::md_musa->misc_readword(address) << 16) |
		(md::md_musa->misc_readword(address + 2) & 0xffff));
}

/* Read data immediately following the PC */
extern "C" unsigned int m68k_read_immediate_8(unsigned int address)
{
	return m68k_read_memory_8(address);
}

extern "C" unsigned int m68k_read_immediate_16(unsigned int address)
{
	return m68k_read_memory_16(address);
}

extern "C" unsigned int m68k_read_immediate_32(unsigned int address)
{
	return m68k_read_memory_32(address);
}

/* Read an instruction (16-bit word immeditately after PC) */
extern "C" unsigned int m68k_read_instruction(unsigned int address)
{
	return m68k_read_memory_16(address);
}

/* Write to anywhere */
extern "C" void m68k_write_memory_8(unsigned int address, unsigned int value)
{
	md::md_musa->misc_writebyte(address, value);
}

extern "C" void m68k_write_memory_16(unsigned int address, unsigned int value)
{
	md::md_musa->misc_writeword(address, value);
}

extern "C" void m68k_write_memory_32(unsigned int address, unsigned int value)
{
	md::md_musa->misc_writeword(address, ((value >> 16) & 0xffff));
	md::md_musa->misc_writeword((address + 2), (value & 0xffff));
}

#endif // WITH_MUSA

#ifdef WITH_CYCLONE

/* Read from anywhere */
extern "C" uint32_t cyclone_read_memory_8(uint32_t address)
{
	return md::md_cyclone->misc_readbyte(address);
}

extern "C" uint32_t cyclone_read_memory_16(uint32_t address)
{
	return md::md_cyclone->misc_readword(address);
}

extern "C" uint32_t cyclone_read_memory_32(uint32_t address)
{
	return ((md::md_cyclone->misc_readword(address) << 16) |
		(md::md_cyclone->misc_readword(address + 2) & 0xffff));
}

/* Write to anywhere */
extern "C" void cyclone_write_memory_8(uint32_t address, uint8_t value)
{
	md::md_cyclone->misc_writebyte(address, value);
}

extern "C" void cyclone_write_memory_16(uint32_t address, uint16_t value)
{
	md::md_cyclone->misc_writeword(address, value);
}

extern "C" void cyclone_write_memory_32(uint32_t address, uint32_t value)
{
	md::md_cyclone->misc_writeword(address, ((value >> 16) & 0xffff));
	md::md_cyclone->misc_writeword((address + 2), (value & 0xffff));
}

uintptr_t md::checkpc(uintptr_t pc)
{
	static uint8_t zero[(M68K_EMPTY1_END + 1)];

	pc -= cyclonecpu.membase; // Get the real program counter.

	pc &= 0x00ffffff; // Clip to 24-bit.
	if ((save_active) && (save_len) &&
	    (pc >= save_start) && ((pc - save_start) < save_len)) {
		cyclonecpu.membase = (uintptr_t)saveram;
		pc -= save_start;
	}
	if (pc <= romlen)
		cyclonecpu.membase = (uintptr_t)rom; // Jump to ROM.
	else if (pc <= M68K_EMPTY1_END)
		cyclonecpu.membase = (uintptr_t)zero; // Scratch area.
	else if (pc >= 0xe00000) {
		pc &= 0xffff;
		cyclonecpu.membase = (uintptr_t)ram; // Jump to RAM.
	}
	else {
		DEBUG(("PC out of bounds: %06x", pc));
		// Freeze, avoid crashing the emulator.
		cyclonecpu.membase = (uintptr_t)no_rom;
		pc = 0;
	}
	return (cyclonecpu.membase + pc); // New program counter.
}

extern "C" uintptr_t cyclone_checkpc(uintptr_t pc)
{
	return md::md_cyclone->checkpc(pc);
}

#endif // WITH_CYCLONE

#ifdef WITH_STAR

extern "C" unsigned star_readbyte(unsigned a, unsigned d)
{
	(void)d;
	return md::md_star->misc_readbyte(a);
}

extern "C" unsigned star_readword(unsigned a, unsigned d)
{
	(void)d;
	return md::md_star->misc_readword(a);
}

extern "C" unsigned star_writebyte(unsigned a, unsigned d)
{
	md::md_star->misc_writebyte(a, d);
	return 0;
}

extern "C" unsigned star_writeword(unsigned a, unsigned d)
{
	md::md_star->misc_writeword(a, d);
	return 0;
}

#endif // WITH_STAR

#ifdef WITH_MZ80

/*
  In case the assembly version of MZ80 is used (WITH_X86_MZ80), prevent
  GCC from optimizing sibling calls (-foptimize-sibling-calls, enabled
  by default at -O2 and above). The ASM code doesn't expect this and
  crashes otherwise.
*/

#ifdef WITH_X86_MZ80
#define MZ80_NOSIBCALL(t, w) *((volatile t *)&(w))
#else
#define MZ80_NOSIBCALL(t, w) (w)
#endif

extern "C" UINT8 mz80_read(UINT32 a, struct MemoryReadByte *unused)
{
	(void)unused;
	return md::md_mz80->z80_read(MZ80_NOSIBCALL(UINT32, a));
}

extern "C" void mz80_write(UINT32 a, UINT8 d, struct MemoryWriteByte *unused)
{
	(void)unused;
	md::md_mz80->z80_write(MZ80_NOSIBCALL(UINT32, a), d);
}

extern "C" UINT16 mz80_ioread(UINT16 a, struct z80PortRead *unused)
{
	(void)unused;
	return md::md_mz80->z80_port_read(MZ80_NOSIBCALL(UINT16, a));
}

extern "C" void mz80_iowrite(UINT16 a, UINT8 d, struct z80PortWrite *unused)
{
	(void)unused;
	return md::md_mz80->z80_port_write(MZ80_NOSIBCALL(UINT16, a), d);
}

#endif // WITH_MZ80

#ifdef WITH_CZ80

extern "C" uint8_t cz80_memread(void *ctx, uint16_t a)
{
	class md* md = (class md*)ctx;

	return md->z80_read(a);
}

extern "C" void cz80_memwrite(void *ctx, uint16_t a, uint8_t d)
{
	class md* md = (class md*)ctx;

	md->z80_write(a, d);
}

extern "C" uint16_t cz80_memread16(void *ctx, uint16_t a)
{
	class md* md = (class md*)ctx;

	return ((uint16_t)md->z80_read(a) |
		((uint16_t)md->z80_read(a + 1) << 8));
}

extern "C" void cz80_memwrite16(void *ctx, uint16_t a, uint16_t d)
{
	class md* md = (class md*)ctx;

	md->z80_write(a, (uint8_t)d);
	md->z80_write((a + 1), (uint8_t)(d >> 8));
}

extern "C" uint8_t cz80_ioread(void *ctx, uint16_t a)
{
	class md* md = (class md*)ctx;

	return md->z80_port_read(a);
}

extern "C" void cz80_iowrite(void *ctx, uint16_t a, uint8_t d)
{
	class md* md = (class md*)ctx;

	md->z80_port_write(a, d);
}

#endif // WITH_CZ80

#ifdef WITH_DRZ80

uintptr_t md::drz80_rebase_pc(uint16_t address)
{
	// PC in memory - rebase PC into memory.
	drz80.Z80PC_BASE = (uintptr_t)z80ram;
	return (drz80.Z80PC_BASE + address);
}

uintptr_t md::drz80_rebase_sp(uint16_t address)
{
	// SP in memory - rebase SP into memory.
	drz80.Z80SP_BASE = (uintptr_t)z80ram;
	return (drz80.Z80SP_BASE + address);
}

uintptr_t drz80_rebaseSP(uint16_t new_sp)
{
	return md::md_drz80->drz80_rebase_sp(new_sp);
}

uintptr_t drz80_rebasePC(uint16_t new_pc)
{
	return md::md_drz80->drz80_rebase_pc(new_pc);
}

uint8_t drz80_read8(uint16_t a)
{
	return md::md_drz80->z80_read(a);
}

uint16_t drz80_read16(uint16_t a)
{
	return ((uint16_t)md::md_drz80->z80_read(a) |
		((uint16_t)md::md_drz80->z80_read(a + 1) << 8));
}

void drz80_write8(uint8_t d, uint16_t a)
{
	md::md_drz80->z80_write(a, d);
}

void drz80_write16(uint16_t d, uint16_t a)
{
	md::md_drz80->z80_write(a, (uint8_t)d);
	md::md_drz80->z80_write((a + 1), (uint8_t)(d >> 8));
}

uint8_t drz80_in(uint16_t p)
{
	(void)p;
	return 0xff;
}

void drz80_out(uint16_t p, uint8_t d)
{
	(void)p;
	(void)d;
}

#endif // WITH_DRZ80
