// DGen v1.29

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "md.h"
#include "rc-vars.h"

// REMEMBER NOT TO USE ANY STATIC variables, because they
// will exist thoughout ALL megadrives!
int md::myfm_write(int a, int v, int md)
{
	int sid = 0;
	int pass = 1;

	(void)md;
	a &= 3;
	sid = ((a & 0x02) >> 1);
	if ((a & 0x01) == 0) {
		fm_sel[sid] = v;
		goto end;
	}
#ifdef WITH_VGMDUMP
	vgm_dump_ym2612(sid, fm_sel[sid], v);
#endif
	if (fm_sel[sid] == 0x2a) {
		dac_submit((uint8_t)v);
		pass = 0;
	}
	if (fm_sel[sid] == 0x2b) {
		dac_enable((uint8_t)v);
		pass = 0;
	}
	if (fm_sel[sid] == 0x27) {
		unsigned int now = frame_usecs();

		if ((v & 0x01) && ((fm_reg[0][0x27] & 0x01) == 0)) {
			// load timer A
			fm_ticker[0] = 0;
			fm_ticker[1] = now;
		}
		if ((v & 0x02) && ((fm_reg[0][0x27] & 0x02) == 0)) {
			// load timer B
			fm_ticker[2] = 0;
			fm_ticker[3] = now;
		}
		// (v & 0x04) enable/disable timer A
		// (v & 0x08) enable/disable timer B
		if (v & 0x10) {
			// reset overflow A
			fm_tover &= ~0x01;
			v &= ~0x10;
			fm_reg[0][0x27] &= ~0x10;
		}
		if (v & 0x20) {
			// reset overflow B
			fm_tover &= ~0x02;
			v &= ~0x20;
			fm_reg[0][0x27] &= ~0x20;
		}
	}
	// stash all values
	fm_reg[sid][(fm_sel[sid])] = v;
end:
	if (pass) {
		YM2612Write(0, a, v);
		if (dgen_mjazz) {
			YM2612Write(1, a, v);
			YM2612Write(2, a, v);
		}
	}
	return 0;
}

int md::myfm_read(int a)
{
	fm_timer_callback();
	return (fm_tover | (YM2612Read(0, (a & 3)) & ~0x03));
}

int md::mysn_write(int d)
{
#ifdef WITH_VGMDUMP
	vgm_dump_sn76496(d);
#endif
	SN76496Write(0, d);
	return 0;
}

int md::fm_timer_callback()
{
	// periods in microseconds for timers A and B
	int amax = (18 * (1024 -
			  (((fm_reg[0][0x24] << 2) |
			    (fm_reg[0][0x25] & 0x03)) & 0x3ff)));
	int bmax = (288 * (256 - (fm_reg[0][0x26] & 0xff)));
	unsigned int now = frame_usecs();

	if ((fm_reg[0][0x27] & 0x01) && ((now - fm_ticker[1]) > 0)) {
		fm_ticker[0] += (now - fm_ticker[1]);
		fm_ticker[1] = now;
		if (fm_ticker[0] >= amax) {
			if (fm_reg[0][0x27] & 0x04)
				fm_tover |= 0x01;
			fm_ticker[0] -= amax;
		}
	}
	if ((fm_reg[0][0x27] & 0x02) && ((now - fm_ticker[3]) > 0)) {
		fm_ticker[2] += (now - fm_ticker[3]);
		fm_ticker[3] = now;
		if (fm_ticker[2] >= bmax) {
			if (fm_reg[0][0x27] & 0x08)
				fm_tover |= 0x02;
			fm_ticker[2] -= bmax;
		}
	}
	return 0;
}

void md::fm_reset()
{
	memset(fm_sel, 0, sizeof(fm_sel));
	fm_tover = 0x00;
	memset(fm_ticker, 0, sizeof(fm_ticker));
	memset(fm_reg, 0, sizeof(fm_reg));
	YM2612ResetChip(0);
	if (dgen_mjazz) {
		YM2612ResetChip(1);
		YM2612ResetChip(2);
	}
	SN76496_init(0,
		     (((pal) ? PAL_MCLK : NTSC_MCLK) / 15),
		     dgen_soundrate, 16);
}

void md::dac_init()
{
	dac_enabled = true;
	dac_len = 0;
#ifndef NDEBUG
	memset(dac_data, 0xff, sizeof(dac_data));
#endif
}

static const struct {
	unsigned int samples;
	unsigned int usecs;
} per_frame[2] = {
	{ (44100 / 60), (1000000 / 60) },
	{ (44100 / 50), (1000000 / 50) },
};

void md::dac_submit(uint8_t d)
{
	unsigned int usecs;
	unsigned int index;
	unsigned int i;

	if (!dac_enabled)
		return;
	if (dac_len == elemof(dac_data))
		return;
	usecs = frame_usecs();
	index = ((usecs << 10) /
		 ((per_frame[pal].usecs << 10) /
		  elemof(dac_data)));
	if (index >= elemof(dac_data))
		return;
	dac_data[index] = d;
	if (dac_len)
		d = dac_data[dac_len - 1];
	for (i = dac_len; (i < index); ++i)
		dac_data[i] = d;
	dac_len = (index + 1);
}

void md::dac_enable(uint8_t d)
{
	dac_enabled = ((d & 0x80) >> 7);
}

#ifdef WITH_VGMDUMP

void md::vgm_dump_ym2612(uint8_t a1, uint8_t reg, uint8_t data)
{
	if (vgm_dump) {
		uint8_t buf[] = { (uint8_t)(0x52 + a1), reg, data };

		fwrite(buf, sizeof(buf), 1, vgm_dump_file);
		if ((a1 == 0) && (reg == 0x2a)) {
			unsigned int usecs = frame_usecs();
			unsigned int samples;
			unsigned int diff;

			if (usecs > per_frame[pal].usecs)
				usecs = per_frame[pal].usecs;
			samples = ((usecs *
				    ((per_frame[pal].samples << 20) /
				     per_frame[pal].usecs)) >> 20);
			diff = (samples - vgm_dump_dac_samples);
			if ((diff > 0) && (diff <= 16)) {
				fputc((0x70 + (diff - 1)), vgm_dump_file);
				vgm_dump_dac_wait += diff;
			}
			vgm_dump_dac_samples = samples;
		}
	}
}

void md::vgm_dump_sn76496(uint8_t data)
{
	if (vgm_dump) {
		uint8_t buf[] = { 0x50, data };

		fwrite(buf, sizeof(buf), 1, vgm_dump_file);
	}
}

void md::vgm_dump_frame()
{
	unsigned int max = per_frame[pal].samples;

	if (!vgm_dump)
		return;
	if (vgm_dump_dac_wait < max) {
		uint8_t buf[] = { 0x61, 0x00, 0x00 };
		uint16_t tmp = h2le16(max - vgm_dump_dac_wait);

		memcpy(&buf[1], &tmp, sizeof(tmp));
		fwrite(buf, sizeof(buf), 1, vgm_dump_file);
	}
	vgm_dump_samples_total += max;
	vgm_dump_dac_wait = 0;
	vgm_dump_dac_samples = 0;
}

// Generate VGM 1.70 header as defined by:
// http://www.smspower.org/uploads/Music/vgmspec170.txt
int md::vgm_dump_start(const char *name)
{
	uint8_t ym2612_buf[0x200];
	uint8_t buf[0x100] = { 0 };
	union {
		uint32_t u32;
		uint16_t u16;
	} tmp;
	unsigned int i;
	int err;

	if (vgm_dump == true)
		vgm_dump_stop();
	vgm_dump_file = dgen_fopen("vgm", name, DGEN_WRITE);
	if (vgm_dump_file == NULL)
		return -1;
	// 0x00: file identifier.
	memcpy(&buf[0x00], "Vgm ", 4);
	// 0x04: EoF offset. Not known yet.
	// 0x08: version number (1.70).
	tmp.u32 = 0x0170;
	memcpy(&buf[0x08], &tmp.u32, 4);
	// 0x0c: SN76489 (PSG) clock.
	tmp.u32 = h2le32(clk0);
	memcpy(&buf[0x0c], &tmp.u32, 4);
	// 0x18: total # samples. Not known yet.
	// 0x24: rate.
	tmp.u32 = h2le32(vhz);
	memcpy(&buf[0x24], &tmp.u32, 4);
	// 0x28: SN76489 (PSG) feedback.
	tmp.u16 = h2le16(0x0009);
	memcpy(&buf[0x28], &tmp.u16, 2);
	// 0x2a: SN76489 shift register width.
	buf[0x2a] = 16;
	// 0x2b: SN76489 flags.
	buf[0x2b] = 0x00;
	// 0x2c: YM2612 clock.
	tmp.u32 = h2le32(clk1);
	memcpy(&buf[0x2c], &tmp.u32, 4);
	// 0x34: VGM data offset.
	tmp.u32 = h2le32(sizeof(buf) - 0x34);
	memcpy(&buf[0x34], &tmp.u32, 4);
	// Dump VGM header.
	if (fwrite(buf, sizeof(buf), 1, vgm_dump_file) != 1)
		goto error;
	// Dump YM2612 registers directly.
	YM2612_dump(0, ym2612_buf);
	// Timers.
	{
		uint8_t buf[] = {
			0x52, 0x24, (uint8_t)fm_reg[0][0x24],
			0x52, 0x25, (uint8_t)fm_reg[0][0x25],
			0x52, 0x26, (uint8_t)fm_reg[0][0x26],
			0x52, 0x27, (uint8_t)fm_reg[0][0x27],
		};

		if (fwrite(buf, sizeof(buf), 1, vgm_dump_file) != 1)
			goto error;
	}
	// DAC.
	{
		uint8_t buf[] = { 0x52, 0x2b, (uint8_t)(dac_enabled << 7) };

		if (fwrite(buf, sizeof(buf), 1, vgm_dump_file) != 1)
			goto error;
	}
	// FM CH1-CH3.
	for (i = 0x30; (i != 0x9e); ++i) {
		uint8_t buf[] = {
			0x52, (uint8_t)i, ym2612_buf[i],
			0x53, (uint8_t)i, ym2612_buf[i | 0x100],
		};

		if (fwrite(buf, sizeof(buf), 1, vgm_dump_file) != 1)
			goto error;
	}
	// FM CH4-CH6.
	for (i = 0xb0; (i != 0xb6); ++i) {
		uint8_t buf[] = {
			0x52, (uint8_t)i, ym2612_buf[i],
			0x53, (uint8_t)i, ym2612_buf[i | 0x100],
		};

		if (fwrite(buf, sizeof(buf), 1, vgm_dump_file) != 1)
			goto error;
	}
	vgm_dump_samples_total = 0;
	vgm_dump_dac_wait = 0;
	vgm_dump_dac_samples = 0;
	vgm_dump = true;
	return 0;
error:
	err = errno;
	fclose(vgm_dump_file);
	vgm_dump_file = NULL;
	errno = err;
	return -1;
}

void md::vgm_dump_stop()
{
	long pos;
	uint32_t tmp;

	if (!vgm_dump)
		return;
	// Append end of sound data.
	fputc(0x66, vgm_dump_file);
	pos = ftell(vgm_dump_file);
	// Fill EoF offset.
	fseek(vgm_dump_file, 0x04, SEEK_SET);
	tmp = h2le32(pos - 4);
	fwrite(&tmp, sizeof(tmp), 1, vgm_dump_file);
	// Fill total number of samples.
	fseek(vgm_dump_file, 0x18, SEEK_SET);
	tmp = h2le32(vgm_dump_samples_total);
	fwrite(&tmp, sizeof(tmp), 1, vgm_dump_file);
	fclose(vgm_dump_file);
	vgm_dump_file = NULL;
	vgm_dump_samples_total = 0;
	vgm_dump_dac_wait = 0;
	vgm_dump_dac_samples = 0;
	vgm_dump = false;
}

#endif // WITH_VGMDUMP
