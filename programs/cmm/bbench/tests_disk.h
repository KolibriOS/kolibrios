#ifndef INCLUDE_TESTS_DISK_H
#define INCLUDE_TESTS_DISK_H
//========================================================//
//  Disk test module.                                     //
//  Writes/reads a temp file via system fn 70.            //
//  Target directory = disk_dir (set from the UI disk      //
//  selector); falls back to the RAM disk /tmp0/1.         //
//  NB: sequential rewrite of one file mostly measures the //
//  FS/cache throughput, not raw platter speed.            //
//========================================================//

#define REF_DWRITE 5000       // 50.00 MB/s
#define REF_DREAD  5000
#define REF_RWRITE 1000       // 10.00 MB/s  (random 4K is much slower)
#define REF_RREAD  1000
#define REF_FS     50000      // 500.00 op/s (create/delete metadata ops)
#define FS_N       10         // files + folders created per unit
#define DISK_CHUNK CPY_BYTES  // 2 MB per op (reuses buf_a - content irrelevant)
#define DISK_MB    2
#define RND_FILE   8388608    // 8 MB test file for random ops
#define RND_BLK    4096       // 4 KB random block
#define RND_NBLK   2048       // RND_FILE / RND_BLK

dword buf_disk = 0;
dword disk_dir = 0;           // -> selected target dir (set by the UI)
dword rnd_state;              // LCG state for random offsets
char  dk_op[32];
char  dk_file[80];

dword FileWrite(dword name, data, size)
{
	ESDWORD[#dk_op+0]  = 2;      // subfn 2 = create/rewrite
	ESDWORD[#dk_op+4]  = 0;
	ESDWORD[#dk_op+8]  = 0;
	ESDWORD[#dk_op+12] = size;
	ESDWORD[#dk_op+16] = data;
	DSBYTE [#dk_op+20] = 0;
	ESDWORD[#dk_op+21] = name;
	EAX = 70;
	EBX = #dk_op;
	$int 0x40;
	return EAX;               // 0 = ok
}

dword FileRead(dword name, data, size)
{
	ESDWORD[#dk_op+0]  = 0;      // subfn 0 = read
	ESDWORD[#dk_op+4]  = 0;
	ESDWORD[#dk_op+8]  = 0;
	ESDWORD[#dk_op+12] = size;
	ESDWORD[#dk_op+16] = data;
	DSBYTE [#dk_op+20] = 0;
	ESDWORD[#dk_op+21] = name;
	EAX = 70;
	EBX = #dk_op;
	$int 0x40;
	return EAX;
}

dword dk_delete(dword name)
{
	ESDWORD[#dk_op+0]  = 8;      // subfn 8 = delete
	ESDWORD[#dk_op+4]  = 0;
	ESDWORD[#dk_op+8]  = 0;
	ESDWORD[#dk_op+12] = 0;
	ESDWORD[#dk_op+16] = 0;
	DSBYTE [#dk_op+20] = 0;
	ESDWORD[#dk_op+21] = name;
	EAX = 70;
	EBX = #dk_op;
	$int 0x40;
	return EAX;
}

dword dk_mkdir(dword name)
{
	ESDWORD[#dk_op+0]  = 9;      // subfn 9 = create folder
	ESDWORD[#dk_op+4]  = 0;
	ESDWORD[#dk_op+8]  = 0;
	ESDWORD[#dk_op+12] = 0;
	ESDWORD[#dk_op+16] = 0;
	DSBYTE [#dk_op+20] = 0;
	ESDWORD[#dk_op+21] = name;
	EAX = 70;
	EBX = #dk_op;
	$int 0x40;
	return EAX;
}

dword FileWriteAt(dword name, data, size, pos)   // fn70.3 = write at position
{
	ESDWORD[#dk_op+0]  = 3;
	ESDWORD[#dk_op+4]  = pos;
	ESDWORD[#dk_op+8]  = 0;
	ESDWORD[#dk_op+12] = size;
	ESDWORD[#dk_op+16] = data;
	DSBYTE [#dk_op+20] = 0;
	ESDWORD[#dk_op+21] = name;
	EAX = 70;
	EBX = #dk_op;
	$int 0x40;
	return EAX;
}

dword FileReadAt(dword name, data, size, pos)    // fn70.0 = read at position
{
	ESDWORD[#dk_op+0]  = 0;
	ESDWORD[#dk_op+4]  = pos;
	ESDWORD[#dk_op+8]  = 0;
	ESDWORD[#dk_op+12] = size;
	ESDWORD[#dk_op+16] = data;
	DSBYTE [#dk_op+20] = 0;
	ESDWORD[#dk_op+21] = name;
	EAX = 70;
	EBX = #dk_op;
	$int 0x40;
	return EAX;
}

void DiskPath()                 // dk_file = <disk_dir>/bbtst.tmp
{
	if (disk_dir) strcpy(#dk_file, disk_dir);
	else          strcpy(#dk_file, "/tmp0/1");
	strcat(#dk_file, "/bbtst.tmp");
}

// create/fill an 8 MB file so random 4K offsets are valid and physically written
void EnsureRandFile()
{
	dword pos;
	FileWrite(#dk_file, buf_disk, DISK_CHUNK);                 // create
	pos = DISK_CHUNK;
	while (pos < RND_FILE) {
		FileWriteAt(#dk_file, buf_disk, DISK_CHUNK, pos);
		pos += DISK_CHUNK;
	}
}

// remove the temp file (called by the UI when a run finishes)
void Disk_Cleanup()
{
	DiskPath();
	dk_delete(#dk_file);
}

dword t_disk_write()
{
	dword c=0, r;
	if (!buf_disk) buf_disk = buf_a;
	DiskPath();
	BenchBegin();
	do {
		r = FileWrite(#dk_file, buf_disk, DISK_CHUNK);
		if (r) return BB_SKIP;   // write error: no result, not a slow one
		c += DISK_MB;
	} while (BenchTicks() < 100);
	return PerSecX100(c);         // MB/s
}

dword t_disk_read()
{
	dword c=0, r;
	if (!buf_disk) buf_disk = buf_a;
	DiskPath();
	// Seq Write may be unticked - create the test file if it is missing
	r = FileRead(#dk_file, buf_disk, RND_BLK);
	if (r!=0) && (r!=6) FileWrite(#dk_file, buf_disk, DISK_CHUNK);
	BenchBegin();
	do {
		r = FileRead(#dk_file, buf_disk, DISK_CHUNK);
		if (r!=0) && (r!=6) return BB_SKIP;   // 6 = EOF (still ok)
		c += DISK_MB;
	} while (BenchTicks() < 100);
	return PerSecX100(c);         // MB/s
}

// random 4K write at random offsets in the 8 MB file
dword t_disk_rwrite()
{
	dword c=0, pos, r;
	if (!buf_disk) buf_disk = buf_a;
	DiskPath();
	EnsureRandFile();
	rnd_state = 0x12345678;             // fixed seed: same offsets on every run
	BenchBegin();
	do {
		rnd_state = rnd_state * 1103515245;  rnd_state = rnd_state + 12345;
		r = rnd_state >> 8;  r = r % RND_NBLK;
		pos = r * RND_BLK;
		if (FileWriteAt(#dk_file, buf_disk, RND_BLK, pos)) return BB_SKIP;
		c++;
	} while (BenchTicks() < 100);
	r = muldiv(c, 40000, BenchTicks()); // KiB/s x100 (c * 4 KiB)
	return r / 1024;                    // -> MB/s x100 (MiB, same as Seq)
}

// random 4K read at random offsets  (NB: hits the FS cache - can't be dropped)
dword t_disk_rread()
{
	dword c=0, pos, r, res;
	if (!buf_disk) buf_disk = buf_a;
	DiskPath();
	EnsureRandFile();
	rnd_state = 0x12345678;             // fixed seed: same offsets on every run
	BenchBegin();
	do {
		rnd_state = rnd_state * 1103515245;  rnd_state = rnd_state + 12345;
		r = rnd_state >> 8;  r = r % RND_NBLK;
		pos = r * RND_BLK;
		res = FileReadAt(#dk_file, buf_disk, RND_BLK, pos);
		if (res!=0) && (res!=6) return BB_SKIP;
		c++;
	} while (BenchTicks() < 100);
	r = muldiv(c, 40000, BenchTicks()); // KiB/s x100 (c * 4 KiB)
	return r / 1024;                    // -> MB/s x100 (MiB, same as Seq)
}

//---- File system: create/delete files and folders (metadata ops) ----
char fs_fn[FS_N*64];          // pre-built file names (no sprintf in the loop)
char fs_dn[FS_N*64];          // pre-built folder names

// names via strcpy/strcat only - the same primitives DiskWritable/DiskPath
// use successfully on every disk (no sprintf/itoa in the path building).
// No trailing slash anywhere: folder path is "<dir>/bbd_0".
void fs_names_build()
{
	dword i, p, dir;
	char dig[2];
	dir = disk_dir;
	if (!dir) dir = "/tmp0/1";
	dig[1] = 0;
	for (i=0; i<FS_N; i++) {
		dig[0] = i + '0';
		p = i*64;  p = p + #fs_fn;
		strcpy(p, dir);  strcat(p, "/bbf_");  strcat(p, #dig);  strcat(p, ".tmp");
		p = i*64;  p = p + #fs_dn;
		strcpy(p, dir);  strcat(p, "/bbd_");  strcat(p, #dig);
	}
}

void fs_wipe()                // best-effort removal (also leftovers of a crash)
{
	dword i, p;
	for (i=0; i<FS_N; i++) {
		p = i*64;  p = p + #fs_fn;  dk_delete(p);
		p = i*64;  p = p + #fs_dn;  dk_delete(p);
	}
}

// one unit = create FS_N files (512 B) + FS_N folders, then delete them all.
// BOTH phases are probed before timing and degrade independently: not every
// writable FS driver implements every fn70 op (e.g. exFAT has no
// CreateFolder at all). 0 only when neither files nor folders work.
dword t_disk_fs()
{
	dword c=0, i, p, r;
	byte files_ok, dirs_ok, ok;
	if (!buf_disk) buf_disk = buf_a;
	fs_names_build();
	fs_wipe();
	files_ok = 0;                      // probe file create+delete
	r = FileWrite(#fs_fn, buf_disk, 512);
	if (r==0) {
		r = dk_delete(#fs_fn);
		if (r==0) files_ok = 1;
	}
	dirs_ok = 0;                       // probe folder create+delete
	r = dk_mkdir(#fs_dn);
	if (r==0) {
		r = dk_delete(#fs_dn);
		if (r==0) dirs_ok = 1;
	}
	if (!files_ok) && (!dirs_ok) return BB_SKIP;
	BenchBegin();
	do {
		if (files_ok) {
			ok = 1;
			for (i=0; i<FS_N; i++) {
				p = i*64;  p = p + #fs_fn;
				if (FileWrite(p, buf_disk, 512)) ok = 0;
			}
			for (i=0; i<FS_N; i++) { p = i*64;  p = p + #fs_fn;  dk_delete(p); }
			if (ok) c += 20;      // 2 * FS_N file operations
			else files_ok = 0;    // driver refused mid-run - stop trying
		}
		if (dirs_ok) {
			ok = 1;
			for (i=0; i<FS_N; i++) {
				p = i*64;  p = p + #fs_dn;
				if (dk_mkdir(p)) ok = 0;
			}
			for (i=0; i<FS_N; i++) { p = i*64;  p = p + #fs_dn;  dk_delete(p); }
			if (ok) c += 20;      // 2 * FS_N folder operations
			else dirs_ok = 0;
		}
		if (!files_ok) && (!dirs_ok) break;
	} while (BenchTicks() < 100);
	fs_wipe();                         // belt and braces: no leftovers
	return PerSecX100(c);              // op/s
}

void Register_DISK()
{
	RegisterTest(SECT_DISK, "Seq Write",   "MB/s", REF_DWRITE, #t_disk_write);
	RegisterTest(SECT_DISK, "Seq Read",    "MB/s", REF_DREAD,  #t_disk_read);
	RegisterTest(SECT_DISK, "Rand Write",  "MB/s", REF_RWRITE, #t_disk_rwrite);
	RegisterTest(SECT_DISK, "Rand Read",   "MB/s", REF_RREAD,  #t_disk_rread);
	RegisterTest(SECT_DISK, "File System", "op/s", REF_FS,     #t_disk_fs);
}

#endif
