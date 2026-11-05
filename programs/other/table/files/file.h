#ifndef FILES_FILE_H
#define FILES_FILE_H

// The table in a CSV file and from one, saved as it was opened: in its
// encoding, with its separator and line ends.

enum { FILE_UTF8, FILE_UTF8_BOM, FILE_CP866 };

// how the file of the table is written: as table_load() found it, or for
// a new table after new_file() UTF-8 with its mark, ',' and "\n"
typedef struct {
	int encoding; // FILE_*
	char separator;
	int crlf;     // lines ended by "\r\n"
} FileFormat;

extern FileFormat file_format;

void new_file(void);
// LOAD_*: not read, the table as it was; read, all of it, or only what
// fits: the file is more than the table holds
enum { LOAD_NONE, LOAD_WHOLE, LOAD_PART };
int table_load(const char *path);
int table_save(const char *path); // 1 saved, 0 not

#endif
