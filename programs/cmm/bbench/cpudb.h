#ifndef INCLUDE_CPUDB_H
#define INCLUDE_CPUDB_H
//========================================================//
//  CPU comparison database  --  EDIT ME                  //
//                                                        //
//  One line per reference CPU in cpudb_init(), copied straight   //
//  from the Calibration block of a run on that machine. Entries  //
//  measured under another BB_REVISION are hidden.                //
//  The values below are placeholders - replace them.             //
//========================================================//

#define CPUDB_MAX 24
dword cpudb_name [CPUDB_MAX];
dword cpudb_mhz  [CPUDB_MAX];
dword cpudb_score[CPUDB_MAX];
int   cpudb_count = 0;

// entries from another revision are dropped: their numbers mean something else
void cpudb_add(dword name, mhz, score, rev)
{
	if (cpudb_count >= CPUDB_MAX) return;
	if (rev != BB_REVISION) return;
	cpudb_name [cpudb_count] = name;
	cpudb_mhz  [cpudb_count] = mhz;
	cpudb_score[cpudb_count] = score;
	cpudb_count++;
}

void cpudb_init()
{
	cpudb_add("Intel Core i5-2400",  3100, 1600, 1);
	cpudb_add("AMD Athlon XP 2500+", 1833,  360, 1);
	cpudb_add("Intel Pentium 4",     2400,  300, 1);
	cpudb_add("Intel Pentium III",   1000,  180, 1);
	cpudb_add("Intel Atom N270",     1600,  420, 1);
	cpudb_add("VIA C7",              1200,  150, 1);
}

#endif
