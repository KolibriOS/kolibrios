#ifndef INCLUDE_BENCH_H
#define INCLUDE_BENCH_H
//========================================================//
//  KolibriMark - benchmark engine                        //
//  Test registry, timing, scoring, system info.          //
//  (No UI here - see kbench.c)                            //
//========================================================//

#define SECT_CPU   0
#define SECT_GPU   1
#define SECT_DISK  2
#define SECT_NUM   3

#define MAX_TESTS  48

//#define BB_CALIB           // uncomment to add the calibration block to the report

#define BB_REVISION 1          // version of the tests + REF_ values, shown in the report
#define BB_SKIP     0xFFFFFFFF // test cannot run here: no score, out of the geomean

// ---- shared work buffers ----
#define GCV_X 0          // graphics canvas = window body (WIN_W/H tied to these)
#define GCV_Y 42
#define GCV_W 455        // wide enough for 3 columns of checkboxes
#define GCV_H 240

#define CPY_BYTES 2097152    // 2 MB: copy/latency working set (> retro-era caches)

// buf_a: copy source, hash input, disk-write source (content never matters)
// buf_b: copy dest, sieve + pointer-chase workspace (each test re-inits it)
dword buf_a, buf_b, buf_canvas;

// ---- parallel-array registry (modular: RegisterTest appends) ----
dword t_name [MAX_TESTS];   // -> name string
dword t_unit [MAX_TESTS];   // -> unit string, e.g. "MB/s"
dword t_fn   [MAX_TESTS];   // -> test function (returns metric x100)
dword t_ref  [MAX_TESTS];   // reference metric x100 that scores 1000
dword t_raw  [MAX_TESTS];   // last measured metric x100
dword t_score[MAX_TESTS];   // last computed score
byte  t_sect [MAX_TESTS];
byte  t_done [MAX_TESTS];   // 0 = not run, 1 = has a result, 2 = skipped
int   t_count = 0;

dword sect_score[SECT_NUM]; // aggregate score per section

void RegisterTest(dword sect, name, unit, ref, fn)
{
	t_sect [t_count] = sect;
	t_name [t_count] = name;
	t_unit [t_count] = unit;
	t_ref  [t_count] = ref;
	t_fn   [t_count] = fn;
	t_raw  [t_count] = 0;
	t_score[t_count] = 0;
	t_done [t_count] = 0;
	t_count++;
}

byte SectionRan(dword sect)      // true if any test here produced a result
{
	int i;
	for (i=0; i<t_count; i++) if (t_sect[i]==sect) && (t_done[i]==1) return 1;
	return 0;
}

// ---- 64-bit-safe  a*b/c  ----
dword muldiv(dword a, b, c)
{
	if (c<1) c = 1;
	EAX = a;
	ECX = b;
	$mul ecx
	ECX = c;
	$div ecx
}

// ---- integer log2 / exp2 in 16.16 fixed point ----
// Used for the geometric mean of section scores: with an arithmetic mean
// one outlier test (VRAM write spans 2 orders of magnitude between WC and
// non-WC machines) would dominate the whole section.
// log2(1+f) ~ f + 0.343*f*(1-f), error < 0.5% after the exp2 round-trip.
dword pw2tab[32];               // 2^n table (C-- has no variable shifts)

dword ilog2_16(dword s)         // log2(s) in 16.16, s >= 1
{
	dword e, m, x, c;
	if (s < 2) return 0;
	e = 0;  m = s;
	while (m > 1) { m = m / 2;  e++; }
	x = muldiv(s, 65536, pw2tab[e]);
	x = x - 65536;                       // fractional mantissa, 16.16
	c = 65536 - x;
	c = muldiv(x, c, 65536);
	c = muldiv(c, 22479, 65536);         // 0.343 in 16.16
	x = x + c;
	e = e << 16;
	return e + x;
}

dword iexp2_16(dword v)         // 2^(v/65536), inverse of ilog2_16
{
	dword e, f, c, m;
	e = v >> 16;
	f = v & 0xFFFF;
	c = 65536 - f;
	c = muldiv(f, c, 65536);
	c = muldiv(c, 22479, 65536);
	f = f - c;
	m = 65536 + f;                       // 2^frac in 16.16
	if (e > 30) return 0xFFFFFFFF;
	return muldiv(m, pw2tab[e], 65536);
}

// ---- timing (system fn 26.9: 1/100 s counter) ----
dword bench_t0;
void  BenchBegin() { bench_t0 = GetStartTime(); }
dword BenchTicks()             // elapsed 1/100 s, min 1
{
	dword e = GetStartTime() - bench_t0;
	if (e < 1) e = 1;
	return e;
}
// count = amount processed in DISPLAY units; returns units/sec x100
dword PerSecX100(dword count)
{
	return muldiv(count, 10000, BenchTicks());
}

// ---- call a registered test by pointer, return its metric x100 ----
dword CallFn(dword fn)
{
	ESI = fn;
	$call esi
}

// ---- run one registered test ----
void RunOne(int i)
{
	dword raw = CallFn(t_fn[i]);
	if (raw == BB_SKIP) {
		t_raw[i]   = 0;
		t_score[i] = 0;
		t_done[i]  = 2;
		return;
	}
	t_raw[i]   = raw;
	t_score[i] = muldiv(raw, 1000, t_ref[i]);
	t_done[i]  = 1;
}

// ---- allocate shared buffers + seed with data ----
byte BenchAllocBuffers()        // 0 = out of memory
{
	dword i, n;
	buf_a      = malloc(CPY_BYTES);
	buf_b      = malloc(CPY_BYTES);
	buf_canvas = malloc(GCV_W*GCV_H*3);
	if (!buf_a) || (!buf_b) || (!buf_canvas) return 0;
	i = 0;
	while (i < CPY_BYTES) {
		ESDWORD[buf_a+i] = i;
		i += 4;
	}
	n = GCV_W*GCV_H;  n = n*3;
	for (i=0; i<n; i++) DSBYTE[buf_canvas+i] = i;   // gradient BBGGRR
	pw2tab[0] = 1;
	for (i=1; i<32; i++) { n = pw2tab[i-1];  pw2tab[i] = n + n; }
	return 1;
}

//======================= system info =======================//
dword sys_cpu_mhz;
dword sys_ram_mb;
dword sys_bpp;              // current video mode bits per pixel

// re-read screen resolution + bpp (they change on a mode switch)
void RefreshScreen()
{
	dword d;
	EAX = 14;  $int 0x40         // screen size: (w-1)<<16 + (h-1)
	d = EAX;
	screen.h = d & 0xFFFF;  screen.h++;
	screen.w = d >> 16;     screen.w++;
	EAX = 61;  EBX = 2;  $int 0x40   // fn 61.2 = bits per pixel
	sys_bpp = EAX;
}

// RAM + CPU clock (fn 18.5). No CPUID/RDTSC -> fully 386-compatible (no
// "CPU required: Pentium"). The processor is just labelled "CPU" in the UI.
void GetSysInfo()
{
	dword hz;
	sys_ram_mb = GetTotalRAM() / 1024;
	EAX = 18;  EBX = 5;  $int 0x40;      // fn 18.5 = CPU clock rate, Hz
	hz = EAX;
	sys_cpu_mhz = hz / 1000000;
}

#endif
