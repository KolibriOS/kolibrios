#ifndef MEM_H_
#define MEM_H_


////           ////
// Z80 MEMORYMAP //
////           ////

/*
 * 0x0000-0x3fff: Z80 RAM
 * 0000-1FFFh : RAM
 * 2000-3FFFh : RAM (mirror)
 */
#define Z80_RAM_START 0x0000
#define Z80_RAM_END 0x3fff

/*
 * 0x4000-0x5fff: YM2612
 * The YM2612 has two address lines, so it is available at 4000-4003h and
 * is mirrored repeatedly up to 5FFFh.
 */
#define YM2612_RAM_START 0x4000
#define YM2612_RAM_END 0x5fff

/* 0x6000-0x6fff: bank register */
#define BANK_RAM_START 0x6000
#define BANK_RAM_END 0x6fff

/* 0x7000-0x7fff: PSG/VDP */
#define PSGVDP_RAM_START 0x7000
#define PSGVDP_RAM_END 0x7fff

/* 0x8000-0xffff: M68K bank */
#define M68K_RAM_START 0x8000
#define M68K_RAM_END 0xffff


////            ////
// m68K MEMORYMAP //
////            ////

/* 0x000000-0x7fffff: Rom */
#define M68K_ROM_START 0x000000
#define M68K_ROM_END 0x7fffff

/* 0x800000-0x9fffff: empty area1 */
#define M68K_EMPTY1_START 0x800000
#define M68K_EMPTY1_END 0x9fffff

/* 0xa00000-0xafffff: system I/O and control */
#define M68K_IO_START 0xa00000
#define M68K_IO_END 0xafffff

/* 0xb00000-0xbfffff: empty area */
#define M68K_EMPTY2_START 0xb00000
#define M68K_EMPTY2_END 0xbfffff

/* 0xc00000-0xdfffff: VDP/PSG */
#define M68K_VDP_START 0xc00000
#define M68K_VDP_END 0xdfffff

#endif
