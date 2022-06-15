/**
 * @file
 * DGen v1.13+
 * Megadrive's VDP C++ module
 *
 * A useful resource for the Genesis VDP:
 * http://cgfm2.emuviews.com/txt/genvdp.txt
 * Thanks to Charles MacDonald for writing these docs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "md.h"

/** Reset the VDP. */
void md_vdp::reset()
{
	hint_pending = false;
	vint_pending = false;
	cmd_pending = false;
	rw_mode = 0x00;
	rw_addr = 0;
	rw_dma = 0;
	memset(mem, 0, sizeof(mem));
	memset(reg, 0, 0x20);
	memset(dirt, 0xff, 0x35); // mark everything as changed
	memset(highpal, 0, sizeof(highpal));
	memset(sprite_order, 0, sizeof(sprite_order));
	memset(sprite_mask, 0xff, sizeof(sprite_mask));
	sprite_base = NULL;
	sprite_count = 0;
	masking_sprite_index_cache = -1;
	dots_cache = 0;
	sprite_overflow_line = INT_MIN;
	dest = NULL;
	bmap = NULL;
}

/**
 * VDP constructor.
 *
 * @param md The md instance this VDP belongs to.
 */
md_vdp::md_vdp(md& md): belongs(md)
{
	vram = (mem + 0x00000);
	cram = (mem + 0x10000);
	vsram = (mem + 0x10080);
	dirt = (mem + 0x10100); // VRAM/CRAM/Reg dirty buffer bitfield
	// Also in 0x34 are global dirt flags (inclduing VSRAM this time)
	Bpp = Bpp_times8 = 0;
	reset();
}

/**
 * VDP destructor.
 */
md_vdp::~md_vdp()
{
	vram = cram = vsram = NULL;
}

/** Calculate the DMA length. */
int md_vdp::dma_len()
{ return (reg[0x14]<<8)+reg[0x13]; }

/** Calculate DMA start address. */
int md_vdp::dma_addr()
{
  int addr=0;
  addr=(reg[0x17]&0x7f)<<17;
  addr+=reg[0x16]<<9;
  addr+=reg[0x15]<<1;
  return addr;
}


/**
 * Do a DMA read.
 * DMA can read from anywhere.
 *
 * @param addr Address where to read from.
 * @return Byte read at "addr".
 */
unsigned char md_vdp::dma_mem_read(int addr)
{
  return belongs.misc_readbyte(addr);
}

/**
 * Set value in VRAM.
 * Must go through these calls to update the dirty flags.
 *
 * @param addr Address to write to.
 * @param d Byte to write.
 * @return Always 0.
 */
int md_vdp::poke_vram(int addr,unsigned char d)
{
  addr&=0xffff;
  if (vram[addr]!=d)
  {
    // Store dirty information down to 256 byte level in bits
    int byt,bit;
    byt=addr>>8; bit=byt&7; byt>>=3; byt&=0x1f;
    dirt[0x00+byt]|=(1<<bit); dirt[0x34]|=1;
    vram[addr]=d;
  }
  return 0;
}

/**
 * Set value in CRAM.
 *
 * @param addr Address to write to.
 * @param d Byte to write.
 * @return Always 0.
 */
int md_vdp::poke_cram(int addr,unsigned char d)
{
  addr&=0x007f;
  if (cram[addr]!=d)
  {
    // Store dirty information down to 1byte level in bits
    int byt,bit;
    byt=addr; bit=byt&7; byt>>=3; byt&=0x0f;
    dirt[0x20+byt]|=(1<<bit); dirt[0x34]|=2;
    cram[addr]=d;
  }

  return 0;
}

/**
 * Set value in VSRAM.
 *
 * @param addr Address to write to.
 * @param d Byte to write.
 * @return Always 0.
 */
int md_vdp::poke_vsram(int addr,unsigned char d)
{
//  int diff=0;
  addr&=0x007f;
  if (vsram[addr]!=d)
  { dirt[0x34]|=4; vsram[addr]=d; }
  return 0;
}

/**
 * Write a word to memory and update dirty flags.
 *
 * @param d 16-bit data to write.
 * @return Always 0.
 */
int md_vdp::putword(unsigned short d)
{
  // Called by dma or a straight write
  switch(rw_mode)
  {
	case 0x04:
		if (rw_addr & 0x0001) {
			poke_vram((rw_addr + 0), (d & 0xff));
			poke_vram((rw_addr + 1), (d >> 8));
		}
		else {
			poke_vram((rw_addr + 0), (d >> 8));
			poke_vram((rw_addr + 1), (d & 0xff));
		}
		break;
	case 0x0c:
		poke_cram((rw_addr + 0), (d >> 8));
		poke_cram((rw_addr + 1), (d & 0xff));
		break;
	case 0x14:
		poke_vsram((rw_addr + 0), (d >> 8));
		poke_vsram((rw_addr + 1), (d & 0xff));
		break;
  }
  rw_addr+=reg[15];
  return 0;
}

/**
 * Write a byte to memory and update dirty flags.
 *
 * @param d 8-bit data to write.
 * @return Always 0.
 */
int md_vdp::putbyte(unsigned char d)
{
  // Called by dma or a straight write
  switch(rw_mode)
  {
    case 0x04: poke_vram (rw_addr,d); break;
    case 0x0c: poke_cram (rw_addr,d); break;
    case 0x14: poke_vsram(rw_addr,d); break;
  }
  rw_addr+=reg[15];
  return 0;
}

#undef MAYCHANGE

/**
 * Read a word from memory.
 *
 * @return Read word.
 */
unsigned short md_vdp::readword()
{
  // Called by a straight read only
  unsigned short result=0x0000;
  switch(rw_mode)
  {
    case 0x00: result=( vram[(rw_addr+0)&0xffff]<<8)+
                        vram[(rw_addr+1)&0xffff]; break;
    case 0x20: result=( cram[(rw_addr+0)&0x007f]<<8)+
                        cram[(rw_addr+1)&0x007f]; break;
    case 0x10: result=(vsram[(rw_addr+0)&0x007f]<<8)+
                       vsram[(rw_addr+1)&0x007f]; break;
  }
  rw_addr+=reg[15];
  return result;
}

/**
 * Read a byte from memory.
 *
 * @return Read byte.
 */
unsigned char md_vdp::readbyte()
{
  // Called by a straight read only
  unsigned char result=0x00;
  switch(rw_mode)
  {
    case 0x00: result= vram[(rw_addr+0)&0xffff]; break;
    case 0x20: result= cram[(rw_addr+0)&0x007f]; break;
    case 0x10: result=vsram[(rw_addr+0)&0x007f]; break;
  }
  rw_addr+=reg[15];
  return result;
}

/**
 * VDP commands
 *
 * A VDP command is 32-bits in length written into the control port
 * as two 16-bit words. The VDP maintains a pending flag so that it knows
 * what to expect next.
 *
 *  CD1 CD0 A13 A12 A11 A10 A09 A08     (D31-D24)
 *  A07 A06 A05 A04 A03 A02 A01 A00     (D23-D16)
 *   ?   ?   ?   ?   ?   ?   ?   ?      (D15-D8)
 *  CD5 CD4 CD3 CD2  ?   ?  A15 A14     (D7-D0)
 *
 * Where CD* indicates which ram is read or written in subsequent
 * data port read/writes. A* is an address.
 *
 * Note that the command is not cached, but rather, the lower 14 address bits
 * are commited as soon as the first half of the command arrives. Then when
 * the second word arrives, the remaining two address bits are commited.
 *
 * It is possible to cancel (but not roll back) a pending command by:
 *  - reading or writing to the data port.
 *  - reading the control port.
 *
 * In these cases the pending flag is cleared, and the first half of
 * the command remains comitted.
 *
 * @return Always 0.
 */
int md_vdp::command(uint16_t cmd)
{
  if (cmd_pending) // If this is the second word of a command
  {
    uint16_t A14_15 = (cmd & 0x0003) << 14;
    rw_addr = (rw_addr & 0xffff3fff) | A14_15;

    // Copy rw_addr to mirror register
    rw_addr = (rw_addr & 0x0000ffff) | (rw_addr << 16);

    // CD{4,3,2}
    uint16_t CD4_2 = (cmd & 0x0070);
    rw_mode |= CD4_2;

    // if CD5 == 1
    rw_dma = ((cmd & 0x80) == 0x80);

    cmd_pending = false;
  }
  else // This is the first word of a command
  {
    // masking away command bits CD1 CD0
    uint16_t A00_13 = cmd & 0x3fff;
    rw_addr = (rw_addr & 0xffffc000) | A00_13;

    // Copy rw_addr to mirror register
    rw_addr = (rw_addr & 0x0000ffff) | (rw_addr << 16);

    // CD {1,0}
    uint16_t CD0_1 = (cmd & 0xc000) >> 12;
    rw_mode = CD0_1;
    rw_dma = 0;

    // we will expect the second half of the command next
    cmd_pending = true;

    return 0;
  }

  // if it's a dma request do it straight away
  if (rw_dma)
  {
    int mode=(reg[0x17]>>6)&3;
    int s=0,d=0,i=0,len=0;
    s=dma_addr(); d=rw_addr; len=dma_len();
    (void)d;
    switch (mode)
    {
      case 0: case 1:
        for (i=0;i<len;i++)
        {
          unsigned short val;
          val= dma_mem_read(s++); val<<=8;
          val|=dma_mem_read(s++); putword(val);
        }
      break;
      case 2:
        // Done later on (VRAM fill I believe)
      break;
      case 3:
        for (i=0;i<len;i++)
        {
          unsigned short val;
          val= vram[(s++)&0xffff]; val<<=8;
          val|=vram[(s++)&0xffff]; putword(val);
        }
      break;
    }
  }

  return 0;
}

/**
 * Write a word to the VDP.
 *
 * @param d 16-bit data to write.
 * @return Always 0.
 */
int md_vdp::writeword(unsigned short d)
{
  if (rw_dma)
  {
    // This is the 'done later on' bit for words
    // Do a dma fill if it's set up:
    if (((reg[0x17]>>6)&3)==2)
    {
      int i,len;
      len=dma_len();
      for (i=0;i<len;i++)
        putword(d);
      return 0;
    }
  }
  else
  {
    putword(d);
    return 0;
  }
  return 0;
}

/**
 * Write a byte to the VDP.
 *
 * @param d 8-bit data to write.
 * @return Always 0.
 */
int md_vdp::writebyte(unsigned char d)
{
  if (rw_dma)
  {
    // This is the 'done later on' bit for bytes
    // Do a dma fill if it's set up:
    if (((reg[0x17]>>6)&3)==2)
    {
      int i,len;
      len=dma_len();
      for (i=0;i<len;i++)
        putbyte(d);
      return 0;
    }
  }
  else
  {
    putbyte(d);
    return 0;
  }

  return 0;
}

/**
 * Write away a VDP register.
 *
 * @param addr Address of register.
 * @param data 8-bit data to write.
 */
void md_vdp::write_reg(uint8_t addr, uint8_t data)
{
	uint8_t byt, bit;

	// store dirty information down to 1 byte level in bits
	if (reg[addr] != data) {
		byt = addr;
		bit = (byt & 7);
		byt >>= 3;
		byt &= 0x03;
		dirt[(0x30 + byt)] |= (1 << bit);
		dirt[0x34] |= 8;
	}
	reg[addr] = data;
	// "Writing to a VDP register will clear the code register."
	rw_mode = 0;
}
