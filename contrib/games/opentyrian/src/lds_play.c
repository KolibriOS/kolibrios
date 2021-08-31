/* 
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "lds_play.h"

#include "file.h"
#include "loudness.h"
#include "opentyr.h"

#include <assert.h>

const unsigned char op_table[9] = {0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12};

/* A substantial amount of this code has been copied and adapted from adplug.
   Thanks, guys! Adplug is awesome! :D */

/* Note frequency table (16 notes / octave) */
static const Uint16 frequency[(13 * 15) - 3] = {
  343, 344, 345, 347, 348, 349, 350, 352, 353, 354, 356, 357, 358,
  359, 361, 362, 363, 365, 366, 367, 369, 370, 371, 373, 374, 375,
  377, 378, 379, 381, 382, 384, 385, 386, 388, 389, 391, 392, 393,
  395, 396, 398, 399, 401, 402, 403, 405, 406, 408, 409, 411, 412,
  414, 415, 417, 418, 420, 421, 423, 424, 426, 427, 429, 430, 432,
  434, 435, 437, 438, 440, 442, 443, 445, 446, 448, 450, 451, 453,
  454, 456, 458, 459, 461, 463, 464, 466, 468, 469, 471, 473, 475,
  476, 478, 480, 481, 483, 485, 487, 488, 490, 492, 494, 496, 497,
  499, 501, 503, 505, 506, 508, 510, 512, 514, 516, 518, 519, 521,
  523, 525, 527, 529, 531, 533, 535, 537, 538, 540, 542, 544, 546,
  548, 550, 552, 554, 556, 558, 560, 562, 564, 566, 568, 571, 573,
  575, 577, 579, 581, 583, 585, 587, 589, 591, 594, 596, 598, 600,
  602, 604, 607, 609, 611, 613, 615, 618, 620, 622, 624, 627, 629,
  631, 633, 636, 638, 640, 643, 645, 647, 650, 652, 654, 657, 659,
  662, 664, 666, 669, 671, 674, 676, 678, 681, 683
};

/* Vibrato (sine) table */
static const Uint8 vibtab[25 + (13 * 3)] = {
  0, 13, 25, 37, 50, 62, 74, 86, 98, 109, 120, 131, 142, 152, 162,
  171, 180, 189, 197, 205, 212, 219, 225, 231, 236, 240, 244, 247,
  250, 252, 254, 255, 255, 255, 254, 252, 250, 247, 244, 240, 236,
  231, 225, 219, 212, 205, 197, 189, 180, 171, 162, 152, 142, 131,
  120, 109, 98, 86, 74, 62, 50, 37, 25, 13
};

/* Tremolo (sine * sine) table */
static const Uint8 tremtab[128] = {
  0, 0, 1, 1, 2, 4, 5, 7, 10, 12, 15, 18, 21, 25, 29, 33, 37, 42, 47,
  52, 57, 62, 67, 73, 79, 85, 90, 97, 103, 109, 115, 121, 128, 134,
  140, 146, 152, 158, 165, 170, 176, 182, 188, 193, 198, 203, 208,
  213, 218, 222, 226, 230, 234, 237, 240, 243, 245, 248, 250, 251,
  253, 254, 254, 255, 255, 255, 254, 254, 253, 251, 250, 248, 245,
  243, 240, 237, 234, 230, 226, 222, 218, 213, 208, 203, 198, 193,
  188, 182, 176, 170, 165, 158, 152, 146, 140, 134, 127, 121, 115,
  109, 103, 97, 90, 85, 79, 73, 67, 62, 57, 52, 47, 42, 37, 33, 29,
  25, 21, 18, 15, 12, 10, 7, 5, 4, 2, 1, 1, 0
};

static const Uint16 maxsound = 0x3f, maxpos = 0xff;

static SoundBank *soundbank = NULL;
static Channel channel[9];
static Position *positions = NULL;

static Uint8 fmchip[0xff], jumping, fadeonoff, allvolume, hardfade, tempo_now, pattplay, tempo, regbd, chandelay[9], mode, pattlen;
static Uint16 posplay, jumppos, speed;
static Uint16 *patterns = NULL;
static Uint16 numpatch, numposi, mainvolume;

bool playing, songlooped;

bool lds_load( FILE *f, unsigned int music_offset, unsigned int music_size )
{
	SoundBank *sb;
	
	fseek(f, music_offset, SEEK_SET);

	/* load header */
	mode = fgetc(f);
	if (mode > 2)
	{
		fprintf(stderr, "error: failed to load music\n");
		return false;
	}
	efread(&speed, 2, 1, f);
	tempo = fgetc(f);
	pattlen = fgetc(f);
	for (unsigned int i = 0; i < 9; i++)
		chandelay[i] = fgetc(f);
	regbd = fgetc(f);

	/* load patches */
	efread(&numpatch, 2, 1, f);

	free(soundbank);
	soundbank = malloc(sizeof(SoundBank) * numpatch);

	for (unsigned int i = 0; i < numpatch; i++)
	{
		sb = &soundbank[i];
		sb->mod_misc = fgetc(f);
		sb->mod_vol = fgetc(f);
		sb->mod_ad = fgetc(f);
		sb->mod_sr = fgetc(f);
		sb->mod_wave = fgetc(f);
		sb->car_misc = fgetc(f);
		sb->car_vol = fgetc(f);
		sb->car_ad = fgetc(f);
		sb->car_sr = fgetc(f);
		sb->car_wave = fgetc(f);
		sb->feedback = fgetc(f);
		sb->keyoff = fgetc(f);
		sb->portamento = fgetc(f);
		sb->glide = fgetc(f);
		sb->finetune = fgetc(f);
		sb->vibrato = fgetc(f);
		sb->vibdelay = fgetc(f);
		sb->mod_trem = fgetc(f);
		sb->car_trem = fgetc(f);
		sb->tremwait = fgetc(f);
		sb->arpeggio = fgetc(f);
		for (unsigned int j = 0; j < 12; j++)
			sb->arp_tab[j] = fgetc(f);
		efread(&sb->start, 2, 1, f);
		efread(&sb->size, 2, 1, f);
		sb->fms = fgetc(f);
		efread(&sb->transp, 2, 1, f);
		sb->midinst = fgetc(f);
		sb->midvelo = fgetc(f);
		sb->midkey = fgetc(f);
		sb->midtrans = fgetc(f);
		sb->middum1 = fgetc(f);
		sb->middum2 = fgetc(f);
	}
	
	/* load positions */
	efread(&numposi, 2, 1, f);
	
	free(positions);
	positions = malloc(sizeof(Position) * 9 * numposi);
	
	for (unsigned int i = 0; i < numposi; i++)
	{
		for (unsigned int j = 0; j < 9; j++)
		{
			/*
			* patnum is a pointer inside the pattern space, but patterns are 16bit
			* word fields anyway, so it ought to be an even number (hopefully) and
			* we can just divide it by 2 to get our array index of 16bit words.
			*/
			Uint16 temp;
			efread(&temp, 2, 1, f);
			positions[i * 9 + j].patnum = temp / 2;
			positions[i * 9 + j].transpose = fgetc(f);
		}
	}
	
	/* load patterns */
	fseek(f, 2, SEEK_CUR); /* ignore # of digital sounds (dunno what this is for) */
	
	unsigned int remaining = music_size - (ftell(f) - music_offset);
	
	free(patterns);
	patterns = malloc(sizeof(Uint16) * (remaining / 2));
	
	for (unsigned int i = 0; i < remaining / 2; i++)
		efread(&patterns[i], 2, 1, f);
	
	lds_rewind();
	
	return true;
}

void lds_free( void )
{
	free(soundbank);
	soundbank = NULL;
	
	free(positions);
	positions = NULL;
	
	free(patterns);
	patterns = NULL;
}

void lds_rewind( void )
{
	int i;

	/* init all with 0 */
	tempo_now = 3;
	playing = true; songlooped = false;
	jumping = fadeonoff = allvolume = hardfade = pattplay = posplay = jumppos =	mainvolume = 0;
	memset(channel, 0, sizeof(channel));
	memset(fmchip, 0, sizeof(fmchip));

	/* OPL2 init */
	opl_init();				/* Reset OPL chip */
	opl_write(1, 0x20);
	opl_write(8, 0);
	opl_write(0xbd, regbd);

	for(i = 0; i < 9; i++) {
		opl_write(0x20 + op_table[i], 0);
		opl_write(0x23 + op_table[i], 0);
		opl_write(0x40 + op_table[i], 0x3f);
		opl_write(0x43 + op_table[i], 0x3f);
		opl_write(0x60 + op_table[i], 0xff);
		opl_write(0x63 + op_table[i], 0xff);
		opl_write(0x80 + op_table[i], 0xff);
		opl_write(0x83 + op_table[i], 0xff);
		opl_write(0xe0 + op_table[i], 0);
		opl_write(0xe3 + op_table[i], 0);
		opl_write(0xa0 + i, 0);
		opl_write(0xb0 + i, 0);
		opl_write(0xc0 + i, 0);
	}
}

void lds_setregs(Uint8 reg, Uint8 val)
{
	if(fmchip[reg] == val) return;

	fmchip[reg] = val;
	opl_write(reg, val);
}

void lds_setregs_adv(Uint8 reg, Uint8 mask, Uint8 val)
{
	lds_setregs(reg, (fmchip[reg] & mask) | val);
}

int lds_update( void )
{
	Uint16 comword, freq, octave, chan, tune, wibc, tremc, arpreg;
	int vbreak;
	Uint8 level, regnum, comhi, comlo;
	int i;
	Channel *c;

	if(!playing) return false;

	/* handle fading */
	if(fadeonoff)
	{
		if(fadeonoff <= 128) {
			if(allvolume > fadeonoff || allvolume == 0)
			{
				allvolume -= fadeonoff;
			} else {
				allvolume = 1;
				fadeonoff = 0;
				if(hardfade != 0) {
					playing = false;
					hardfade = 0;
					for(i = 0; i < 9; i++)
					{
						channel[i].keycount = 1;
					}
				}
			}

		} else {
			if( (Uint8) ((allvolume + (0x100 - fadeonoff)) & 0xff) <= mainvolume)
			{
				allvolume += 0x100 - fadeonoff;
			} else {
				allvolume = mainvolume;
				fadeonoff = 0;
			}
		}
	}

	/* handle channel delay */
	for(chan = 0; chan < 9; chan++) {
		c = &channel[chan];
		if(c->chancheat.chandelay) {
			if(!(--c->chancheat.chandelay)) {
				lds_playsound(c->chancheat.sound, chan, c->chancheat.high);
			}
		}
	}

	/* handle notes */
	if(!tempo_now && positions)
	{
		vbreak = false;
		for(chan = 0; chan < 9; chan++)
		{
			c = &channel[chan];
			if(!c->packwait) {
				Uint16 patnum = positions[posplay * 9 + chan].patnum;
				Uint8 transpose = positions[posplay * 9 + chan].transpose;
				/*printf("> %p", positions);*/

				comword = patterns[patnum + c->packpos];
				comhi = comword >> 8; comlo = comword & 0xff;
				if(comword) {
					if(comhi == 0x80)
					{
						c->packwait = comlo;
					} else {
						if(comhi >= 0x80)
						{
							switch(comhi)
							{
							case 0xff:
								c->volcar = (((c->volcar & 0x3f) * comlo) >> 6) & 0x3f;
								if(fmchip[0xc0 + chan] & 1)
								c->volmod = (((c->volmod & 0x3f) * comlo) >> 6) & 0x3f;
								break;

							case 0xfe:
								tempo = comword & 0x3f;
								break;

							case 0xfd:
								c->nextvol = comlo;
								break;

							case 0xfc:
								playing = false;
								/* in real player there's also full keyoff here, but we don't need it */
								break;

							case 0xfb:
								c->keycount = 1;
								break;

							case 0xfa:
								vbreak = true;
								jumppos = (posplay + 1) & maxpos;
								break;

							case 0xf9:
								vbreak = true;
								jumppos = comlo & maxpos;
								jumping = 1;
								if(jumppos < posplay)
								{
									songlooped = true;
								}
								break;

							case 0xf8:
								c->lasttune = 0;
								break;

							case 0xf7:
								c->vibwait = 0;
								/* PASCAL: c->vibspeed = ((comlo >> 4) & 15) + 2; */
								c->vibspeed = (comlo >> 4) + 2;
								c->vibrate = (comlo & 15) + 1;
								break;

							case 0xf6:
								c->glideto = comlo;
								break;

							case 0xf5:
								c->finetune = comlo;
								break;

							case 0xf4:
								if(!hardfade)
								{
									allvolume = mainvolume = comlo;
									fadeonoff = 0;
								}
								break;

							case 0xf3:
								if(!hardfade)
								{
									fadeonoff = comlo;
								}
								break;

							case 0xf2:
								c->trmstay = comlo;
								break;

							case 0xf1:	/* panorama */

							case 0xf0:	/* progch */
								/* MIDI commands (unhandled) */
								/*AdPlug_LogWrite("CldsPlayer(): not handling MIDI command 0x%x, "
									"value = 0x%x\n", comhi);*/
								break;

							default:
								if(comhi < 0xa0)
								{
									c->glideto = comhi & 0x1f;
								} else {
									/*AdPlug_LogWrite("CldsPlayer(): unknown command 0x%x encountered!"
									  " value = 0x%x\n", comhi, comlo);*/
								}
								break;
							}
						} else {
							Uint8	sound;
							Uint16	high;
							Sint8	transp = transpose & 127;
							/*
							 * Originally, in assembler code, the player first shifted
							 * logically left the transpose byte by 1 and then shifted
							 * arithmetically right the same byte to achieve the final,
							 * signed transpose value. Since we can't do arithmetic shifts
							 * in C, we just duplicate the 7th bit into the 8th one and
							 * discard the 8th one completely.
							 */

							if(transpose & 64)
							{
								transp |= 128;
							}

							if(transpose & 128)
							{
								sound = (comlo + transp) & maxsound;
								high = comhi << 4;
							} else {
								sound = comlo & maxsound;
								high = (comhi + transp) << 4;
							}

							  /*
							PASCAL:
							  sound = comlo & maxsound;
							  high = (comhi + (((transpose + 0x24) & 0xff) - 0x24)) << 4;
							  */

							if(!chandelay[chan]) {
								lds_playsound(sound, chan, high);
							} else {
								c->chancheat.chandelay = chandelay[chan];
								c->chancheat.sound = sound;
								c->chancheat.high = high;
							}
						}
					}
				}

				c->packpos++;
			} else {
			c->packwait--;
			}
		}

		tempo_now = tempo;
		/*
		  The continue table is updated here, but this is only used in the
		  original player, which can be paused in the middle of a song and then
		  unpaused. Since AdPlug does all this for us automatically, we don't
		  have a continue table here. The continue table update code is noted
		  here for reference only.

		  if(!pattplay) {
			conttab[speed & maxcont].position = posplay & 0xff;
			conttab[speed & maxcont].tempo = tempo;
		  }
		*/
		pattplay++;
		if(vbreak)
		{
			pattplay = 0;
			for(i = 0; i < 9; i++)
			{
				channel[i].packpos = channel[i].packwait = 0;
			}
			posplay = jumppos;
		} else {
			if(pattplay >= pattlen) {
				pattplay = 0;
				for(i = 0; i < 9; i++)
				{
					channel[i].packpos = channel[i].packwait = 0;
				}
				posplay = (posplay + 1) & maxpos;
			}
		 }
	} else {
		tempo_now--;
	}

	/* make effects */
	for(chan = 0; chan < 9; chan++) {
		c = &channel[chan];
		regnum = op_table[chan];
		if(c->keycount > 0) {
			if(c->keycount == 1)
				lds_setregs_adv(0xb0 + chan, 0xdf, 0);
			c->keycount--;
		}

		/* arpeggio */
		if(c->arp_size == 0)
			arpreg = 0;
		else {
			arpreg = c->arp_tab[c->arp_pos] << 4;
			if(arpreg == 0x800) {
				if(c->arp_pos > 0) c->arp_tab[0] = c->arp_tab[c->arp_pos - 1];
				c->arp_size = 1; c->arp_pos = 0;
				arpreg = c->arp_tab[0] << 4;
			}
	
			if(c->arp_count == c->arp_speed) {
				c->arp_pos++;
				if(c->arp_pos >= c->arp_size) c->arp_pos = 0;
				c->arp_count = 0;
			} else
				c->arp_count++;
		}
	
		/* glide & portamento */
		if(c->lasttune && (c->lasttune != c->gototune)) {
			if(c->lasttune > c->gototune) {
				if(c->lasttune - c->gototune < c->portspeed)
					c->lasttune = c->gototune;
				else
					c->lasttune -= c->portspeed;
			} else {
				if(c->gototune - c->lasttune < c->portspeed)
					c->lasttune = c->gototune;
				else
					c->lasttune += c->portspeed;
			}
	
			if(arpreg >= 0x800)
				arpreg = c->lasttune - (arpreg ^ 0xff0) - 16;
			else
				arpreg += c->lasttune;
	
			freq = frequency[arpreg % (12 * 16)];
			octave = arpreg / (12 * 16) - 1;
			lds_setregs(0xa0 + chan, freq & 0xff);
			lds_setregs_adv(0xb0 + chan, 0x20, ((octave << 2) + (freq >> 8)) & 0xdf);
		} else {
			/* vibrato */
			if(!c->vibwait) {
				if(c->vibrate) {
					wibc = vibtab[c->vibcount & 0x3f] * c->vibrate;
	
					if((c->vibcount & 0x40) == 0)
						tune = c->lasttune + (wibc >> 8);
					else
						tune = c->lasttune - (wibc >> 8);
		
					if(arpreg >= 0x800)
						tune = tune - (arpreg ^ 0xff0) - 16;
					else
						tune += arpreg;
		
					freq = frequency[tune % (12 * 16)];
					octave = tune / (12 * 16) - 1;
					lds_setregs(0xa0 + chan, freq & 0xff);
					lds_setregs_adv(0xb0 + chan, 0x20, ((octave << 2) + (freq >> 8)) & 0xdf);
					c->vibcount += c->vibspeed;
				} else if(c->arp_size != 0) {	/* no vibrato, just arpeggio */
					if(arpreg >= 0x800)
						tune = c->lasttune - (arpreg ^ 0xff0) - 16;
					else
						tune = c->lasttune + arpreg;
	
					freq = frequency[tune % (12 * 16)];
					octave = tune / (12 * 16) - 1;
					lds_setregs(0xa0 + chan, freq & 0xff);
					lds_setregs_adv(0xb0 + chan, 0x20, ((octave << 2) + (freq >> 8)) & 0xdf);
				}
			} else {	/* no vibrato, just arpeggio */
				c->vibwait--;
	
				if(c->arp_size != 0) {
					if(arpreg >= 0x800)
						tune = c->lasttune - (arpreg ^ 0xff0) - 16;
					else
						tune = c->lasttune + arpreg;
	
					freq = frequency[tune % (12 * 16)];
					octave = tune / (12 * 16) - 1;
					lds_setregs(0xa0 + chan, freq & 0xff);
					lds_setregs_adv(0xb0 + chan, 0x20, ((octave << 2) + (freq >> 8)) & 0xdf);
				}
			}
		}
	
		/* tremolo (modulator) */
		if(!c->trmwait) {
			if(c->trmrate) {
				tremc = tremtab[c->trmcount & 0x7f] * c->trmrate;
				if((tremc >> 8) <= (c->volmod & 0x3f))
					level = (c->volmod & 0x3f) - (tremc >> 8);
				else
					level = 0;
				
				if(allvolume != 0 && (fmchip[0xc0 + chan] & 1))
					lds_setregs_adv(0x40 + regnum, 0xc0, ((level * allvolume) >> 8) ^ 0x3f);
				else
					lds_setregs_adv(0x40 + regnum, 0xc0, level ^ 0x3f);
				
				c->trmcount += c->trmspeed;
			} else if(allvolume != 0 && (fmchip[0xc0 + chan] & 1))
				lds_setregs_adv(0x40 + regnum, 0xc0, ((((c->volmod & 0x3f) * allvolume) >> 8) ^ 0x3f) & 0x3f);
			else
				lds_setregs_adv(0x40 + regnum, 0xc0, (c->volmod ^ 0x3f) & 0x3f);
		} else {
			c->trmwait--;
			if(allvolume != 0 && (fmchip[0xc0 + chan] & 1))
			lds_setregs_adv(0x40 + regnum, 0xc0, ((((c->volmod & 0x3f) * allvolume) >> 8) ^ 0x3f) & 0x3f);
		}
			
		/* tremolo (carrier) */
		if(!c->trcwait) {
			if(c->trcrate) {
				tremc = tremtab[c->trccount & 0x7f] * c->trcrate;
				if((tremc >> 8) <= (c->volcar & 0x3f))
					level = (c->volcar & 0x3f) - (tremc >> 8);
				else
					level = 0;
				
				if(allvolume != 0)
					lds_setregs_adv(0x43 + regnum, 0xc0, ((level * allvolume) >> 8) ^ 0x3f);
				else
					lds_setregs_adv(0x43 + regnum, 0xc0, level ^ 0x3f);
				c->trccount += c->trcspeed;
			} else if(allvolume != 0)
				lds_setregs_adv(0x43 + regnum, 0xc0, ((((c->volcar & 0x3f) * allvolume) >> 8) ^ 0x3f) & 0x3f);
			else
				lds_setregs_adv(0x43 + regnum, 0xc0, (c->volcar ^ 0x3f) & 0x3f);
		} else {
			c->trcwait--;
			if(allvolume != 0)
			lds_setregs_adv(0x43 + regnum, 0xc0, ((((c->volcar & 0x3f) * allvolume) >> 8) ^ 0x3f) & 0x3f);
		}
	}

	return (!playing || songlooped) ? false : true;
}

void lds_playsound(int inst_number, int channel_number, int tunehigh)
{
	Channel		*c = &channel[channel_number];		/* current channel */
	SoundBank		*i = &soundbank[inst_number];		/* current instrument */
	Uint32		regnum = op_table[channel_number];	/* channel's OPL2 register */
	Uint8		volcalc, octave;
	Uint16	freq;
	
	/* set fine tune */
	tunehigh += ((i->finetune + c->finetune + 0x80) & 0xff) - 0x80;
	
	/* arpeggio handling */
	if(!i->arpeggio) {
		Uint16	arpcalc = i->arp_tab[0] << 4;
	
		if(arpcalc > 0x800)
			tunehigh = tunehigh - (arpcalc ^ 0xff0) - 16;
		else
			tunehigh += arpcalc;
	}
	
	/* glide handling */
	if(c->glideto != 0) {
		c->gototune = tunehigh;
		c->portspeed = c->glideto;
		c->glideto = c->finetune = 0;
		return;
	}
	
	/* set modulator registers */
	lds_setregs(0x20 + regnum, i->mod_misc);
	volcalc = i->mod_vol;
	if(!c->nextvol || !(i->feedback & 1))
		c->volmod = volcalc;
	else
		c->volmod = (volcalc & 0xc0) | ((((volcalc & 0x3f) * c->nextvol) >> 6));
	
	if((i->feedback & 1) == 1 && allvolume != 0)
		lds_setregs(0x40 + regnum, ((c->volmod & 0xc0) | (((c->volmod & 0x3f) * allvolume) >> 8)) ^ 0x3f);
	else
		lds_setregs(0x40 + regnum, c->volmod ^ 0x3f);
	lds_setregs(0x60 + regnum, i->mod_ad);
	lds_setregs(0x80 + regnum, i->mod_sr);
	lds_setregs(0xe0 + regnum, i->mod_wave);
	
	/* Set carrier registers */
	lds_setregs(0x23 + regnum, i->car_misc);
	volcalc = i->car_vol;
	if(!c->nextvol)
		c->volcar = volcalc;
	else
		c->volcar = (volcalc & 0xc0) | ((((volcalc & 0x3f) * c->nextvol) >> 6));
	
	if(allvolume)
		lds_setregs(0x43 + regnum, ((c->volcar & 0xc0) | (((c->volcar & 0x3f) * allvolume) >> 8)) ^ 0x3f);
	else
		lds_setregs(0x43 + regnum, c->volcar ^ 0x3f);
	lds_setregs(0x63 + regnum, i->car_ad);
	lds_setregs(0x83 + regnum, i->car_sr);
	lds_setregs(0xe3 + regnum, i->car_wave);
	lds_setregs(0xc0 + channel_number, i->feedback);
	lds_setregs_adv(0xb0 + channel_number, 0xdf, 0);		/* key off */
	
	freq = frequency[tunehigh % (12 * 16)];
	octave = tunehigh / (12 * 16) - 1;
	if(!i->glide) {
		if(!i->portamento || !c->lasttune) {
			lds_setregs(0xa0 + channel_number, freq & 0xff);
			lds_setregs(0xb0 + channel_number, (octave << 2) + 0x20 + (freq >> 8));
			c->lasttune = c->gototune = tunehigh;
		} else {
			c->gototune = tunehigh;
			c->portspeed = i->portamento;
			lds_setregs_adv(0xb0 + channel_number, 0xdf, 0x20);	/* key on */
		}
	} else {
		lds_setregs(0xa0 + channel_number, freq & 0xff);
		lds_setregs(0xb0 + channel_number, (octave << 2) + 0x20 + (freq >> 8));
		c->lasttune = tunehigh;
		c->gototune = tunehigh + ((i->glide + 0x80) & 0xff) - 0x80;	/* set destination */
		c->portspeed = i->portamento;
	}
	
	if(!i->vibrato)
		c->vibwait = c->vibspeed = c->vibrate = 0;
	else {
		c->vibwait = i->vibdelay;
		/* PASCAL:    c->vibspeed = ((i->vibrato >> 4) & 15) + 1; */
		c->vibspeed = (i->vibrato >> 4) + 2;
		c->vibrate = (i->vibrato & 15) + 1;
	}
	
	if(!(c->trmstay & 0xf0)) {
		c->trmwait = (i->tremwait & 0xf0) >> 3;
		/* PASCAL:    c->trmspeed = (i->mod_trem >> 4) & 15; */
		c->trmspeed = i->mod_trem >> 4;
		c->trmrate = i->mod_trem & 15;
		c->trmcount = 0;
	}
	
	if(!(c->trmstay & 0x0f)) {
		c->trcwait = (i->tremwait & 15) << 1;
		/* PASCAL:    c->trcspeed = (i->car_trem >> 4) & 15; */
		c->trcspeed = i->car_trem >> 4;
		c->trcrate = i->car_trem & 15;
		c->trccount = 0;
	}
	
	c->arp_size = i->arpeggio & 15;
	c->arp_speed = i->arpeggio >> 4;
	memcpy(c->arp_tab, i->arp_tab, 12);
	c->keycount = i->keyoff;
	c->nextvol = c->glideto = c->finetune = c->vibcount = c->arp_pos = c->arp_count = 0;
}

