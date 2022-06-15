// DGen v1.10+
// Megadrive C++ module saving and loading

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "md.h"
#include "system.h"

void md::m68k_state_dump()
{
	/*
	  32 and 16-bit values must be stored LSB first even though M68K is
	  big-endian for compatibility with other emulators.
	*/
	switch (cpu_emu) {
#ifdef WITH_MUSA
	case CPU_EMU_MUSA:
		if (md_set_musa(true))
			md_set_musa_sync(false);
		md_set_musa(false);
		break;
#endif
#ifdef WITH_STAR
	case CPU_EMU_STAR:
		if (md_set_star(true))
			md_set_star_sync(false);
		md_set_star(false);
		break;
#endif
#ifdef WITH_CYCLONE
	case CPU_EMU_CYCLONE:
		if (md_set_cyclone(true))
			md_set_cyclone_sync(false);
		md_set_cyclone(false);
		break;
#endif
	default:
		break;
	}
}

void md::m68k_state_restore()
{
	/* 32 and 16-bit values are stored LSB first. */
	switch (cpu_emu) {
#ifdef WITH_MUSA
	case CPU_EMU_MUSA:
		if (md_set_musa(true))
			md_set_musa_sync(true);
		md_set_musa(false);
		break;
#endif
#ifdef WITH_STAR
	case CPU_EMU_STAR:
		if (md_set_star(true))
			md_set_star_sync(true);
		md_set_star(false);
		break;
#endif
#ifdef WITH_CYCLONE
	case CPU_EMU_CYCLONE:
		if (md_set_cyclone(true))
			md_set_cyclone_sync(true);
		md_set_cyclone(false);
		break;
#endif
	default:
		break;
	}
}

void md::z80_state_dump()
{
	/* 16-bit values must be stored LSB first. */
	z80_state.irq_asserted = z80_st_irq;
	z80_state.irq_vector = z80_irq_vector;
	switch (z80_core) {
#ifdef WITH_CZ80
	case Z80_CORE_CZ80:
		if (md_set_cz80(true))
			md_set_cz80_sync(false);
		md_set_cz80(false);
		break;
#endif
#ifdef WITH_MZ80
	case Z80_CORE_MZ80:
		if (md_set_mz80(true))
			md_set_mz80_sync(false);
		md_set_mz80(false);
		break;
#endif
#ifdef WITH_DRZ80
	case Z80_CORE_DRZ80:
		if (md_set_drz80(true))
			md_set_drz80_sync(false);
		md_set_drz80(false);
		break;
#endif
	default:
		break;
	}
}

void md::z80_state_restore()
{
	/* 16-bit values are stored LSB first. */
	switch (z80_core) {
#ifdef WITH_CZ80
	case Z80_CORE_CZ80:
		if (md_set_cz80(true))
			md_set_cz80_sync(true);
		md_set_cz80(false);
		break;
#endif
#ifdef WITH_MZ80
	case Z80_CORE_MZ80:
		if (md_set_mz80(true))
			md_set_mz80_sync(true);
		md_set_mz80(false);
		break;
#endif
#ifdef WITH_DRZ80
	case Z80_CORE_DRZ80:
		if (md_set_drz80(true))
			md_set_drz80_sync(true);
		md_set_drz80(false);
		break;
#endif
	default:
		break;
	}
	if (z80_state.irq_asserted)
		z80_irq(z80_state.irq_vector);
	else
		z80_irq_clear();
}

/*
gs0 genecyst save file INfo

GST\0 to start with

80-9f = d0-d7 almost certain
a0-bf = a0-a7 almost certain
c8    = pc    fairly certain
d0    = sr    fairly certain


112 Start of cram len 0x80
192 Start of vsram len 0x50
1e2-474 UNKNOWN sound info?
Start of z80 ram at 474 (they store 2000)
Start of RAM at 2478 almost certain (BYTE SWAPPED)
Start of VRAM at 12478
end of VRAM
*/

/*
  2011-11-05 - Adapted from Gens' genecyst_save_file_format.txt:

  Range        Size   Description
  -----------  -----  -----------
  00000-00002  3      "GST"
  00003-00004  2      "\x40\xe0" (Gens and new DGen format)
  00006-00007  2      "\xe0\x40" (old DGen format)
  00040-00043  4      Last VDP control data written
  00044-00044  1      Second write flag
  00045-00045  1      DMA fill flag
  00048-0004B  4      VDP write address
  00050-00050  1      Version
  00051-00051  1      Emulator ID
  00052-00052  1      System ID
  00060-0006F  16     PSG registers
  00080-000D9  90     M68K registers
  000FA-00111  24     VDP registers
  00112-00191  128    Color RAM
  00192-001E1  80     Vertical scroll RAM
  001E2-001E2  1      YM2612 part I address register (DGen)
  001E3-001E3  1      YM2612 part II address register (DGen)
  001E4-003E3  512    YM2612 registers
  00404-00437  52     Z80 registers
  00438-0043F  8      Z80 state
  00474-02473  8192   Z80 RAM
  02478-12477  65536  68K RAM
  12478-22477  65536  Video RAM

  M68K registers (stored little-endian for compatibility)
  --------------
  00080-0009F : D0-D7
  000A0-000BF : A0-A7
  000C8 : PC
  000D0 : SR
  000D2 : USP
  000D6 : SSP

  Z80 registers (little-endian)
  -------------
  00404 : AF
  00408 : BC
  0040C : DE
  00410 : HL
  00414 : IX
  00418 : IY
  0041C : PC
  00420 : SP
  00424 : AF'
  00428 : BC'
  0042C : DE'
  00430 : HL'

  00434 : I
  00435 : R (DGen)
  00436 : x x x x x IFF1 x IFF2 (Gens v5)
  00437 : IM (DGen)

  Z80 State
  ---------
  00438 : Z80 RESET
  00439 : Z80 BUSREQ
  0043A : Unknown
  0043B : Unknown
  0043C : Z80 BANK (DWORD)

  Gens and Kega ADD
  -----------------
  00040 : last VDP Control data written (DWORD)
  00044 : second write flag (1 for second write)
  00045 : DMA Fill flag (1 mean next data write will cause a DMA fill)
  00048 : VDP write address (DWORD)

  00050 : Version       (Genecyst=0; Kega=5; Gens=5; DGen=5)
  00051 : Emulator ID   (Genecyst=0; Kega=0; Gens=1; DGen=9)
  00052 : System ID     (Genesis=0; SegaCD=1; 32X=2; SegaCD32X=3)

  00060-0007F : PSG registers (DWORDs, little endian).
*/

static void *swap16cpy(void *dest, const void *src, size_t n)
{
	size_t i;

	for (i = 0; (i != (n & ~1)); ++i)
		((uint8_t *)dest)[(i ^ 1)] = ((uint8_t *)src)[i];
	((uint8_t *)dest)[i] = ((uint8_t *)src)[i];
	return dest;
}

int md::import_gst(FILE *hand)
{
	uint8_t (*buf)[0x22478] =
		(uint8_t (*)[sizeof(*buf)])malloc(sizeof(*buf));
	uint8_t *p;
	uint8_t *q;
	size_t i;
	uint32_t tmp;

	if ((buf == NULL) ||
	    (fread((*buf), sizeof(*buf), 1, hand) != 1) ||
	    /* GST header */
	    ((memcmp((*buf), "GST\x0\x0\0\xe0\x40", 8) != 0) &&
	     (memcmp((*buf), "GST\x40\xe0", 5) != 0))) {
		fprintf(stderr,
			"%s: error: invalid save file header.\n",
			__func__);
		free(buf);
		return -1;
	}
	p = &(*buf)[0x50];
	if (p[2] != 0) {
		fprintf(stderr,
			"%s: error: this is not a Genesis/Mega Drive"
			" save file.\n",
			__func__);
		free(buf);
		return -1;
	}
	if (p[1] != 9) {
		fprintf(stderr,
			"%s: warning: save file was probably not generated by"
			" DGen/SDL.\n",
			__func__);
	}
	else if (p[0] != 5)
		fprintf(stderr,
			"%s: warning: unknown save file version.\n",
			__func__);
	/* Reset console first. */
	reset();
	/* FIXME: VDP stuff */
	/* PSG registers (8x16-bit, 16 bytes) */
	SN76496_restore(0, &(*buf)[0x60]);
	/* M68K registers (19x32-bit, 1x16-bit, 90 bytes (padding: 12)) */
	p = &(*buf)[0x80];
	q = &(*buf)[0xa0];
	for (i = 0; (i != 8); ++i, p += 4, q += 4) {
		memcpy(&m68k_state.d[i], p, 4);
		memcpy(&m68k_state.a[i], q, 4);
	}
	memcpy(&m68k_state.pc, &(*buf)[0xc8], 4);
	memcpy(&m68k_state.sr, &(*buf)[0xd0], 2);
	/*
	  FIXME?
	  memcpy(&m68k_state.usp, &(*buf)[0xd2], 4);
	  memcpy(&m68k_state.ssp, &(*buf)[0xd6], 4);
	*/
	m68k_state_restore();
	/* VDP registers (24x8-bit VDP registers, not sizeof(vdp.reg)) */
	memcpy(vdp.reg, &(*buf)[0xfa], 0x18);
	memset(&vdp.reg[0x18], 0, (sizeof(vdp.reg) - 0x18));
	/* CRAM (64x16-bit registers, 128 bytes), swapped */
	swap16cpy(vdp.cram, &(*buf)[0x112], 0x80);
	/* VSRAM (40x16-bit words, 80 bytes), swapped */
	swap16cpy(vdp.vsram, &(*buf)[0x192], 0x50);
	/* YM2612 registers */
	p = &(*buf)[0x1e2];
	fm_sel[0] = p[0];
	fm_sel[1] = p[1];
	p = &(*buf)[0x1e4];
	YM2612_restore(0, p);
	fm_reg[0][0x24] = p[0x24];
	fm_reg[0][0x25] = p[0x25];
	fm_reg[0][0x26] = p[0x26];
	fm_reg[0][0x27] = p[0x27];
	dac_data[0] = p[0x2a];
	dac_enabled = (p[0x2b] >> 7);
	memset(fm_ticker, 0, sizeof(fm_ticker));
	/* Z80 registers (12x16-bit and 4x8-bit, 52 bytes (padding: 24)) */
	p = &(*buf)[0x404];
	for (i = 0; (i != 2); ++i, p = &(*buf)[0x424]) {
		memcpy(&z80_state.alt[i].fa, &p[0x0], 2);
		memcpy(&z80_state.alt[i].cb, &p[0x4], 2);
		memcpy(&z80_state.alt[i].ed, &p[0x8], 2);
		memcpy(&z80_state.alt[i].lh, &p[0xc], 2);
	}
	p = &(*buf)[0x414];
	memcpy(&z80_state.ix, &p[0x0], 2);
	memcpy(&z80_state.iy, &p[0x4], 2);
	memcpy(&z80_state.pc, &p[0x8], 2);
	memcpy(&z80_state.sp, &p[0xc], 2);
	p = &(*buf)[0x434];
	z80_state.i = p[0];
	z80_state.r = p[1];
	z80_state.iff = ((p[2] << 1) | p[2]); /* IFF2 = IFF1 */
	z80_state.im = ((p[3] == 0) ? 1 : p[3]);
	z80_state_restore();
	/* Z80 state (8 bytes) */
	p = &(*buf)[0x438];
	z80_st_reset = !p[0]; /* BUS RESET state */
	if (z80_st_reset) {
		z80_reset();
		fm_reset();
	}
	z80_st_busreq = (p[1] & 1); /* BUSREQ state */
	memcpy(&tmp, &(*buf)[0x43c], 4);
	z80_bank68k = le2h32(tmp);
	/* Z80 RAM (8192 bytes) */
	memcpy(z80ram, &(*buf)[0x474], 0x2000);
	/* RAM (65536 bytes), swapped */
	swap16cpy(ram, &(*buf)[0x2478], 0x10000);
	/* VRAM (65536 bytes) */
	memcpy(vdp.vram, &(*buf)[0x12478], 0x10000);
	/* Mark everything as changed */
	memset(vdp.dirt, 0xff, 0x35);
	free(buf);
	return 0;
}

int md::export_gst(FILE *hand)
{
	uint8_t (*buf)[0x22478] =
		(uint8_t (*)[sizeof(*buf)])calloc(1, sizeof(*buf));
	uint8_t *p;
	uint8_t *q;
	size_t i;
	uint32_t tmp;

	if (buf == NULL)
		return -1;
	/* GST header */
	memcpy((*buf), "GST\x40\xe0", 5);
	/* FIXME: VDP stuff */
	/* Version */
	(*buf)[0x50] = 5;
	/* Emulator ID */
	(*buf)[0x51] = 9;
	/* System ID */
	(*buf)[0x52] = 0;
	/* PSG registers (8x16-bit, 16 bytes) */
	SN76496_dump(0, &(*buf)[0x60]);
	/* M68K registers (19x32-bit, 1x16-bit, 90 bytes (padding: 12)) */
	m68k_state_dump();
	p = &(*buf)[0x80];
	q = &(*buf)[0xa0];
	for (i = 0; (i != 8); ++i, p += 4, q += 4) {
		memcpy(p, &m68k_state.d[i], 4);
		memcpy(q, &m68k_state.a[i], 4);
	}
	memcpy(&(*buf)[0xc8], &m68k_state.pc, 4);
	memcpy(&(*buf)[0xd0], &m68k_state.sr, 2);
	/*
	  FIXME?
	  memcpy(&(*buf)[0xd2], &m68k_state.usp, 4);
	  memcpy(&(*buf)[0xd6], &m68k_state.ssp, 4);
	*/
	/* VDP registers (24x8-bit VDP registers, not sizeof(vdp.reg)) */
	memcpy(&(*buf)[0xfa], vdp.reg, 0x18);
	/* CRAM (64x16-bit registers, 128 bytes), swapped */
	swap16cpy(&(*buf)[0x112], vdp.cram, 0x80);
	/* VSRAM (40x16-bit words, 80 bytes), swapped */
	swap16cpy(&(*buf)[0x192], vdp.vsram, 0x50);
	/* YM2612 registers */
	p = &(*buf)[0x1e2];
	p[0] = fm_sel[0];
	p[1] = fm_sel[1];
	p = &(*buf)[0x1e4];
	YM2612_dump(0, p);
	p[0x24] = fm_reg[0][0x24];
	p[0x25] = fm_reg[0][0x25];
	p[0x26] = fm_reg[0][0x26];
	p[0x27] = fm_reg[0][0x27];
	p[0x2a] = 0xff;
	p[0x2b] = (dac_enabled << 7);
	/* Z80 registers (12x16-bit and 4x8-bit, 52 bytes (padding: 24)) */
	z80_state_dump();
	p = &(*buf)[0x404];
	for (i = 0; (i != 2); ++i, p = &(*buf)[0x424]) {
		memcpy(&p[0x0], &z80_state.alt[i].fa, 2);
		memcpy(&p[0x4], &z80_state.alt[i].cb, 2);
		memcpy(&p[0x8], &z80_state.alt[i].ed, 2);
		memcpy(&p[0xc], &z80_state.alt[i].lh, 2);
	}
	p = &(*buf)[0x414];
	memcpy(&p[0x0], &z80_state.ix, 2);
	memcpy(&p[0x4], &z80_state.iy, 2);
	memcpy(&p[0x8], &z80_state.pc, 2);
	memcpy(&p[0xc], &z80_state.sp, 2);
	p = &(*buf)[0x434];
	p[0] = z80_state.i;
	p[1] = z80_state.r;
	p[2] = ((z80_state.iff >> 1) | z80_state.iff);
	p[3] = z80_state.im;
	/* Z80 state (8 bytes) */
	p = &(*buf)[0x438];
	p[0] = !z80_st_reset;
	p[1] = z80_st_busreq;
	tmp = h2le32(z80_bank68k);
	memcpy(&(*buf)[0x43c], &tmp, 4);
	/* Z80 RAM (8192 bytes) */
	memcpy(&(*buf)[0x474], z80ram, 0x2000);
	/* RAM (65536 bytes), swapped */
	swap16cpy(&(*buf)[0x2478], ram, 0x10000);
	/* VRAM (65536 bytes) */
	memcpy(&(*buf)[0x12478], vdp.vram, 0x10000);
	/* Output */
	i = fwrite((*buf), sizeof(*buf), 1, hand);
	free(buf);
	if (i != 1)
		return -1;
	return 0;
}
