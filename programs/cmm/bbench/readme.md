# BirdBench

A small, modular system benchmark for KolibriOS, styled after Geekbench.
Single-threaded (KolibriOS exposes no SMP to apps). 386-compatible
(no CPUID/RDTSC — the MHz value comes from sysfn 18.5).

## Sections

* **CPU** — Integer, Floating Point, Memory Copy, Memory Latency,
  Prime Sieve, Hash (FNV-1a).
  Integer/Float run **4 independent chains** so superscalar CPUs (P5+) can
  pair/overlap them — a strictly serial chain would only measure latency.
  Float has no division on purpose: one fdiv (15–40+ cycles) would dominate
  the chain and the test would degenerate into an fdiv-latency meter.
  Memory Copy is a **forward** `rep movsd` over 2 MB (lib `memmov` copies
  backward when dst>src, which never hits the fast-string path on modern
  CPUs). Memory Latency is a serial pointer chase over a 2 MB random cycle
  (Sattolo shuffle) — defeats prefetch and cache, measures dependent-load
  latency; `100 / (Macc/s) = ns` per access.
* **Graphics** — Fill (f13), Blit (f7), VRAM Read (f36), VRAM Write (gs:),
  Lines (f38), Text (f4).
  Fill/Blit = writes through the driver's blitter (may be HW-accelerated),
  VRAM Read (f36) = read-back, VRAM Write (gs:) = raw CPU `rep stosd` straight
  into the LFB via the GS selector — this one exposes write-combining (MTRR/PAT):
  no WC and it collapses ~10-50x. The report shows the current video mode —
  switch modes with the vidmode picker and re-run to compare VESA-LFB vs a
  native driver vs KMS at the same resolution.
  Before writing, the GS segment limit is checked with `lsl` — on banked-VGA
  modes (no LFB) the test skips instead of #GP-faulting. Color counters are
  masked to 24 bits: fn13 treats bit 31 as the gradient-fill flag and fn38
  treats bit 24 as the inversed-line flag — unmasked they silently switch the
  kernel to a different (slower) operation mid-test.
  Text draws **both kernel fonts** (6×9 and 8×16) and forces **subpixel font
  smoothing** for the duration (fn 48.9 saves the user's setting, fn 48.10
  sets/restores it) — the heaviest glyph path, so machines are compared on
  equal footing regardless of their smoothing setting.
* **Disk** — Sequential write / read (2 MB ops) + Random write / read
  (4 KB blocks at fixed-seed random offsets in an 8 MB file, via fn 70
  positioned I/O) + **File System** (create 10 files + 10 folders, delete
  them all — metadata op/s, fn 70.2/70.9/70.8; names are pre-built so no
  sprintf lands in the timed loop). Folder support is probed first: not every
  writable FS driver implements fn 70.9 (exFAT has no CreateFolder at all) —
  there the test degrades to files-only instead of scoring 0. Pick the target
  disk in the UI — the report's Disk header shows which disk was tested. All
  disk rates are MiB/s.
  Temp files are deleted when the run finishes.
  NB: KolibriOS apps can't drop the FS cache, so on a real disk the random
  numbers reflect the FS small-op path (not raw seek/IOPS), and on the RAM
  disk random ≈ sequential (no seek penalty).

The window is a **launcher**: tick the tests you want (per-test checkboxes,
per-section master checkbox, or All / None), pick a disk target, press **Run**.
Only ticked tests run; **Esc aborts** between tests. During a run the window
is forced always-on-top: the kernel only draws the visible parts of a window,
so an occluded window would inflate the Graphics scores. When a run finishes,
an HTML report is written to `/tmp0/1/bb_YYMMDD_HHMMSS.htm` (unique name) and
**auto-opened in WebView**.

The report (not the window) shows the results: per-section score, one row per
test with a colored bar (alternating shades) plus a gray/light-gray track, and
a **CPU compare** chart of your score vs the CPUs in `cpudb.h` (with a MHz
column; the compare bars line up with the test bars). Results are only
in the web page — clearer there than in the small window.

Each test reports a raw throughput (MB/s, MPix/s, ...) and a *score*
(1000 = the reference machine defined by the `REF_*` constants in the
module). The section score is the **geometric mean** of its ticked tests'
scores — a single outlier test (VRAM Write spans two orders of magnitude
between WC and non-WC machines) can't dominate the section the way it
would with an arithmetic mean.

## How to add a test

1. Open the section module (`tests_cpu.h`, `tests_gpu.h` or `tests_disk.h`).
2. Write a function returning the metric ×100 (two decimals):

   ```c
   dword t_cpu_mytest()
   {
       dword c = 0;
       BenchBegin();
       do { /* one unit of work */  c++; } while (BenchTicks() < 100);
       return PerSecX100(c);        // units/sec ×100
   }
   ```

   `c` counts how many *display units* you processed (1 MB, 1 Mpixel,
   1 Kline, ...). The framework turns that into a per-second rate.
3. Register it and give it a reference value:

   ```c
   #define REF_MYTEST 5000                 // 50.00 units/s -> score 1000
   RegisterTest(SECT_CPU, "My Test", "u/s", REF_MYTEST, #t_cpu_mytest);
   ```

That's it — it shows up in the list automatically.

## How to add a comparison CPU

Edit `cpudb.h`, add a line to `cpudb_init()`:

```c
cpudb_add("Intel Core 2 Duo E8400", 3000, 520);   // name, MHz, measured CPU score
```

NB: any change to the test definitions invalidates previously measured
db entries — re-measure them with the same binary you compare against.

## Build

Run `build.bat` (needs `../c--/c--.exe`). Output: `bbench`.

## Notes / calibration

* The `REF_*` constants are placeholders. Pick a machine as your baseline,
  run it, and set each `REF_*` to that machine's raw metric ×100 so it
  scores ~1000. Everything else scales relative to it.
* All "MB" are MiB (2^20) across CPU, VRAM Write and Disk tests.
* Random-I/O offsets and the latency chain use **fixed seeds** — every run
  and every machine gets the same access pattern (reproducibility).
* On machines with a large L3 the 2 MB working set of Memory Copy/Latency
  measures the cache, not RAM — unavoidable at sizes that still fit
  retro-era boxes; scores stay comparable within an era.
* Needs ~6 MB RAM (see `MEMSIZE`).
