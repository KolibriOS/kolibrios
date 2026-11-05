#include "../sheet/cells.h"
#include "csv.h"
#include "encoding.h"
#include "file.h"

#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>

// the longest file it loads: the longest one it saves, all cells CELL_MAX
// quotes, each one doubled, or CELL_MAX letters of two bytes in UTF-8, in
// quotes, with a separator; the line ends and the mark of UTF-8
#define FILE_MAX (COLS * ROWS * (2 * CELL_MAX + 3) + 2 * ROWS + 3)
#define UTF8_BOM "\xEF\xBB\xBF"
#define UTF8_BOM_LEN (sizeof(UTF8_BOM) - 1)

FileFormat file_format = { FILE_UTF8_BOM, ',', 0 };

void new_file(void)
{
	file_format.encoding = FILE_UTF8_BOM; // Excel takes it for UTF-8 only with the mark
	file_format.separator = ',';
	file_format.crlf = 0;
}

// the subfunctions of function 70 it uses
enum { F70_READ = 0, F70_CREATE = 2, F70_INFO = 5 };

// function 70, subfunction func, for the file at path: size bytes from
// its start into or out of buf; KSYS_FS_ERR_SUCCESS if done
static int file_call(int func, const char *path, uint32_t size, void *buf)
{
	ksys70_t k;

	k.p00 = func;
	k.p04dw = 0;
	k.p08dw = 0;
	k.p12 = size;
	k.buf16 = buf;
	k.p20 = 0;
	k.p21 = path;
	return _ksys70(&k).status;
}

// the whole file, zero ended, or NULL
static char *file_read_all(const char *path)
{
	ksys_bdfe_t info; // F70_INFO gives no name
	char *data;

	if (file_call(F70_INFO, path, 0, &info) != KSYS_FS_ERR_SUCCESS || info.size > FILE_MAX)
		return NULL;
	data = calloc(info.size + 1, 1);
	if (!data)
		return NULL;
	if (file_call(F70_READ, path, info.size, data) != KSYS_FS_ERR_SUCCESS) { // or EOF: shorter now
		free(data);
		return NULL;
	}
	return data;
}

int table_load(const char *path)
{
	char *data = file_read_all(path), *text, *s, *d;
	int whole = 1;

	if (!data)
		return LOAD_NONE;
	// the encoding: UTF-8 with its mark, UTF-8, else cp866; the cells in cp866
	text = data;
	if (strncmp(text, UTF8_BOM, UTF8_BOM_LEN) == 0) {
		file_format.encoding = FILE_UTF8_BOM;
		text += UTF8_BOM_LEN;
		whole = is_utf8(text); // characters cp866 has not become '?'
	} else
		file_format.encoding = is_utf8(text) ? FILE_UTF8 : FILE_CP866;
	if (file_format.encoding != FILE_CP866)
		utf8_to_cp866(text, text, strlen(text) + 1);
	// the line ends: "\r\n" if the first one is; all of them '\n', a '\r'
	// alone too
	s = strchr(text, '\n');
	file_format.crlf = s && s > text && s[-1] == '\r';
	for (s = d = text; *s; s++) {
		if (*s != '\r')
			*d++ = *s;
		else if (s[1] != '\n')
			*d++ = '\n';
	}
	*d = '\0';
	file_format.separator = csv_separator(text);

	table_clear();
	if (!csv_load(text, file_format.separator))
		whole = 0;
	free(data);
	table_recalc();
	return whole ? LOAD_WHOLE : LOAD_PART;
}

int table_save(const char *path)
{
	Out o = { NULL, 0, 0, 0, file_format.encoding != FILE_CP866 };
	int ok;

	if (file_format.encoding == FILE_UTF8_BOM)
		out_text(&o, UTF8_BOM);
	csv_save(&o, file_format.separator, file_format.crlf ? "\r\n" : "\n");
	ok = !o.failed && file_call(F70_CREATE, path, o.len, o.data) == KSYS_FS_ERR_SUCCESS; // or replace
	free(o.data);
	if (ok)
		table_changed = 0;
	return ok;
}
