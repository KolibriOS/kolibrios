#ifndef INCLUDE_TESTS_CPU_H
#define INCLUDE_TESTS_CPU_H
//========================================================//
//  CPU test module.                                      //
//  Each test returns its metric x100 (two decimals).      //
//  To add a test: write t_cpu_xxx() then RegisterTest().  //
//========================================================//

// reference metric x100 that should score 1000 (calibration)
#define REF_INT    5000       //  50.00 MOps/s
#define REF_FLOAT  2000       //  20.00 MFLOP/s
#define REF_MEM    80000      // 800.00 MB/s
#define REF_LAT    1000       //  10.00 Macc/s (= 100 ns per access)
#define REF_SIEVE  3000       //  30.00 Mcell/s
#define REF_HASH   10000      // 100.00 MB/s

#define CPY_MB     2          // CPY_BYTES in whole MiB (see bbench.h)
#define LAT_N      524288     // nodes in the 2 MB pointer chain

dword cpu_sink;
float cpu_fsink;
dword lat_rnd;

// forward rep movsd block copy. NB: lib memmov() copies BACKWARD when
// dst>src (std; rep movsd), which never hits the fast-string path on
// modern CPUs and would understate them - so we do our own, forward.
inline fastcall copy_fwd(EDI, ESI, ECX)      // dst, src, bytes (mult of 4)
{
	asm {
		CLD
		SHR ECX, 2
		REP MOVSD
	}
}

void sieve1m(dword buf)
{
	dword i, j;
	EDI = buf;
	EAX = 0;
	ECX = 262144;                    // 1 MB / 4
	asm {
		CLD
		REP STOSD
	}
	for (i=2; i*i<1000000; i++) {
		if (DSBYTE[buf+i]==0) {
			j = i*i;
			while (j<1000000) { DSBYTE[buf+j]=1; j += i; }
		}
	}
}

dword hash1mb(dword buf)
{
	dword i, h;
	h = 2166136261;
	for (i=0; i<1048576; i++) {
		h = h ^ DSBYTE[buf+i];
		h = h * 16777619;
	}
	return h;
}

// build one random cycle of byte-offsets over buf (Sattolo shuffle):
// following p = [buf+p] visits every node in random order - defeats
// both the prefetcher and the cache -> pure dependent-load latency.
void lat_build(dword buf)
{
	dword i, j, t, ai, aj;
	for (i=0; i<LAT_N; i++) { ai = i*4;  ESDWORD[buf+ai] = ai; }
	lat_rnd = 0x1A2B3C4D;            // fixed seed: same chain on every machine
	i = LAT_N;
	while (i > 1) {
		i = i - 1;
		lat_rnd = lat_rnd * 1103515245;  lat_rnd = lat_rnd + 12345;
		j = lat_rnd >> 8;  j = j % i;
		ai = i*4;  ai = ai + buf;
		aj = j*4;  aj = aj + buf;
		t = ESDWORD[ai];  ESDWORD[ai] = ESDWORD[aj];  ESDWORD[aj] = t;
	}
}

//--- Integer: 4 independent ALU chains, ~1e6 ops per unit ---
// Interleaved so superscalar CPUs (P5+) can overlap them; a strictly
// serial chain would only measure latency and hide pipelining.
dword t_cpu_int()
{
	dword c=0, i, x1, x2, x3, x4;
	BenchBegin();
	do {
		x1 = 0x12345678;  x2 = 0x9E3779B9;  x3 = 0x01234567;  x4 = 0xABCDEF01;
		for (i=0; i<50000; i++) {
			x1 = x1 + i;          x2 = x2 + i;          x3 = x3 + i;          x4 = x4 + i;
			x1 = x1 ^ 0x55AA55AA; x2 = x2 ^ 0x33CC33CC; x3 = x3 ^ 0x0F0F0F0F; x4 = x4 ^ 0x5A5A5A5A;
			x1 = x1 * 3;          x2 = x2 * 3;          x3 = x3 * 3;          x4 = x4 * 3;
			x1 = x1 - i;          x2 = x2 - i;          x3 = x3 - i;          x4 = x4 - i;
			x1 = x1 + x1;         x2 = x2 + x2;         x3 = x3 + x3;         x4 = x4 + x4;
		}
		cpu_sink = x1;  cpu_sink = cpu_sink ^ x2;
		cpu_sink = cpu_sink ^ x3;  cpu_sink = cpu_sink ^ x4;
		c++;                  // 50000 * 20 = 1e6 ops
	} while (BenchTicks() < 100);
	return PerSecX100(c);         // MOps/s
}

//--- Floating point: 4 independent mul/add chains, ~1e6 flops per unit ---
// No division: one fdiv (15-40+ cycles) would dominate the whole chain
// and the test would just measure fdiv latency.
dword t_cpu_float()
{
	dword c=0, i;
	float a1, a2, a3, a4, b, k;
	BenchBegin();
	do {
		a1 = 1.0;  a2 = 1.1;  a3 = 1.2;  a4 = 1.3;
		b = 1.0000151;  k = 0.9999847;
		for (i=0; i<62500; i++) {
			a1 = a1 * b;  a2 = a2 * b;  a3 = a3 * b;  a4 = a4 * b;
			a1 = a1 + k;  a2 = a2 + k;  a3 = a3 + k;  a4 = a4 + k;
			a1 = a1 * k;  a2 = a2 * k;  a3 = a3 * k;  a4 = a4 * k;
			a1 = a1 - k;  a2 = a2 - k;  a3 = a3 - k;  a4 = a4 - k;
		}
		cpu_fsink = a1;  cpu_fsink = cpu_fsink + a2;
		cpu_fsink = cpu_fsink + a3;  cpu_fsink = cpu_fsink + a4;
		c++;                  // 62500 * 16 = 1e6 flops
	} while (BenchTicks() < 100);
	return PerSecX100(c);         // MFLOP/s
}

//--- Memory bandwidth: 2 MB forward copy per unit ---
dword t_cpu_mem()
{
	dword c=0;
	BenchBegin();
	do { copy_fwd(buf_b, buf_a, CPY_BYTES);  c += CPY_MB; } while (BenchTicks() < 100);
	return PerSecX100(c);         // MB/s
}

//--- Memory latency: serial pointer chase over 2 MB, 1e6 hops per unit ---
dword t_cpu_latency()
{
	dword c=0, k, p;
	lat_build(buf_b);
	BenchBegin();
	do {
		p = 0;
		for (k=0; k<250000; k++) {
			p = ESDWORD[buf_b+p];
			p = ESDWORD[buf_b+p];
			p = ESDWORD[buf_b+p];
			p = ESDWORD[buf_b+p];
		}
		cpu_sink = p;
		c++;                  // 250000 * 4 = 1e6 dependent loads
	} while (BenchTicks() < 100);
	return PerSecX100(c);         // Macc/s (100/result = ns per access)
}

//--- Prime sieve: 1e6 cells per unit ---
dword t_cpu_sieve()
{
	dword c=0;
	BenchBegin();
	do { sieve1m(buf_b);  c++; } while (BenchTicks() < 100);
	return PerSecX100(c);         // Mcell/s
}

//--- Hash throughput (FNV-1a): 1 MB per unit ---
dword t_cpu_hash()
{
	dword c=0;
	BenchBegin();
	do { cpu_sink = hash1mb(buf_a);  c++; } while (BenchTicks() < 100);
	return PerSecX100(c);         // MB/s
}

void Register_CPU()
{
	RegisterTest(SECT_CPU, "Integer",        "MOps/s",  REF_INT,   #t_cpu_int);
	RegisterTest(SECT_CPU, "Floating Point", "MFLOP/s", REF_FLOAT, #t_cpu_float);
	RegisterTest(SECT_CPU, "Memory Copy",    "MB/s",    REF_MEM,   #t_cpu_mem);
	RegisterTest(SECT_CPU, "Memory Latency", "Macc/s",  REF_LAT,   #t_cpu_latency);
	RegisterTest(SECT_CPU, "Prime Sieve",    "Mcell/s", REF_SIEVE, #t_cpu_sieve);
	RegisterTest(SECT_CPU, "Hash FNV-1a",    "MB/s",    REF_HASH,  #t_cpu_hash);
}

#endif
