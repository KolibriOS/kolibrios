// DGen/SDL v1.17+
// Megadrive C++ module

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#ifdef HAVE_MEMCPY_H
#include "memcpy.h"
#endif
#include "md.h"
#include "system.h"
#include "romload.h"
#include "rc-vars.h"
#include "debug.h"
#include "decode.h"

extern FILE *debug_log;

#ifdef WITH_STAR
extern "C" unsigned star_readbyte(unsigned a, unsigned d);
extern "C" unsigned star_readword(unsigned a, unsigned d);
extern "C" unsigned star_writebyte(unsigned a, unsigned d);
extern "C" unsigned star_writeword(unsigned a, unsigned d);

/**
 * This sets up an array of memory locations.
 * This method is StarScream specific.
 * @return 0 on success
 */
int md::memory_map()
{
  int i=0,j=0;
  int rommax=romlen;

  if (rommax>0xa00000) rommax=0xa00000;
  if (rommax<0) rommax=0;

// FETCH: Set up 2 or 3 FETCH sections
  i=0;
  if (rommax>0)
  { fetch[i].lowaddr=0x000000; fetch[i].highaddr=rommax-1; fetch[i].offset=(unsigned)rom-0x000000; i++; }
  fetch[i].lowaddr=0xff0000; fetch[i].highaddr=0xffffff; fetch[i].offset=(unsigned)ram-  0xff0000; i++;
// Testing
  fetch[i].lowaddr=0xffff0000; fetch[i].highaddr=0xffffffff; fetch[i].offset=(unsigned)ram-0xffff0000; i++;
// Testing 2
  fetch[i].lowaddr=0xff000000; fetch[i].highaddr=0xff000000+rommax-1; fetch[i].offset=(unsigned)rom-0xff000000; i++;
  fetch[i].lowaddr=fetch[i].highaddr=0xffffffff; fetch[i].offset=0; i++;

  if (debug_log!=NULL)
    fprintf (debug_log,"StarScream memory_map has %d fetch sections\n",i);

  i=0; j=0;

#if 0
// Simple version ***************
  readbyte[i].lowaddr=   readword[i].lowaddr=
  writebyte[j].lowaddr=  writeword[j].lowaddr=   0;
  readbyte[i].highaddr=  readword[i].highaddr=
  writebyte[j].highaddr= writeword[j].highaddr=  0xffffffff;

  readbyte[i].memorycall=(void *)star_readbyte;
  readword[i].memorycall=(void *)star_readword;
  writebyte[j].memorycall=(void *)star_writebyte;
  writeword[j].memorycall=(void *)star_writeword;

  readbyte[i].userdata=  readword[i].userdata=
  writebyte[j].userdata= writeword[j].userdata=  NULL;
  i++; j++;
// Simple version end ***************

#else
// Faster version ***************
// IO: Set up 3/4 read sections, and 2/3 write sections
  if (rommax>0)
  {
// Cartridge save RAM memory
    if(save_len) {
      readbyte[i].lowaddr=    readword[i].lowaddr=
      writebyte[j].lowaddr=   writeword[j].lowaddr=   save_start;
      readbyte[i].highaddr=   readword[i].highaddr=
      writebyte[j].highaddr=  writeword[j].highaddr=  save_start+save_len-1;
      readbyte[i].memorycall = star_readbyte;
      readword[j].memorycall = star_readword;
      writebyte[i].memorycall = star_writebyte;
      writeword[j].memorycall = star_writeword;
      readbyte[i].userdata=   readword[i].userdata=
      writebyte[j].userdata=  writeword[j].userdata=  NULL;
      i++; j++;
    }
// Cartridge ROM memory (read only)
    readbyte[i].lowaddr=   readword[i].lowaddr=   0x000000;
    readbyte[i].highaddr=  readword[i].highaddr=  rommax-1;
    readbyte[i].memorycall=readword[i].memorycall=NULL;
    readbyte[i].userdata=  readword[i].userdata=  rom;
    i++;
// misc memory (e.g. aoo and coo) through star_rw
    readbyte[i].lowaddr=   readword[i].lowaddr=
    writebyte[j].lowaddr=  writeword[j].lowaddr=   rommax;
  }
  else
    readbyte[i].lowaddr=   readword[i].lowaddr=
    writebyte[j].lowaddr=  writeword[j].lowaddr=   0;

  readbyte[i].highaddr=  readword[i].highaddr=
  writebyte[j].highaddr= writeword[j].highaddr=  0xfeffff;

  readbyte[i].memorycall = star_readbyte;
  readword[i].memorycall = star_readword;
  writebyte[j].memorycall = star_writebyte;
  writeword[j].memorycall = star_writeword;

  readbyte[i].userdata=  readword[i].userdata=
  writebyte[j].userdata= writeword[j].userdata=  NULL;
  i++; j++;

// scratch RAM memory
  readbyte[i].lowaddr =   readword[i].lowaddr =
  writebyte[j].lowaddr =  writeword[j].lowaddr =   0xff0000;
  readbyte[i].highaddr=   readword[i].highaddr=
  writebyte[j].highaddr=  writeword[j].highaddr=   0xffffff;
  readbyte[i].memorycall= readword[i].memorycall=
  writebyte[j].memorycall=writeword[j].memorycall= NULL;
  readbyte[i].userdata=  readword[i].userdata =
  writebyte[j].userdata= writeword[j].userdata =   ram;
  i++; j++;
// Faster version end ***************
#endif

// The end
   readbyte[i].lowaddr  =   readword[i].lowaddr  =
  writebyte[j].lowaddr  =  writeword[j].lowaddr  =
   readbyte[i].highaddr =   readword[i].highaddr =
  writebyte[j].highaddr =  writeword[j].highaddr = 0xffffffff;

	readbyte[i].memorycall = 0;
	readword[i].memorycall = 0;
	writebyte[j].memorycall = 0;
	writeword[j].memorycall = 0;
	readbyte[i].userdata = 0;
	readword[i].userdata = 0;
	writebyte[j].userdata = 0;
	writeword[j].userdata = 0;

  i++; j++;

  if (debug_log!=NULL)
    fprintf (debug_log,"StarScream memory_map has %d read sections and %d write sections\n",i,j);

  return 0;
}

void star_irq_callback(void)
{
	assert(md::md_star != NULL);
	md::md_star->m68k_vdp_irq_handler();
}
#endif

#ifdef WITH_MUSA
/**
 * This sets up an array of memory locations for Musashi.
 */
void md::musa_memory_map()
{
	unsigned int rom0_len = romlen;
	unsigned int rom1_sta = 0;
	unsigned int rom1_len = 0;

	m68k_register_memory(NULL, 0);
	if (save_len) {
		DEBUG(("[%06x-%06x] ???? (SAVE)",
		       save_start, (save_start + save_len - 1)));
		if (save_start < romlen) {
			/* Punch a hole through the ROM area. */
			rom0_len = save_start;
			/* Add entry for ROM leftovers, if any. */
			if ((save_start + save_len) < romlen) {
				rom1_sta = (save_start + save_len);
				rom1_len = (romlen - rom1_sta);
			}
		}
	}

#ifdef ROM_BYTESWAP
#define S 1
#else
#define S 0
#endif

	const m68k_mem_t mem[3] = {
		// r, w, x, swab, addr, size, mask, mem
		{ 1, 0, 1, S, 0x000000, rom0_len, 0x7fffff, rom }, // M68K ROM
		{ 1, 1, 1, 1, 0xe00000, 0x200000, 0x00ffff, ram }, // M68K RAM
		{ 1, 0, 1, S, rom1_sta, rom1_len, 0x7fffff, &rom[rom1_sta] }
	};
	unsigned int i;
	unsigned int j = 0;

	for (i = 0; ((i < elemof(mem)) && (j < elemof(musa_memory))); ++i) {
		if (mem[i].size == 0)
			continue;
		DEBUG(("[%06x-%06x] %c%c%c%c (%s)",
		       mem[i].addr,
		       (mem[i].addr + mem[i].size - 1),
		       (mem[i].r ? 'r' : '-'),
		       (mem[i].w ? 'w' : '-'),
		       (mem[i].x ? 'x' : '-'),
		       (mem[i].swab ? 's' : '-'),
		       (mem[i].w ? "RAM" : "ROM")));
		musa_memory[j] = mem[i];
		++j;
	}
	if (j)
		m68k_register_memory(musa_memory, j);
	else
		DEBUG(("no memory region defined"));
}

int musa_irq_callback(int level)
{
	(void)level;
	assert(md::md_musa != NULL);
	md::md_musa->m68k_vdp_irq_handler();
	return M68K_INT_ACK_AUTOVECTOR;
}
#endif

#ifdef WITH_CYCLONE
extern "C" uint32_t cyclone_read_memory_8(uint32_t address);
extern "C" uint32_t cyclone_read_memory_16(uint32_t address);
extern "C" uint32_t cyclone_read_memory_32(uint32_t address);
extern "C" void cyclone_write_memory_8(uint32_t address, uint8_t value);
extern "C" void cyclone_write_memory_16(uint32_t address, uint16_t value);
extern "C" void cyclone_write_memory_32(uint32_t address, uint32_t value);
extern "C" uintptr_t cyclone_checkpc(uintptr_t pc);

int cyclone_irq_callback(int level)
{
	(void)level;
	assert(md::md_cyclone != NULL);
	md::md_cyclone->m68k_vdp_irq_handler();
	return CYCLONE_INT_ACK_AUTOVECTOR;
}
#endif

/**
 * Resets everything (Z80, M68K, VDP, etc).
 * @return 0 on success
 */
int md::reset()
{
	// Clear memory.
	memset(mem, 0, 0x20000);
	// Reset the VDP.
	vdp.reset();
	// Erase CPU states.
	memset(&m68k_state, 0, sizeof(m68k_state));
	memset(&z80_state, 0, sizeof(z80_state));
	m68k_state_restore();
	z80_state_restore();
#ifdef WITH_STAR
	md_set_star(1);
	s68000reset();
	md_set_star(0);
#endif
#ifdef WITH_MUSA
	md_set_musa(1);
	m68k_pulse_reset();
	md_set_musa(0);
#endif
#ifdef WITH_CYCLONE
	md_set_cyclone(1);
	CycloneReset(&cyclonecpu);
	md_set_cyclone(0);
#endif
#ifdef WITH_DEBUGGER
	debug_m68k_instr_count = 0;
	debug_z80_instr_count = 0;
#endif
  if (debug_log) fprintf (debug_log,"reset()\n");

    aoo3_toggle=aoo5_toggle=aoo3_six=aoo5_six
    =aoo3_six_timeout=aoo5_six_timeout
    =coo4=coo5=0;
  pad[0] = MD_PAD_UNTOUCHED;
  pad[1] = MD_PAD_UNTOUCHED;
  memset(pad_com, 0, sizeof(pad_com));

#ifdef WITH_PICO
  // Initialize Pico pen X, Y coordinates
  pico_pen_coords[0] = 0x3c;
  pico_pen_coords[1] = 0x1fc;
#endif

  // Reset FM registers
  fm_reset();
  dac_init();

  memset(&odo, 0, sizeof(odo));
  ras = 0;

  z80_st_running = 0;
  m68k_st_running = 0;
  z80_reset();
  z80_st_busreq = 1;
  z80_st_reset = 1;
  z80_st_irq = 0;
  return 0;
}

#ifdef WITH_MZ80

extern "C" UINT8 mz80_read(UINT32 a, struct MemoryReadByte *unused);
extern "C" void mz80_write(UINT32 a, UINT8 d, struct MemoryWriteByte *unused);
extern "C" UINT16 mz80_ioread(UINT16 a, struct z80PortRead *unused);
extern "C" void mz80_iowrite(UINT16 a, UINT8 d, struct z80PortWrite *unused);

static struct MemoryReadByte mem_read[] = {
	{ 0x0000, 0xffff, mz80_read, NULL },
	{ (UINT32)-1, (UINT32)-1, NULL, NULL }
};

static struct MemoryWriteByte mem_write[] = {
	{ 0x0000, 0xffff, mz80_write, NULL },
	{ (UINT32)-1, (UINT32)-1, NULL, NULL }
};

static struct z80PortRead io_read[] = {
	{ 0x00, 0xff, mz80_ioread, NULL },
	{ (UINT16)-1, (UINT16)-1, NULL, NULL }
};

static struct z80PortWrite io_write[] = {
	{ 0x00, 0xff, mz80_iowrite, NULL },
	{ (UINT16)-1, (UINT16)-1, NULL, NULL }
};

#endif // WITH_MZ80

#ifdef WITH_CZ80

extern "C" uint8_t cz80_memread(void *ctx, uint16_t a);
extern "C" void cz80_memwrite(void *ctx, uint16_t a, uint8_t d);
extern "C" uint16_t cz80_memread16(void *ctx, uint16_t a);
extern "C" void cz80_memwrite16(void *ctx, uint16_t a, uint16_t d);
extern "C" uint8_t cz80_ioread(void *ctx, uint16_t a);
extern "C" void cz80_iowrite(void *ctx, uint16_t a, uint8_t d);

#endif // WITH_CZ80

#ifdef WITH_DRZ80

extern uintptr_t drz80_rebaseSP(uint16_t new_sp);
extern uintptr_t drz80_rebasePC(uint16_t new_pc);
extern uint8_t drz80_read8(uint16_t a);
extern uint16_t drz80_read16(uint16_t a);
extern void drz80_write8(uint8_t d, uint16_t a);
extern void drz80_write16(uint16_t d, uint16_t a);
extern uint8_t drz80_in(uint16_t p);
extern void drz80_out(uint16_t p, uint8_t d);

void drz80_irq_callback()
{
	md::md_drz80->drz80_irq_cb();
}

void md::drz80_irq_cb()
{
	drz80.Z80_IRQ = 0x00; // lower irq when in accepted
}

#endif // WITH_DRZ80

/**
 * Initialise the Z80.
 */
void md::z80_init()
{
#ifdef WITH_MZ80
	md_set_mz80(1);
	mz80init();
	mz80reset();
	// Erase local context with global context.
	mz80GetContext(&z80);
	// Configure callbacks in local context.
	z80.z80Base = z80ram;
	z80.z80MemRead = mem_read;
	z80.z80MemWrite = mem_write;
	z80.z80IoRead = io_read;
	z80.z80IoWrite = io_write;
	// Erase global context with the above.
	mz80SetContext(&z80);
	md_set_mz80(0);
#endif
#ifdef WITH_CZ80
	Cz80_Set_Ctx(&cz80, this);
	Cz80_Set_Fetch(&cz80, 0x0000, 0xffff, (void *)z80ram);
	Cz80_Set_ReadB(&cz80, cz80_memread);
	Cz80_Set_WriteB(&cz80, cz80_memwrite);
	Cz80_Set_ReadW(&cz80, cz80_memread16);
	Cz80_Set_WriteW(&cz80, cz80_memwrite16);
	Cz80_Set_INPort(&cz80, cz80_ioread);
	Cz80_Set_OUTPort(&cz80, cz80_iowrite);
	Cz80_Reset(&cz80);
#endif
#ifdef WITH_DRZ80
	memset(&drz80, 0, sizeof(drz80));
	drz80.z80_write8 = drz80_write8;
	drz80.z80_write16 = drz80_write16;
	drz80.z80_in = drz80_in;
	drz80.z80_out = drz80_out;
	drz80.z80_read8 = drz80_read8;
	drz80.z80_read16 = drz80_read16;
	drz80.z80_rebasePC = drz80_rebasePC;
	drz80.z80_rebaseSP = drz80_rebaseSP;
	drz80.z80_irq_callback = drz80_irq_callback;
#endif
	z80_st_busreq = 1;
	z80_st_reset = 0;
	z80_bank68k = 0xff8000;
}

/**
 * Reset the Z80.
 */
void md::z80_reset()
{
	z80_bank68k = 0xff8000;
#ifdef WITH_MZ80
	md_set_mz80(1);
	mz80reset();
	md_set_mz80(0);
#endif
#ifdef WITH_CZ80
	Cz80_Reset(&cz80);
#endif
#ifdef WITH_DRZ80
	md_set_drz80(1);
	drz80.Z80A = (1 << 2); // set ZFlag
	drz80.Z80F = (1 << 2); // set ZFlag
	drz80.Z80BC = 0;
	drz80.Z80DE = 0;
	drz80.Z80HL = 0;
	drz80.Z80A2 = 0;
	drz80.Z80F2 = 0;
	drz80.Z80BC2 = 0x0000 << 16;
	drz80.Z80DE2 = 0x0000 << 16;
	drz80.Z80HL2 = 0x0000 << 16;
	drz80.Z80IX = 0xFFFF << 16;
	drz80.Z80IY = 0xFFFF << 16;

	drz80.Z80I = 0x00;
	drz80.Z80_IRQ = 0x00;
	drz80.Z80IF = 0x00;
	drz80.Z80IM = 0x00;
	drz80.Z80R = 0x00;

	drz80.Z80PC = drz80_rebasePC(0);
	drz80.Z80SP = drz80_rebaseSP(0x2000);
	md_set_drz80(0);
#endif
}

/**
 * Initialise sound.
 * @return True when successful.
 */
bool md::init_sound()
{
	if (lock == false)
		return false;
	if (ok_ym2612) {
		YM2612Shutdown();
		ok_ym2612 = false;
	}
	if (ok_sn76496) {
		(void)0;
		ok_sn76496 = false;
	}
	// Initialize two additional chips when MJazz is enabled.
	if (YM2612Init((dgen_mjazz ? 3 : 1),
		       (((pal) ? PAL_MCLK : NTSC_MCLK) / 7),
		       dgen_soundrate, dgen_mjazz, NULL, NULL))
		return false;
	ok_ym2612 = true;
	if (SN76496_init(0,
			 (((pal) ? PAL_MCLK : NTSC_MCLK) / 15),
			 dgen_soundrate, 16))
		return false;
	ok_sn76496 = true;
	return true;
}

/**
 * Switch to PAL or NTSC.
 * This method's name is a bit misleading.  This switches to PAL or not
 * depending on "md::pal".
 */
void md::init_pal()
{
	unsigned int hc;

	if (pal) {
		mclk = PAL_MCLK;
		lines = PAL_LINES;
		vhz = PAL_HZ;
	}
	else {
		mclk = NTSC_MCLK;
		lines = NTSC_LINES;
		vhz = NTSC_HZ;
	}
	clk0 = (mclk / 15);
	clk1 = (mclk / 7);
	// Initialize horizontal counter table (Gens style)
	for (hc = 0; (hc < 512); ++hc) {
		// H32
		hc_table[hc][0] = (((hc * 170) / M68K_CYCLES_PER_LINE) - 0x18);
		// H40
		hc_table[hc][1] = (((hc * 205) / M68K_CYCLES_PER_LINE) - 0x1c);
	}
}

bool md::lock = false;

/**
 * MD constructor.
 * @param pal True if we are running the MD in PAL mode.
 * @param region Region to emulate ('J', 'U', or 'E').
 */
md::md(bool pal, char region):
#ifdef WITH_MUSA
	md_musa_ref(0), md_musa_prev(0),
#endif
#ifdef WITH_CYCLONE
	md_cyclone_ref(0), md_cyclone_prev(0),
#endif
#ifdef WITH_STAR
	md_star_ref(0), md_star_prev(0),
#endif
#ifdef WITH_CZ80
	md_cz80_ref(0),
#endif
#ifdef WITH_MZ80
	md_mz80_ref(0), md_mz80_prev(0),
#endif
	pal(pal), ok_ym2612(false), ok_sn76496(false),
	vdp(*this), region(region), plugged(false)
{
	// Only one MD object is allowed to exist at once.
	if (lock)
		return;
	lock = true;

	// PAL or NTSC.
	init_pal();

	// Start up the sound chips.
	if (init_sound() == false)
		goto cleanup;

	romlen = no_rom_size;
	rom = (uint8_t*)no_rom;
  mem=ram=z80ram=saveram=NULL;
  save_start=save_len=save_prot=save_active=0;

  fm_reset();

#ifdef WITH_VGMDUMP
	vgm_dump_file = NULL;
	vgm_dump_samples_total = 0;
	vgm_dump_dac_wait = 0;
	vgm_dump_dac_samples = 0;
	vgm_dump = false;
#endif

#ifdef WITH_PICO
	pico_enabled = false;
#endif

  memset(&m68k_state, 0, sizeof(m68k_state));
  memset(&z80_state, 0, sizeof(z80_state));

#ifdef WITH_MUSA
	ctx_musa = calloc(1, m68k_context_size());
	if (ctx_musa == NULL)
		goto cleanup;
	md_set_musa(1);
	m68k_init();
	m68k_set_cpu_type(M68K_CPU_TYPE_68000);
	m68k_register_memory(NULL, 0);
	m68k_set_int_ack_callback(musa_irq_callback);
	md_set_musa(0);
#endif

#ifdef WITH_STAR
  fetch=NULL;
  readbyte=readword=writebyte=writeword=NULL;
  memset(&cpu,0,sizeof(cpu));
#endif

#ifdef WITH_CYCLONE
  memset(&cyclonecpu, 0, sizeof(cyclonecpu));
  cyclonecpu.read8 = cyclone_read_memory_8;
  cyclonecpu.read16 = cyclone_read_memory_16;
  cyclonecpu.read32 = cyclone_read_memory_32;
  cyclonecpu.write8 = cyclone_write_memory_8;
  cyclonecpu.write16 = cyclone_write_memory_16;
  cyclonecpu.write32 = cyclone_write_memory_32;
  cyclonecpu.checkpc = cyclone_checkpc;
  cyclonecpu.fetch8 = cyclone_read_memory_8;
  cyclonecpu.fetch16 = cyclone_read_memory_16;
  cyclonecpu.fetch32 = cyclone_read_memory_32;
  cyclonecpu.IrqCallback = cyclone_irq_callback;
  md_set_cyclone(1);
  CycloneInit();
  md_set_cyclone(0);
#endif

#ifdef WITH_MZ80
  memset(&z80,0,sizeof(z80));
#endif
#ifdef WITH_CZ80
  Cz80_Init(&cz80);
#endif
#ifdef WITH_DRZ80
  memset(&drz80, 0, sizeof(drz80));
#endif
  memset(&cart_head, 0, sizeof(cart_head));

  memset(romname, 0, sizeof(romname));

  ok=0;

  //  Format of pad is: __SA____ UDLRBC__

  rom = (uint8_t*)no_rom;
  romlen = no_rom_size;
  mem=ram=z80ram=NULL;
  mem=(unsigned char *)malloc(0x20008);
	if (mem == NULL)
		goto cleanup;
  memset(mem,0,0x20000);
  ram=   mem+0x00000;
  z80ram=mem+0x10000;
  // Hack for DrZ80 to avoid crashing when PC leaves z80ram.
  z80ram[0x10000] = 0x00; // NOP
  z80ram[0x10001] = 0x00; // NOP
  z80ram[0x10002] = 0x00; // NOP
  z80ram[0x10003] = 0x00; // NOP
  z80ram[0x10004] = 0x00; // NOP
  z80ram[0x10005] = 0xc3; // JP 0x0000
  z80ram[0x10006] = 0x00;
  z80ram[0x10007] = 0x00;

#ifdef WITH_MUSA
	md_set_musa(1);
	musa_memory_map();
	md_set_musa(0);
#endif
#ifdef WITH_STAR
	md_set_star(1);
	if (s68000init() != 0) {
		md_set_star(0);
		printf ("s68000init failed!\n");
		goto cleanup;
	}
	md_set_star(0);

// Dave: Rich said doing point star stuff is done after s68000init
// in Asgard68000, so just in case...
	if (((fetch = new STARSCREAM_PROGRAMREGION [6]) == NULL) ||
	    ((readbyte = new STARSCREAM_DATAREGION [5]) == NULL) ||
	    ((readword = new STARSCREAM_DATAREGION [5]) == NULL) ||
	    ((writebyte = new STARSCREAM_DATAREGION [5]) == NULL) ||
	    ((writeword = new STARSCREAM_DATAREGION [5]) == NULL))
		goto cleanup;

  memory_map();

  // point star stuff
  cpu.s_fetch     = cpu.u_fetch     =     fetch;
  cpu.s_readbyte  = cpu.u_readbyte  =  readbyte;
  cpu.s_readword  = cpu.u_readword  =  readword;
  cpu.s_writebyte = cpu.u_writebyte = writebyte;
  cpu.s_writeword = cpu.u_writeword = writeword;

	cpu.inthandler = star_irq_callback;
	md_set_star(1);
	s68000reset();
	md_set_star(0);
#endif

	// M68K: 0 = none, 1 = StarScream, 2 = Musashi, 3 = Cyclone
	switch (dgen_emu_m68k) {
#ifdef WITH_STAR
	case 1:
		cpu_emu = CPU_EMU_STAR;
		break;
#endif
#ifdef WITH_MUSA
	case 2:
		cpu_emu = CPU_EMU_MUSA;
		break;
#endif
#ifdef WITH_CYCLONE
	case 3:
		cpu_emu = CPU_EMU_CYCLONE;
		break;
#endif
	default:
		cpu_emu = CPU_EMU_NONE;
		break;
	}
	// Z80: 0 = none, 1 = CZ80, 2 = MZ80, 3 = DrZ80
	switch (dgen_emu_z80) {
#ifdef WITH_MZ80
	case 1:
		z80_core = Z80_CORE_MZ80;
		break;
#endif
#ifdef WITH_CZ80
	case 2:
		z80_core = Z80_CORE_CZ80;
		break;
#endif
#ifdef WITH_DRZ80
	case 3:
		z80_core = Z80_CORE_DRZ80;
		break;
#endif
	default:
		z80_core = Z80_CORE_NONE;
		break;
	}

#ifdef WITH_MUSA
	md_set_musa(1);
	m68k_pulse_reset();
	md_set_musa(0);
#endif

#ifdef WITH_DEBUGGER
	debug_init();
#endif

  z80_init();

  reset(); // reset megadrive

	patch_elem = NULL;

  ok=1;

	return;
cleanup:
	if (ok_ym2612)
		YM2612Shutdown();
	if (ok_sn76496)
		(void)0;
#ifdef WITH_MUSA
	free(ctx_musa);
#endif
#ifdef WITH_STAR
	delete [] fetch;
	delete [] readbyte;
	delete [] readword;
	delete [] writebyte;
	delete [] writeword;
#endif
	free(mem);
	memset(this, 0, sizeof(*this));
	lock = false;
}

md::~md()
{
#ifdef WITH_VGMDUMP
	vgm_dump_stop();
#endif

	assert(rom != NULL);
	if (rom != no_rom)
		unplug();

  free(mem);
  rom=mem=ram=z80ram=NULL;

#ifdef WITH_DEBUGGER
	debug_leave();
#endif
#ifdef WITH_MUSA
	free(ctx_musa);
#endif
#ifdef WITH_STAR
	delete [] fetch;
	delete [] readbyte;
	delete [] readword;
	delete [] writebyte;
	delete [] writeword;
#endif

	if (ok_ym2612)
		YM2612Shutdown();
	if (ok_sn76496)
		(void)0;
	ok=0;
	memset(this, 0, sizeof(*this));
	lock = false;
}

#ifdef ROM_BYTESWAP
/**
 * Byteswaps memory.
 * @param[in] start Byte array of cart memory.
 * @param len How many bytes to byteswap.
 * @return 0 on success (always 0).
 */
// Byteswaps memory
int byteswap_memory(unsigned char *start,int len)
{ int i; unsigned char tmp;
  for (i=0;i<len;i+=2)
  { tmp=start[i+0]; start[i+0]=start[i+1]; start[i+1]=tmp; }
  return 0;
}
#endif

/**
 * Plug a cart into the MD.
 * @param[in] cart Cart's memory as a byte array.
 * @param len Length of the cart.
 * @return 0 on success.
 */
int md::plug_in(unsigned char *cart,int len)
{
  // Plug in the cartridge specified by the uchar *
  // NB - The megadrive will free() it if unplug() is called, or it exits
  // So it must be a single piece of malloced data
  if (cart==NULL) return 1; if (len<=0) return 1;
#ifdef ROM_BYTESWAP
  byteswap_memory(cart,len); // for starscream
#endif
  romlen=len;
  rom=cart;
  // Get saveram start, length (remember byteswapping)
  // First check magic, if there is saveram
  if(rom[ROM_ADDR(0x1b0)] == 'R' && rom[ROM_ADDR(0x1b1)] == 'A')
    {
      save_start = rom[ROM_ADDR(0x1b4)] << 24 | rom[ROM_ADDR(0x1b5)] << 16 |
                   rom[ROM_ADDR(0x1b6)] << 8  | rom[ROM_ADDR(0x1b7)];
      save_len = rom[ROM_ADDR(0x1b8)] << 24 | rom[ROM_ADDR(0x1b9)] << 16 |
                 rom[ROM_ADDR(0x1ba)] << 8  | rom[ROM_ADDR(0x1bb)];
      // Make sure start is even, end is odd, for alignment
// A ROM that I came across had the start and end bytes of
// the save ram the same and wouldn't work.  Fix this as seen
// fit, I know it could probably use some work. [PKH]
      if(save_start != save_len) {
        if(save_start & 1) --save_start;
        if(!(save_len & 1)) ++save_len;
        save_len -= (save_start - 1);
        saveram = (unsigned char*)calloc(1, save_len);
	if (saveram == NULL) {
	  save_len = 0;
	  save_start = 0;
	}
	// If save RAM does not overlap main ROM, set it active by default since
	// a few games can't manage to properly switch it on/off.
	if(save_start >= (unsigned int)romlen)
	  save_active = 1;
      }
      else {
        save_start = save_len = 0;
        saveram = NULL;
      }
    }
  else
    {
      save_start = save_len = 0;
      saveram = NULL;
    }
#ifdef WITH_MUSA
	md_set_musa(1);
	musa_memory_map();
	md_set_musa(0);
#endif
#ifdef WITH_STAR
	md_set_star(1);
	memory_map(); // Update memory map to include this cartridge
	md_set_star(0);
#endif
  reset(); // Reset megadrive
  return 0;
}

/**
 * Region to emulate according to dgen_region_order and ROM header.
 * @return Region identifier ('J', 'U' or 'E').
 */
uint8_t md::region_guess()
{
	char const* order = dgen_region_order.val;
	char const* avail = this->cart_head.countries;
	size_t r;
	size_t i;

	assert(order != NULL);
	assert(avail != NULL);
	for (r = 0; (order[r] != '\0'); ++r)
		for (i = 0; (i != sizeof(this->cart_head.countries)); ++i)
			if ((isprint(order[r])) &&
			    (toupper(order[r]) == toupper(avail[i])))
				return toupper(order[r]);
	// Use default region.
	return dgen_region;
}

/**
 * Unplug a cart from the system.
 * @return 0 on success.
 */
int md::unplug()
{
  assert(rom != NULL);
  assert(romlen != 0);
  if (rom == no_rom) return 1;
  unload_rom(rom);
  rom = (uint8_t*)no_rom;
  romlen = no_rom_size;
  free(saveram);
  saveram = NULL;
  save_start = save_len = 0;
#ifdef WITH_MUSA
	md_set_musa(1);
	musa_memory_map();
	md_set_musa(0);
#endif
#ifdef WITH_STAR
	md_set_star(1);
	memory_map(); // Update memory map to include no rom
	md_set_star(0);
#endif
  memset(romname, 0, sizeof(romname));
  memset(&cart_head, 0, sizeof(cart_head));
  reset();

	while (patch_elem != NULL) {
		struct patch_elem *next = patch_elem->next;

		free(patch_elem);
		patch_elem = next;
	}
	plugged = false;

  return 0;
}

/**
 * Load a ROM.
 * @param[in] name File name of cart to load.
 * @return 0 on success.
 */
int md::load(const char *name)
{
	uint8_t *temp;
	size_t size;
	const char *b_name;

	if ((name == NULL) ||
	    ((b_name = dgen_basename(name)) == NULL))
		return 1;
	temp = load_rom(&size, name);
	if (temp == NULL)
		return 1;

	// Register name
	romname[0] = '\0';
	if ((b_name[0] != '\0')) {
		unsigned int i;

		snprintf(romname, sizeof(romname), "%s", b_name);
		for (i = 0; (romname[i] != '\0'); ++i)
			if (romname[i] == '.') {
				memset(&(romname[i]), 0,
				       (sizeof(romname) - i));
				break;
			}
	}
	if (romname[0] == '\0')
		snprintf(romname, sizeof(romname), "%s", "unknown");

  // Fill the header with ROM info (god this is ugly)
  memcpy((void*)cart_head.system_name,  (void*)(temp + 0x100), 0x10);
  memcpy((void*)cart_head.copyright,    (void*)(temp + 0x110), 0x10);
  memcpy((void*)cart_head.domestic_name,(void*)(temp + 0x120), 0x30);
  memcpy((void*)cart_head.overseas_name,(void*)(temp + 0x150), 0x30);
  memcpy((void*)cart_head.product_no,   (void*)(temp + 0x180), 0x0e);
  cart_head.checksum = temp[0x18e]<<8 | temp[0x18f]; // ugly, but endian-neutral
  memcpy((void*)cart_head.control_data, (void*)(temp + 0x190), 0x10);
  cart_head.rom_start  = temp[0x1a0]<<24 | temp[0x1a1]<<16 | temp[0x1a2]<<8 | temp[0x1a3];
  cart_head.rom_end    = temp[0x1a4]<<24 | temp[0x1a5]<<16 | temp[0x1a6]<<8 | temp[0x1a7];
  cart_head.ram_start  = temp[0x1a8]<<24 | temp[0x1a9]<<16 | temp[0x1aa]<<8 | temp[0x1ab];
  cart_head.ram_end    = temp[0x1ac]<<24 | temp[0x1ad]<<16 | temp[0x1ae]<<8 | temp[0x1af];
  cart_head.save_magic = temp[0x1b0]<<8 | temp[0x1b1];
  cart_head.save_flags = temp[0x1b2]<<8 | temp[0x1b3];
  cart_head.save_start = temp[0x1b4]<<24 | temp[0x1b5]<<16 | temp[0x1b6]<<8 | temp[0x1b7];
  cart_head.save_end   = temp[0x1b8]<<24 | temp[0x1b9]<<16 | temp[0x1ba]<<8 | temp[0x1bb];
  memcpy((void*)cart_head.memo,       (void*)(temp + 0x1c8), 0x28);
  memcpy((void*)cart_head.countries,  (void*)(temp + 0x1f0), 0x10);

#ifdef WITH_PICO
	// Check if cartridge inserted is intended for Sega Pico.
	// If it is, the Sega Pico I/O area will be enabled, and the
	// Megadrive I/O area will be disabled.
	if ((!strncmp(cart_head.system_name, "SEGA PICO", 9)) ||
	    (!strncmp(cart_head.system_name, "SEGATOYS PICO", 13)))
		pico_enabled = true;
	else
		pico_enabled = false;
#endif
	// Plug it into the memory map
	plug_in(temp, size); // md then deallocates it when it's done
	plugged = true;
	return 0;
}

/**
 * Cycle through Z80 CPU implementations.
 */
void md::cycle_z80()
{
	z80_state_dump();
	z80_core = (enum z80_core)((z80_core + 1) % Z80_CORE_TOTAL);
	z80_state_restore();
}

/**
 * Cycle between M68K CPU implementations.
 */
void md::cycle_cpu()
{
	m68k_state_dump();
	cpu_emu = (enum cpu_emu)((cpu_emu + 1) % CPU_EMU_TOTAL);
	m68k_state_restore();
}

/**
 * Dump Z80 ram to a file named "dgz80ram".
 * @return Always returns 0.
 */
int md::z80dump()
{
  FILE *hand;
  hand = dgen_fopen(NULL, "dgz80ram", DGEN_WRITE);
  if (hand!=NULL)
  { fwrite(z80ram,1,0x10000,hand); fclose(hand); }
  return 0;
}

/**
 * This takes a comma or whitespace-separated list of Game Genie and/or hex
 * codes to patch the ROM with.
 * @param[in] list List of codes separated by '\\t', '\\n', or ','.
 * @param[out] errors Number of codes that failed to apply.
 * @param[out] applied Number of codes that applied correctly.
 * @param[out] reverted Number of codes that were reverted.
 * @return 0 on success.
 */
int md::patch(const char *list, unsigned int *errors,
	      unsigned int *applied, unsigned int *reverted)
{
  static const char delims[] = " \t\n,";
  char *worklist, *tok;
  struct patch p;
  int ret = 0;
  size_t wl_sz;

  if (errors != NULL)
    *errors = 0;
  if (applied != NULL)
    *applied = 0;
  if (reverted != NULL)
    *reverted = 0;

  // Copy the given list to a working list so we can strtok it
  wl_sz = strlen(list) + 1;
  worklist = (char *)malloc(wl_sz);
  if (worklist == NULL)
    return -1;
  strncpy(worklist, list, wl_sz);

  for(tok = strtok(worklist, delims); tok; tok = strtok(NULL, delims))
    {
      struct patch_elem *elem = patch_elem;
      struct patch_elem *prev = NULL;
      uint8_t *dest = rom;
      size_t mask = ~(size_t)0;
      int rev = 0;
      bool swap = true;

      // If it's empty, toss it
      if(*tok == '\0') continue;
      // Decode it
      decode(tok, &p);
      // Discard it if it was bad code
      if (((signed)p.addr == -1) || (p.addr >= (size_t)(romlen - 1))) {
		if ((p.addr < 0xff0000) || (p.addr >= 0xffffff)) {
			printf("Bad patch \"%s\"\n", tok);
			if (errors != NULL)
				++(*errors);
			ret = -1;
			continue;
		}
		// This is a RAM patch.
		dest = ram;
		mask = 0xffff;
      }
      if (dest == no_rom) {
	      printf("Cannot patch this ROM\n");
	      continue;
      }
#ifndef ROM_BYTESWAP
      if (dest == rom)
	      swap = false;
#endif
      // Put it into dest (remember byteswapping)
      while (elem != NULL) {
	if (elem->addr == p.addr) {
	  // Revert a previous patch.
	  p.data = elem->data;
	  if (prev != NULL)
	    prev->next = elem->next;
	  else
	    patch_elem = NULL;
	  free(elem);
	  rev = 1;
	  break;
	}
	prev = elem;
	elem = elem->next;
      }
      if (rev) {
        printf("Reverting patch \"%s\" -> %06X\n", tok, p.addr);
	if (reverted != NULL)
	  ++(*reverted);
      }
      else {
	printf("Patch \"%s\" -> %06X:%04X\n", tok, p.addr, p.data);
	if (applied != NULL)
	  ++(*applied);
	if ((elem = (struct patch_elem *)malloc(sizeof(*elem))) != NULL) {
	  elem->next = patch_elem;
	  elem->addr = p.addr;
	  elem->data = ((dest[((p.addr + 0) ^ swap) & mask] << 8) |
			(dest[((p.addr + 1) ^ swap) & mask]));
	  patch_elem = elem;
	}
      }
      dest[((p.addr + 0) ^ swap) & mask] = (uint8_t)(p.data >> 8);
      dest[((p.addr + 1) ^ swap) & mask] = (uint8_t)(p.data & 0xff);
    }
  // Done!
  free(worklist);
  return ret;
}

/**
 * Get saveram from FILE*.
 * @param from File to read from.
 * @return 0 on success.
 */
int md::get_save_ram(FILE *from)
{
	return !fread((void*)saveram, save_len, 1, from);
}

/**
 * Write a saveram to FILE*.
 * @param into File to write to.
 * @return 0 on success.
 */
int md::put_save_ram(FILE *into)
{
	return !fwrite((void*)saveram, save_len, 1, into);
}

/**
 * Calculates a ROM's checksum.
 * @param rom ROM memory area.
 * @param len ROM size.
 * @return Checksum.
 */
static unsigned short calculate_checksum(unsigned char *rom,int len)
{
  unsigned short checksum=0;
  int i;
  for (i=512;i<=(len-2);i+=2)
  {
    checksum+=(rom[ROM_ADDR(i+0)]<<8);
    checksum+=rom[ROM_ADDR(i+1)];
  }
  return checksum;
}

/**
 * Replace the in-memory ROM checksum with a calculated checksum.
 */
void md::fix_rom_checksum()
{
  unsigned short cs; cs=calculate_checksum(rom,romlen);
  if (romlen>=0x190) { rom[ROM_ADDR(0x18e)]=cs>>8; rom[ROM_ADDR(0x18f)]=cs&255; }
}

/**
 * This is the default ROM, used when nothing is loaded.
 */
#ifdef ROM_BYTESWAP

const uint8_t md::no_rom[] = {
	// Note: everything is byte swapped.
	"\x72\x4e" "\xff\xff" // stop #0xffff
	"\x71\x4e"            // nop
	"\x71\x4e"            // nop
	"\xf6\x60"            // bra.b 0
};

#else

const uint8_t md::no_rom[] = {
	"\x4e\x72" "\xff\xff" // stop #0xffff
	"\x4e\x71"            // nop
	"\x4e\x71"            // nop
	"\x60\xf6"            // bra.b 0
};

#endif

const size_t md::no_rom_size = sizeof(no_rom);
