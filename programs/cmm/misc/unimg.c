// SPDX-License-Identifier: GPL-2.0-only
//
// unimg - FAT12 (floppy) image unpacker
// Copyright (C) 2020-2026 KolibriOS Team
//
// Contributor Magomed Kostoev (Boppan, mkostoevr) - initial version in C
// Contributor Kyryl Lipatov (Leency) - rewrite to C--, support any image type
// Contributor Burer - Russian and Spanish translations, small fixes
//
// Usage:
// unimg /path/to/img ["/output/dir"]
// If output folder is not specified, image will be unpacked at /tmp0/1/[img_name.img_ext]


#define MEMSIZE 1024*30
#define ENTRY_POINT #main

#include "../lib/fs.h"

// NB: this file is stored in CP866 (for the Russian strings below).
//     Edit it with a CP866-aware editor, not UTF-8.
#ifdef LANG_RUS
?define MSG_USAGE    "'unimg\nРаспаковщик образов FAT12.\nИспользование: unimg /путь/к/образу [\"/папка/вывода\"]' -tI"
?define MSG_READ     "'unimg\nОшибка чтения файла\n%s' -tE"
?define MSG_FORMAT   "'unimg\nОбраз не FAT12.\n%s' -tE"
?define MSG_MKDIR    "'unimg\nОшибка создания папки\n%s' -tE"
?define MSG_DONE     "'unimg\nРаспаковано в:\n%s' -tO"
?define MSG_NOMEM    "'unimg\nНедостаточно памяти' -tE"
?define MSG_OOB      "'unimg\nКластера FAT вне границ' -tE"
?define INFO_NAME    "\nВнутреннее имя      "
?define INFO_BPS     "\nБайт в секторе      "
?define INFO_SPC     "\nСекторов в кластере "
?define INFO_RSC     "\nРезервных секторов  "
?define INFO_NFAT    "\nКоличество FAT      "
?define INFO_MRE     "\nЗаписей в корне     "
?define INFO_BPC     "\nБайт в кластере     "
?define INFO_SPF     "\nСекторов в FAT      "
?define INFO_FATOFF  "\nПервая FAT          "
?define INFO_ROOTOFF "\nКорневой каталог    "
?define INFO_DATAOFF "\nОбласть данных      "
#else
#ifdef LANG_SPA
?define MSG_USAGE    "'unimg\nDescompresor de imagenes FAT12.\nUso: unimg /ruta/a/imagen [\"/carpeta/salida\"]' -tI"
?define MSG_READ     "'unimg\nError al leer el archivo\n%s' -tE"
?define MSG_FORMAT   "'unimg\nNo es una imagen FAT12.\n%s' -tE"
?define MSG_MKDIR    "'unimg\nNo se pudo crear la carpeta\n%s' -tE"
?define MSG_DONE     "'unimg\nDescomprimido en\n%s' -tO"
?define MSG_NOMEM    "'unimg\nSin memoria' -tE"
?define MSG_OOB      "'unimg\nCluster FAT fuera de limites' -tE"
?define INFO_NAME    "\nNombre interno      "
?define INFO_BPS     "\nBytes en sector     "
?define INFO_SPC     "\nSectores en cluster "
?define INFO_RSC     "\nSectores reservados "
?define INFO_NFAT    "\nNumero de FATs      "
?define INFO_MRE     "\nEntradas raiz max   "
?define INFO_BPC     "\nBytes en cluster    "
?define INFO_SPF     "\nSectores en FAT     "
?define INFO_FATOFF  "\nPrimera FAT         "
?define INFO_ROOTOFF "\nDirectorio raiz     "
?define INFO_DATAOFF "\nRegion de datos     "
#else
?define MSG_USAGE    "'unimg\nFAT12 image unpacker.\nUsage: unimg /path/to/img [\"/output/dir\"]' -tI"
?define MSG_READ     "'unimg\nFile read error\n%s' -tE"
?define MSG_FORMAT   "'unimg\nNot a FAT12 image.\n%s' -tE"
?define MSG_MKDIR    "'unimg\nCannot create output folder\n%s' -tE"
?define MSG_DONE     "'unimg\nUnpacked to\n%s' -tO"
?define MSG_NOMEM    "'unimg\nOut of memory' -tE"
?define MSG_OOB      "'unimg\nFAT cluster out of bounds' -tE"
?define INFO_NAME    "\nInternal name       "
?define INFO_BPS     "\nBytes per sector    "
?define INFO_SPC     "\nSectors per cluster "
?define INFO_RSC     "\nReserved sectors    "
?define INFO_NFAT    "\nNumber of FATs      "
?define INFO_MRE     "\nMax root entries    "
?define INFO_BPC     "\nBytes per cluster   "
?define INFO_SPF     "\nSectors per FAT     "
?define INFO_FATOFF  "\nFirst FAT           "
?define INFO_ROOTOFF "\nRoot directory      "
?define INFO_DATAOFF "\nData region         "
#endif
#endif

dword img_ptr;
dword img_sz;

int bps;
int spc;
int rsc;
int nfat;
int mre;
int spf;
int bpc;
int fat_off;
int root_off;
int data_off;

int lfn_nonascii;
int lfn_checksum;

char cur_path[4096];
int  cur_path_len;

inline int fat_u8(int offset) { return DSBYTE[img_ptr + offset] & 0xFF; }
inline int fat_u16(int offset) { return DSWORD[img_ptr + offset] & 0xFFFF; }
inline dword fat_u32(int offset) { return ESDWORD[img_ptr + offset]; }

void fat12__open()
{
	bps      = fat_u16(11);
	spc      = fat_u8(13);
	rsc      = fat_u16(14);
	nfat     = fat_u8(16);
	mre      = fat_u16(17);
	spf      = fat_u16(22);
	bpc      = bps * spc;
	fat_off  = rsc * bps;
	root_off = nfat * spf * bps + fat_off;
	data_off = mre * 32 + root_off;
}

dword GetImageInfo(dword buf)
{
	strcpy(buf, INFO_NAME);    strncpy(buf+21, img_ptr+3, 8);
	strcat(buf, INFO_BPS);     strcat(buf, itoa(bps));
	strcat(buf, INFO_SPC);     strcat(buf, itoa(spc));
	strcat(buf, INFO_RSC);     strcat(buf, itoa(rsc));
	strcat(buf, INFO_NFAT);    strcat(buf, itoa(nfat));
	strcat(buf, INFO_MRE);     strcat(buf, itoa(mre));
	strcat(buf, INFO_BPC);     strcat(buf, itoa(bpc));
	strcat(buf, INFO_SPF);     strcat(buf, itoa(spf));
	strcat(buf, INFO_FATOFF);  strcat(buf, itoa(fat_off));
	strcat(buf, INFO_ROOTOFF); strcat(buf, itoa(root_off));
	strcat(buf, INFO_DATAOFF); strcat(buf, itoa(data_off));
}

int cluster_to_offset(int c)
{
	return c - 2 * bpc + data_off;
}

int next_cluster(int c)
{
	int v;
	v = fat_u16(c / 2 + fat_off + c);
	if (c & 1) return v >> 4;
	return v & 0xFFF;
}

void extract_file(dword buf, int size, int cluster)
{
	int written, n, src;
	written = 0;
	while (cluster >= 2) && (cluster < 0xFF7) {
		src = cluster_to_offset(cluster);
		n = bpc;
		if (written + n > size) n = size - written;
		if (n <= 0) break;
		// Stop on a corrupt FAT chain pointing outside the image
		if (src < 0) || (src + n > img_sz) break;
		memmov(buf + written, img_ptr + src, n);
		written += n;
		cluster = next_cluster(cluster);
	}
}

void make_parent_dirs(dword initial_dir)
{
	char path[4096];
	int i;
	strncpy(#path, initial_dir, strlen(initial_dir) + 1);
	for (i = 1; path[i]; i++) {
		if (path[i] == '/') {
			path[i] = '\0';
			CreateDir(#path);
			path[i] = '/';
		}
	}
}

int entry_short_name(int off, dword out)
{
	int attr, i, n, b;
	b = fat_u8(off);
	if (!b) return -1;
	if (b == 0xE5) return 0;
	attr = fat_u8(off + 11);
	if (attr & 0x08) return 0;

	n = 0;
	for (i = 0; i < 8; i++) {
		b = fat_u8(off + i);
		if (b == ' ') break;
		ESBYTE[out + n] = b;
		n++;
	}
	b = fat_u8(off + 8);
	if (b != ' ') {
		ESBYTE[out + n] = '.';
		n++;
		for (i = 8; i < 11; i++) {
			b = fat_u8(off + i);
			if (b == ' ') break;
			ESBYTE[out + n] = b;
			n++;
		}
	}
	ESBYTE[out + n] = '\0';
	return n;
}

// Extracts characters from an LFN record. 
// Zeroes the buffer when the end of line (0x0000 or 0xFFFF) is encountered.
int get_lfn_chars(int off, dword out)
{
	int i, c, pos;
	// LFN stores 13 UCS-2 chars at non-contiguous offsets 1..10, 14..25, 28..31
	// (gaps: 11..13 attr/type/checksum, 26..27 cluster).
	i = 1;
	for (pos = 0; pos < 13; pos++) {
		c = fat_u16(off + i);
		if (c == 0x0000) || (c == 0xFFFF) { ESBYTE[out+pos] = 0; return pos; }
		if (c > 0x7F) lfn_nonascii = 1;
		ESBYTE[out + pos] = c & 0xFF;
		i += 2;
		if (i == 11) i = 14;
		if (i == 26) i = 28;
	}
	ESBYTE[out + pos] = 0;
	return pos;
}

// Returns -1 to stop iteration, 0 to continue.
int process_one_entry(int entry_off, dword lfn_buf, dword p_seq)
{
	char chunk[15];
	int b, attr, seq, chunk_len, dest_offset;

	b = fat_u8(entry_off);
	if (!b) return -1;
	if (b == 0xE5) { ESDWORD[p_seq] = -1; return 0; }

	attr = fat_u8(entry_off + 11);
	if (attr == 0x0F) {
		seq = b;
		if (seq & 0x40) {
			seq &= 0x3F;
			if (seq < 1) || (seq > 20) { ESDWORD[p_seq] = -1; return 0; }
			ESDWORD[p_seq] = seq;
			lfn_nonascii = 0;
			lfn_checksum = fat_u8(entry_off + 13);
			EDI = lfn_buf; ECX = 65; EAX = 0; MEMSETD(EDI, ECX, EAX);
		}
		if (seq == ESDWORD[p_seq]) {
			chunk_len = get_lfn_chars(entry_off, #chunk);
			dest_offset = seq - 1;
			dest_offset *= 13;
			memmov(lfn_buf + dest_offset, #chunk, chunk_len);
			ESDWORD[p_seq] = ESDWORD[p_seq] - 1;
		} else {
			ESDWORD[p_seq] = -1;
		}
	} else {
		if (ESDWORD[p_seq] == 0) {
			if (handle_entry(entry_off, lfn_buf) < 0) return -1;
		} else {
			if (handle_entry(entry_off, NULL) < 0) return -1;
		}
		ESDWORD[p_seq] = -1;
		ESBYTE[lfn_buf] = 0;
	}
	return 0;
}

int handle_entry(int off, dword lfn_name)
{
	char  name[256];
	int   nlen, attr, cluster, size, saved_len, use_lfn, sum, hi, i;
	dword buf;

	if (off < 0) || (off + 32 > img_sz) {
		return -1;
	}

	attr = fat_u8(off + 11);
	if (attr == 0x0F) return 0;

	// Verify LFN checksum against the short-name field (off..off+10).
	use_lfn = 0;
	if (lfn_name) && (ESBYTE[lfn_name]) && (!lfn_nonascii) {
		sum = 0;
		for (i = 0; i < 11; i++) {
			hi = sum & 1;
			hi = hi << 7;
			sum = sum >> 1;
			sum = sum + hi;
			sum = sum + fat_u8(off + i);
			sum = sum & 0xFF;
		}
		if (sum == lfn_checksum) use_lfn = 1;
	}

	if (use_lfn) {
		strncpy(#name, lfn_name, 255);
		nlen = strlen(#name);
	} else {
		nlen = entry_short_name(off, #name);
		if (nlen < 0) return -1;
		if (nlen == 0) return 0;
	}

	// Skip only the "." and ".." directory entries (a real LFN name may start with '.').
	if (name[0] == '.') && (name[1] == '\0') return 0;
	if (name[0] == '.') && (name[1] == '.') && (name[2] == '\0') return 0;

	cluster = fat_u16(off + 26);

	saved_len = cur_path_len;
	cur_path[cur_path_len] = '/';
	memmov(#cur_path + cur_path_len + 1, #name, nlen + 1);
	cur_path_len = cur_path_len + 1 + nlen;

	if (attr & 0x10) {
		CreateDir(#cur_path);
		handle_folder(cluster);
	} else {
		size = fat_u32(off + 28);
		if (size > 0) {
			buf = malloc(size);
			if (!buf) {
				notify(MSG_NOMEM);
			} else {
				extract_file(buf, size, cluster);
				CreateFile(size, buf, #cur_path);
				free(buf);
			}
		} else {
			CreateFile(0, img_ptr, #cur_path);
		}
	}

	cur_path_len = saved_len;
	cur_path[cur_path_len] = '\0';
	return 0;
}

void handle_folder(int cluster)
{
	int off, entries, i, entry_off;
	int lfn_expected_seq;
	char lfn_buf[260];

	lfn_buf[0] = 0;
	lfn_expected_seq = -1;

	while (cluster >= 2) && (cluster < 0xFF7) {
		off = cluster_to_offset(cluster);

		if (off < 0) || (off + bpc > img_sz) {
			notify(MSG_OOB);
			return;
		}

		entries = bpc / 32;
		for (i = 0; i < entries; i++) {
			entry_off = i * 32 + off;
			if (process_one_entry(entry_off, #lfn_buf, #lfn_expected_seq) < 0) return;
		}
		cluster = next_cluster(cluster);
	}
}

void NotifyAndExit(dword message, opt)
{
	miniprintf(#param, message, opt);
	notify(#param);
	ExitProcess();
}

void main()
{
	char img_file[4096];
	char img_info[512];
	char root_lfn[260];
	int  i, entry_off, lfn_expected_seq;

	mem_init();

	if (!param[0]) {
		NotifyAndExit(MSG_USAGE, NULL);
	}

	if (i = strchr(#param, '"')) {
		ESBYTE[i-1] = '\0';
		strcpy(#cur_path, i+1);
		i = strchr(#cur_path, '"');
		if (i) ESBYTE[i] = '\0';
	}

	strcpy(#img_file, #param);

	if (!cur_path[0]) {
		memmov(#cur_path, "/tmp0/1", 8);
		strcat(#cur_path, #img_file + strrchr(#img_file, '/') - 1);
	}
	cur_path_len = strlen(#cur_path);

	read_file(#img_file, #img_ptr, #img_sz);
	if (!img_ptr) || (!img_sz) {
		NotifyAndExit(MSG_READ, #img_file);
	}

	fat12__open();
	if (root_off > img_sz) || (data_off > img_sz) {
		GetImageInfo(#img_info);
		NotifyAndExit(MSG_FORMAT, #img_info);
	}

	make_parent_dirs(#cur_path);
	if (CreateDir(#cur_path)) {
		NotifyAndExit(MSG_MKDIR, #cur_path);
	}

	root_lfn[0] = 0;
	lfn_expected_seq = -1;
	for (i = 0; i < mre; i++) {
		entry_off = i * 32 + root_off;
		if (process_one_entry(entry_off, #root_lfn, #lfn_expected_seq) < 0) break;
	}

	NotifyAndExit(MSG_DONE, #cur_path);
}
