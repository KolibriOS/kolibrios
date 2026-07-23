#ifndef INCLUDE_TESTS_GPU_H
#define INCLUDE_TESTS_GPU_H
//========================================================//
//  Graphics (2D) test module.                            //
//  Tests draw into the window canvas (GCV_* region).      //
//========================================================//

#define REF_FILL    20000     // 200.00 MPix/s
#define REF_BLIT    10000     // 100.00 MPix/s
#define REF_READ    5000      //  50.00 MPix/s  (VRAM read is slow)
#define REF_VWRITE  100000    // 1000.00 MB/s   (direct LFB write, WC-dependent)
#define REF_LINE    50000     // 500.00 Kline/s
#define REF_TEXT    30000     // 300.00 Kchar/s

char gpu_text_str[] = "BirdBench benchmark 0123456789ab";

void DrawLine38(dword x1,y1,x2,y2,color)
{
	EAX = 38;
	EBX = x1<<16+x2;
	ECX = y1<<16+y2;
	EDX = color;
	$int 0x40;
}

//--- Fill rate: solid bars (fn 13) ---
dword t_gpu_fill()
{
	dword c=0, col=0, mp;
	BenchBegin();
	do {
		DrawBar(GCV_X, GCV_Y, GCV_W, GCV_H, col);
		col += 0x0A0C0E;
		col = col & 0xFFFFFF;   // bit 31 = gradient-fill flag (fn13 0x80RRGGBB)!
		c++;
	} while (BenchTicks() < 100);
	mp = muldiv(c, GCV_W*GCV_H, 1000000);   // total mega-pixels
	return PerSecX100(mp);                   // MPix/s
}

//--- Blit rate: 24bpp image (fn 7) ---
dword t_gpu_blit()
{
	dword c=0, mp;
	BenchBegin();
	do { PutImage(GCV_X, GCV_Y, GCV_W, GCV_H, buf_canvas);  c++; }
	while (BenchTicks() < 100);
	mp = muldiv(c, GCV_W*GCV_H, 1000000);
	return PerSecX100(mp);                   // MPix/s
}

//--- Line rate: 1000 lines (fn 38) per unit ---
dword t_gpu_lines()
{
	dword c=0, i, col=0x3388CC;
	dword x1,y1,x2,y2;
	BenchBegin();
	do {
		for (i=0; i<1000; i++) {
			x1 = i*7;   x1 = x1 % GCV_W;  x1 = x1 + GCV_X;
			y1 = i*13;  y1 = y1 % GCV_H;  y1 = y1 + GCV_Y;
			x2 = i*29;  x2 = x2 % GCV_W;  x2 = x2 + GCV_X;
			y2 = i*23;  y2 = y2 % GCV_H;  y2 = y2 + GCV_Y;
			DrawLine38(x1,y1,x2,y2,col);
			col += 0x0A1420;
			col = col & 0xFFFFFF;   // bit 24 = inversed-line flag (fn38 0x01xxxxxx)!
		}
		c++;                  // 1000 lines = 1 Kline
	} while (BenchTicks() < 100);
	return PerSecX100(c);         // Kline/s
}

//--- Text rate: WriteText (fn 4), both kernel fonts (6x9 + 8x16) ---
// Subpixel smoothing is forced for the duration (the heaviest glyph
// path), then the user's setting is restored.
dword t_gpu_text()
{
	dword c=0, i, y, len, kc, sm;
	len = strlen(#gpu_text_str);
	EAX = 48;  EBX = 9;  $int 0x40;               // SSF_GET_FONT_SMOOTH
	sm = EAX;
	EAX = 48;  EBX = 10;  ECX = 2;  $int 0x40;    // SSF_SET_FONT_SMOOTH: subpixel
	BenchBegin();
	do {
		for (i=0; i<25; i++) {
			y = i*11;  y = y % GCV_H;  y = y + GCV_Y;
			WriteText(GCV_X, y, 0x80, 0x101010, #gpu_text_str);   // 6x9
		}
		for (i=0; i<25; i++) {
			y = i*17;  y = y % GCV_H;  y = y + GCV_Y;
			WriteText(GCV_X, y, 0x90, 0x101010, #gpu_text_str);   // 8x16
		}
		c++;                  // 50 * len chars
	} while (BenchTicks() < 100);
	EAX = 48;  EBX = 10;  ECX = sm;  $int 0x40;   // restore user's smoothing
	kc = muldiv(c, 50*len, 1000);       // total kilo-chars
	return PerSecX100(kc);               // Kchar/s
}

//--- VRAM read-back: read screen area to RAM (fn 36) ---
dword t_gpu_read()
{
	dword c=0, mp, sx, sy;
	proc_info p;
	GetProcessInfo(#p, SelfInfo);        // fn 36 uses absolute screen coords
	sx = p.left;  sx = sx + 5;        sx = sx + GCV_X;
	sy = p.top;   sy = sy + skin_h;   sy = sy + GCV_Y;
	BenchBegin();
	do {
		CopyScreen(buf_canvas, sx, sy, GCV_W, GCV_H);   // 24bpp -> buf
		c++;
	} while (BenchTicks() < 100);
	mp = muldiv(c, GCV_W*GCV_H, 1000000);
	return PerSecX100(mp);               // MPix/s
}

//--- direct VRAM write: rep stosd into the LFB via the GS selector ---
// The kernel gives apps a writable GS whose base = LFBAddress (see kernel.asm:
// "Set base of graphic segment to linear address of LFB"). rep stosd needs
// ES:EDI, so we copy GS into ES for the duration, then restore it. No window
// clipping here - we fill only our own on-screen rectangle, scanline by scanline.
// GS segment limit via LSL (unprivileged). If the video mode has no LFB
// (banked VGA), the selector window is tiny and writing past it would #GP.
dword gs_limit()
{
	asm {
		XOR EAX, EAX
		MOV AX, GS
		LSL EAX, EAX
	}
}

inline fastcall vram_fill_gs(EDI, ESI, EDX, ECX)   // ofs, dwords/row, row-advance, rows
{
	asm {
		push es
		push ebx
		mov  ebx, ecx           // ebx = row count
		mov  ax, gs
		mov  es, ax             // es -> LFB
		cld
	kvw1:
		mov  ecx, esi           // dwords per scanline
		mov  eax, 0x00336699    // fill pattern
		rep  stosd
		add  edi, edx           // -> next scanline
		dec  ebx
		jnz  kvw1
		pop  ebx
		pop  es
	}
}

dword t_gpu_vramwrite()
{
	dword c=0, sx, sy, cols, rows, pitch, bpp, bpp_b, off, rowbytes, rowdw, rowadv, bpf, mb, t, lim;
	proc_info p;
	GetProcessInfo(#p, SelfInfo);
	EAX = 61;  EBX = 3;  $int 0x40;  pitch = EAX;   // bytes per scanline
	EAX = 61;  EBX = 2;  $int 0x40;  bpp   = EAX;   // bits per pixel
	bpp_b = bpp + 7;  bpp_b = bpp_b / 8;
	if (bpp_b < 1) bpp_b = 4;
	sx = p.left;  sx = sx + 5;        sx = sx + GCV_X;
	sy = p.top;   sy = sy + skin_h;   sy = sy + GCV_Y;
	// clamp to the screen - writing past the LFB would #GP (small screens!)
	if (sx >= screen.w) return BB_SKIP;
	if (sy >= screen.h) return BB_SKIP;
	cols = GCV_W;  t = sx + cols;  if (t > screen.w) cols = screen.w - sx;
	rows = GCV_H;  t = sy + rows;  if (t > screen.h) rows = screen.h - sy;
	rowbytes = cols * bpp_b;
	if (pitch < rowbytes) pitch = rowbytes;
	// stosd moves whole dwords: trim the row to a multiple of 4 so rowadv and
	// the reported byte count match what was actually written (at 24/16 bpp
	// rowbytes is not divisible by 4 and every row would drift by 1-2 bytes)
	rowdw    = rowbytes / 4;
	rowbytes = rowdw * 4;
	rowadv   = pitch - rowbytes;
	t   = sx * bpp_b;
	off = sy * pitch;  off = off + t;
	// last byte we will touch must be inside the GS segment (no-LFB safety)
	t = rows - 1;  t = t * pitch;  t = t + off;  t = t + rowbytes;  t = t - 1;
	lim = gs_limit();
	if (t > lim) return BB_SKIP;
	BenchBegin();
	do {
		vram_fill_gs(off, rowdw, rowadv, rows);
		c++;
	} while (BenchTicks() < 100);
	bpf = rowbytes * rows;                // bytes per frame
	mb  = muldiv(c, bpf, 1048576);        // total MB written (MiB, like the disk tests)
	return PerSecX100(mb);                // MB/s
}

void Register_GPU()
{
	RegisterTest(SECT_GPU, "Fill Rate",      "MPix/s",  REF_FILL,   #t_gpu_fill);
	RegisterTest(SECT_GPU, "Blit (f7)",      "MPix/s",  REF_BLIT,   #t_gpu_blit);
	RegisterTest(SECT_GPU, "VRAM Read (f36)","MPix/s",  REF_READ,   #t_gpu_read);
	RegisterTest(SECT_GPU, "VRAM Write (gs)","MB/s",    REF_VWRITE, #t_gpu_vramwrite);
	RegisterTest(SECT_GPU, "Lines (f38)",    "Kline/s", REF_LINE,   #t_gpu_lines);
	RegisterTest(SECT_GPU, "Text (f4)",      "Kchar/s", REF_TEXT,   #t_gpu_text);
}

#endif
