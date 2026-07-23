#include "func.h"
#include "parser.h"
#include "calc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>

#define DEFAULT_CELL_W 82
#define DEFAULT_CELL_H 21

#define sign(x) ((x) < 0 ? -1 : ((x) == 0 ? 0 : 1))

// the cell model lives in the main module
extern int col_count, row_count;
extern char ***cells;
extern int *cell_w, *cell_h;
extern int *cell_x, *cell_y;
extern char ***values;
extern char ***buffer;
extern int buf_col, buf_row;

extern const char *sFileSign;

int cf_x0, cf_x1, cf_y0, cf_y1;

static uint32_t filesize;

struct cell_list {
	int x, y;
	struct cell_list *next;
};
typedef struct cell_list cell_list;

static char *make_col_cap(int i);
static char *make_row_cap(int i);
static char *make_cell_name(int x, int y, int xd, int yd);
static int parse_cell_name(char *str, int *px, int *py, int *xd, int *yd);
static char *change_cell_ref(char *name, int sx, int sy);
static int str_is_csv(char *str);
static char GetCsvSeparator(char *fname);
static char *file_readline(const char *name, uint32_t *off);
static int SaveCSV(char *fname);
static int LoadCSV(char *fname);
static double calc_callback(char *str);
static double depend_callback(char *str);
static cell_list *find_depend(char *str);
static int is_in_list(cell_list *c1, cell_list *c2);

/* ===================== file helpers (syscall 70) ===================== */

// append len bytes to file at *off, advance *off; 1 ok / 0 error
static int file_put(const char *name, uint32_t *off, const void *buf, uint32_t len)
{
	ksys70_status_t st = _ksys_file_write(name, *off, len, buf);
	if (st.status != 0)
		return 0;
	*off += len;
	return 1;
}

// create/truncate a file (fn70.2). _ksys_file_create() leaves the p20
// separator byte uninitialized, which corrupts the request - do it right here.
static int file_create(const char *name)
{
	ksys70_t k;
	k.p00 = 2;
	k.p04dw = 0;
	k.p08dw = 0;
	k.p12 = 0;
	k.p16 = 0;
	k.p20 = 0;
	k.p21 = name;
	return _ksys70(&k).status;
}

// read up to len bytes at off; *got = bytes read; returns fs status (0 ok, 6 eof)
static int file_get(const char *name, uint32_t off, uint32_t len, void *buf, uint32_t *got)
{
	ksys70_status_t st = _ksys_file_read(name, off, len, buf);
	*got = st.rw_bytes;
	return st.status;
}

// read the next '\n'-terminated line starting at *off; advances *off;
// returns a fresh string ("" for a blank line) or NULL at end/error
static char *file_readline(const char *name, uint32_t *off)
{
	char buffer[512], *p, *r;
	uint32_t got;
	int status, l;

	memset(buffer, 0, 512);
	status = file_get(name, *off, 512, buffer, &got);
	if (status != 0 && status != 6)
		return NULL;

	p = buffer;
	while (*p && *p++ != '\n')
		;
	if (p == buffer)
		return NULL;

	r = malloc(p - buffer);
	if (!r)
		return NULL;
	memset(r, 0, p - buffer);
	for (l = 0; l < p - buffer - 1; l++)
		r[l] = buffer[l];
	*off += p - buffer;
	return r;
}

/* ===================== cell captions / model ===================== */

// column caption: 1->"A", 27->"AA", ...
static char *make_col_cap(int i)
{
	char *r = malloc(3);
	if (i <= 26) {
		r[0] = 'A' + i - 1;
		r[1] = '\0';
		return r;
	}
	if (i % 26 == 0) { // last letter would be 'Z', carry to the previous one
		r[0] = (i / 26) - 1 + 'A' - 1;
		r[1] = 'Z';
		r[2] = '\0';
		return r;
	}
	r[0] = (i / 26) + 'A' - 1;
	r[1] = (i % 26) + 'A' - 1;
	r[2] = '\0';
	return r;
}

// row caption: numeric (up to 3 digits, so row 100 renders as "100")
static char *make_row_cap(int i)
{
	char *r = malloc(4);
	if (i <= 9) {
		r[0] = '0' + i;
		r[1] = '\0';
	} else if (i <= 99) {
		r[0] = (i / 10) + '0';
		r[1] = (i % 10) + '0';
		r[2] = '\0';
	} else {
		r[0] = (i / 100) + '0';
		r[1] = ((i / 10) % 10) + '0';
		r[2] = (i % 10) + '0';
		r[3] = '\0';
	}
	return r;
}

void init(void)
{
	int i, j;

	cell_w = malloc(col_count * sizeof(int));
	cell_h = malloc(row_count * sizeof(int));
	cell_x = malloc(col_count * sizeof(int));
	cell_y = malloc(row_count * sizeof(int));

	for (i = 0; i < col_count; i++)
		cell_w[i] = DEFAULT_CELL_W;
	cell_w[0] = 30; // make row headers smaller

	for (i = 0; i < row_count; i++)
		cell_h[i] = DEFAULT_CELL_H;

	cells = malloc(col_count * sizeof(char **));
	values = malloc(col_count * sizeof(char **));
	for (i = 0; i < col_count; i++) {
		cells[i] = malloc(row_count * sizeof(char *));
		values[i] = malloc(row_count * sizeof(char *));
		for (j = 0; j < row_count; j++) {
			cells[i][j] = NULL;
			values[i][j] = NULL;
			if (i == 0 && j)
				cells[i][j] = make_row_cap(j);
			else if (j == 0 && i)
				cells[i][j] = make_col_cap(i);
		}
	}
}

void reinit(void)
{
	int i, j;

	for (i = 0; i < col_count; i++)
		cell_w[i] = DEFAULT_CELL_W;
	cell_w[0] = 30;

	for (i = 0; i < row_count; i++)
		cell_h[i] = DEFAULT_CELL_H;

	for (i = 1; i < col_count; i++)
		for (j = 1; j < row_count; j++) {
			if (cells[i][j])
				free(cells[i][j]);
			cells[i][j] = NULL;
			if (values[i][j])
				free(values[i][j]);
			values[i][j] = NULL;
		}
}

/* ===================== cell references ===================== */

static int parse_cell_name(char *str, int *px, int *py, int *xd, int *yd)
{
	int i, j, x, y, dx = 0, dy = 0;

	if (*str == '$') {
		str++;
		dx = 1;
	}
	for (i = 0; i < strlen(str); i++)
		if (str[i] >= '0' && str[i] <= '9')
			break;
	if (i > 0 && str[i - 1] == '$') {
		i--;
		dy = 1;
	}
	if (i == strlen(str))
		return 0;

	x = -1;
	for (j = 1; j < col_count; j++)
		if (strncmp(str, cells[j][0], i) == 0) {
			x = j;
			break;
		}
	if (str[i] == '$')
		i++;
	y = -1;
	for (j = 1; j < row_count; j++)
		if (strcmp(str + i, cells[0][j]) == 0) {
			y = j;
			break;
		}
	if (x == -1 || y == -1)
		return 0;

	*px = x;
	*py = y;
	if (xd)
		*xd = dx;
	if (yd)
		*yd = dy;
	return 1;
}

static char *make_cell_name(int x, int y, int xd, int yd)
{
	char *col_cap, *row_cap, *res;
	int i = 0;

	if (x <= 0 || x > col_count || y <= 0 || y > row_count)
		return NULL;

	col_cap = make_col_cap(x);
	row_cap = make_row_cap(y);

	res = malloc(strlen(col_cap) + strlen(row_cap) + (xd ? 1 : 0) + (yd ? 1 : 0) + 1);
	if (xd)
		res[i++] = '$';
	strcpy(res + i, col_cap);
	i += strlen(col_cap);
	if (yd)
		res[i++] = '$';
	strcpy(res + i, row_cap);
	i += strlen(row_cap);
	res[i] = '\0';

	free(col_cap);
	free(row_cap);
	return res;
}

// shift one cell reference by (sx, sy) unless it is absolute ($) or outside cf_*
static char *change_cell_ref(char *name, int sx, int sy)
{
	int x0, y0, xd, yd;

	parse_cell_name(name, &x0, &y0, &xd, &yd);

	if (x0 >= cf_x0 && x0 <= cf_x1 && y0 >= cf_y0 && y0 <= cf_y1) {
		if (!xd) {
			x0 += sx;
			if (x0 <= 0 || x0 > col_count)
				x0 -= sx;
		}
		if (!yd) {
			y0 += sy;
			if (y0 <= 0 || y0 > row_count)
				y0 -= sy;
		}
	}

	return make_cell_name(x0, y0, xd, yd);
}

// rewrite every cell reference in a formula by (sx, sy)
char *change_formula(char *name, int sx, int sy)
{
	int i = 0;
	int in_name = 0; // 0 nothing, 1 letters part, 2 digits part
	int alp_len = 0, dig_len = 0;
	int buf_i = 0;
	char buffer[256];
	char *res;

	memset(buffer, 0, 256);

	while (i < strlen(name) + 1) {
		char c = (i == strlen(name)) ? ' ' : name[i];
		buffer[buf_i++] = c;

		switch (in_name) {
		case 0:
			if (isalpha2(c) || c == '$') {
				in_name = 1;
				alp_len = 1;
				dig_len = 0;
			}
			break;
		case 1:
			if (isalpha2(c)) {
				alp_len++;
			} else if (c == '$' || isdigit2(c)) {
				in_name = 2;
				dig_len++;
			} else {
				in_name = 0;
				alp_len = dig_len = 0;
			}
			break;
		case 2:
			if (isdigit2(c)) {
				dig_len++;
			} else if (alp_len > 0 && dig_len > 0) {
				int idx = i - alp_len - dig_len;
				int len = alp_len + dig_len;
				int l;
				char *cell = malloc(len + 1);
				char *cell_new;
				for (l = 0; l < len; l++)
					cell[l] = name[idx + l];
				cell[len] = '\0';

				cell_new = change_cell_ref(cell, sx, sy);
				if (cell_new) {
					char cc = buffer[buf_i - 1];
					strcpy(buffer + buf_i - len - 1, cell_new);
					buf_i += strlen(cell_new) - len;
					buffer[buf_i - 1] = cc;
					free(cell_new);
				}
				free(cell);
				alp_len = dig_len = 0;
				in_name = 0;
			}
			break;
		}
		i++;
	}

	res = malloc(strlen(buffer) + 1);
	strcpy(res, buffer);
	return res;
}

void fill_cells(int sel_x, int sel_y, int sel_end_x, int sel_end_y, int old_end_x, int old_end_y)
{
	int i, start, end, step, gdir = -1;
	char *source;

	cf_x0 = cf_y0 = 0;
	cf_x1 = col_count;
	cf_y1 = row_count;

	if (sel_end_x == -1)
		sel_end_x = sel_x;
	if (sel_end_y == -1)
		sel_end_y = sel_y;

	if (old_end_x == sel_end_x && sel_y == old_end_y)
		gdir = 0;
	else if (old_end_y == sel_end_y && sel_x == old_end_x)
		gdir = 1;

	if (gdir != -1) {
		int gstep = gdir ? sign(old_end_y - sel_y) : sign(old_end_x - sel_x);
		if (gstep == 0)
			gstep = 1;

		for (; gdir ? (sel_y != old_end_y + gstep) : (sel_x != old_end_x + gstep);
		     gdir ? (sel_y += gstep) : (sel_x += gstep)) {
			int dir;
			source = cells[sel_x][sel_y];
			if (gdir == 0) {
				start = sel_y;
				end = sel_end_y;
				step = (sel_y < sel_end_y ? 1 : -1);
				dir = 1;
			} else {
				start = sel_x;
				end = sel_end_x;
				step = (sel_x < sel_end_x ? 1 : -1);
				dir = 0;
			}

			for (i = start + step; i != end + step; i += step) {
				if (cells[dir ? sel_x : i][dir ? i : sel_y])
					free(cells[dir ? sel_x : i][dir ? i : sel_y]);
				if (source)
					cells[dir ? sel_x : i][dir ? i : sel_y] =
						change_formula(source, dir ? 0 : (i - start), dir ? (i - start) : 0);
				else
					cells[dir ? sel_x : i][dir ? i : sel_y] = NULL;
			}
		}
	}

	calculate_values();
}

/* ===================== save / load ===================== */

static int str_is_csv(char *str)
{
	int str_len = strlen(str);
	if (str_len >= 5 && strnicmp(str + str_len - 4, ".CSV", 4) == 0)
		return 1;
	return 0;
}

static int SaveCSV(char *fname)
{
	int i, j;
	int min_col = col_count, min_row = row_count, max_row = -1, max_col = -1;
	uint32_t off = 0;

	_ksys_file_delete(fname);
	file_create(fname);

	for (i = 1; i < col_count; i++)
		for (j = 1; j < row_count; j++)
			if (cells[i][j]) {
				min_col = min(min_col, i);
				min_row = min(min_row, j);
				max_col = max(max_col, i);
				max_row = max(max_row, j);
			}

	for (j = min_row; j <= max_row; j++) {
		char buffer[1024];
		int buf_len = 0;
		memset(buffer, 0, 1024);

		for (i = min_col; i <= max_col; i++) {
			char *cur = values[i][j] ? values[i][j] : cells[i][j];
			if (cur) {
				int k, n = strlen(cur);
				buffer[buf_len++] = '\"';
				for (k = 0; k < n; k++) {
					if (cur[k] == '\"')
						buffer[buf_len++] = '\"'; // escape by doubling
					buffer[buf_len++] = cur[k];
				}
				buffer[buf_len++] = '\"';
			}
			buffer[buf_len++] = ',';
		}
		buffer[buf_len++] = '\n';
		if (!file_put(fname, &off, buffer, buf_len))
			return 0;
	}
	return 1;
}

#define BUF_FOR_ALL 5000
int SaveFile(char *fname)
{
	uint32_t off = 0;
	int i, j;
	char *buffer;

	if (str_is_csv(fname))
		return SaveCSV(fname);

	_ksys_file_delete(fname);
	file_create(fname);

	buffer = malloc(BUF_FOR_ALL);
	if (!buffer)
		return 0;

	if (!file_put(fname, &off, sFileSign, strlen(sFileSign))) {
		free(buffer);
		return 0;
	}

	// column widths line
	memset(buffer, 0, BUF_FOR_ALL);
	for (i = 1; i < col_count; i++) {
		char smalbuf[32];
		sprintf(smalbuf, "%u,", cell_w[i]);
		strcpy(buffer + strlen(buffer), smalbuf);
	}
	buffer[strlen(buffer) - 1] = '\n'; // trailing comma -> newline
	if (!file_put(fname, &off, buffer, strlen(buffer))) {
		free(buffer);
		return 0;
	}

	// row heights line
	memset(buffer, 0, BUF_FOR_ALL);
	for (i = 1; i < row_count; i++) {
		char smalbuf[32];
		sprintf(smalbuf, "%u,", cell_h[i]);
		strcpy(buffer + strlen(buffer), smalbuf);
	}
	buffer[strlen(buffer) - 1] = '\n';
	if (!file_put(fname, &off, buffer, strlen(buffer))) {
		free(buffer);
		return 0;
	}

	// cells as "col row:text"
	for (i = 1; i < row_count; i++)
		for (j = 1; j < col_count; j++)
			if (cells[j][i]) {
				sprintf(buffer, "%u %u:%s\n", j, i, cells[j][i]);
				if (!file_put(fname, &off, buffer, strlen(buffer))) {
					free(buffer);
					return 0;
				}
			}

	free(buffer);
	return 1;
}

static char GetCsvSeparator(char *fname)
{
	char buffer[512];
	uint32_t got, load_size = (filesize < 512) ? filesize : 511;

	memset(buffer, 0, 512);
	if (file_get(fname, 0, load_size, buffer, &got) == 0) {
		int commas = chrnum(buffer, ',');
		int semicolons = chrnum(buffer, ';');
		if (semicolons > commas)
			return ';';
	}
	return ',';
}

static int LoadCSV(char *fname)
{
	uint32_t off = 0;
	char separator, *line;
	int col = 1, row = 1;

	reinit();
	separator = GetCsvSeparator(fname);

	do {
		int i, llen;
		line = file_readline(fname, &off);
		if (!line || *line == '\0') {
			free(line);
			break;
		}
		llen = strlen(line);

		i = 0;
		while (i <= llen) {
			int inPar = 0; // 0 outside, 1 just opened quote, 2 inside quotes
			int start = i;
			while (i <= llen) {
				char c = line[i];
				int field_end = 0;
				if (!c)
					c = separator;

				switch (inPar) {
				case 0:
					if (c == '\"')
						inPar = 1;
					else if (c == separator)
						field_end = 1;
					break;
				case 1:
					inPar = 2;
					break;
				case 2:
					if (c == '\"')
						inPar = 0;
					break;
				}
				if (field_end) {
					int tmp = line[start] == '"' ? 1 : 0;
					int sz = i - start - tmp * 2;
					if (sz > 0) {
						int l, m = 0;
						cells[col][row] = malloc(sz + 1);
						memset(cells[col][row], 0, sz + 1);
						for (l = 0; l < sz; l++) {
							if (line[start + tmp + l] == '\"') {
								cells[col][row][m++] = '\"';
								l++; // skip the doubled quote
							} else
								cells[col][row][m++] = line[start + tmp + l];
						}
					}
					start = i + 1;
					col++;
				}
				i++;
			}
			row++;
			col = 1;
			i++;
		}
		free(line);
	} while (line);

	return 1;
}

int LoadFile(char *fname)
{
	char info[560]; // BDFE header + long name
	uint32_t off = 0, got;
	int i, j, step = 0, items;
	char buffer[512 + 1];
	char *d, *s, *k;

	if (_ksys_file_info(fname, (ksys_bdfe_t *)info) != 0)
		return -1;
	filesize = (uint32_t)((ksys_bdfe_t *)info)->size;

	if (str_is_csv(fname))
		return LoadCSV(fname);

	reinit();

	// verify signature
	memset(buffer, 0, 512);
	file_get(fname, 0, strlen(sFileSign), buffer, &got);
	s = (char *)sFileSign;
	d = buffer;
	while (*s && *d && *s++ == *d++)
		;
	if (*s != '\0' || *d != '\0')
		return -2;
	off += strlen(sFileSign);

	items = 1;
	while (off < filesize) {
		memset(buffer, 0, 512);
		file_get(fname, off, 512, buffer, &got);

		switch (step) {
		case 0: // column widths
			d = buffer;
			while (*d && *d != ',' && *d != '\n')
				d++;
			if (!*d)
				return -2;
			*d = '\0';
			i = atoi(buffer);
			cell_w[items++] = i;
			if (items == col_count) {
				step++;
				items = 1;
			}
			d += 2;
			break;

		case 1: // row heights
			d = buffer;
			while (*d && *d != ',' && *d != '\n')
				d++;
			if (!*d)
				return -2;
			*d = '\0';
			i = atoi(buffer);
			cell_h[items++] = i;
			if (items == row_count)
				step++;
			d += 2;
			break;

		case 2: // cells "col row:text"
			d = buffer;
			while (*d && *d++ != ' ')
				;
			d--;
			if (!*d)
				return -2;
			*d = '\0';
			i = atoi(buffer);
			d++;
			s = d;
			while (*d && *d++ != ':')
				;
			d--;
			if (!*d)
				return -2;
			*d = '\0';
			j = atoi(s);
			d++;
			k = d;
			while (*d && *d++ != '\n')
				;
			d--;
			*d = '\0';
			d += 2;
			cells[i][j] = malloc(strlen(k) + 1);
			strcpy(cells[i][j], k);
		}
		off += d - buffer - 1;
	}
	return 1;
}

void freeBuffer(void)
{
	int i, j;

	if (!buffer)
		return;
	for (i = 0; i < buf_col; i++) {
		for (j = 0; j < buf_row; j++)
			if (buffer[i][j])
				free(buffer[i][j]);
		free(buffer[i]);
	}
	free(buffer);
	buffer = NULL;
	buf_row = buf_col = 0;
}

/* ===================== formula evaluation ===================== */

static int abort_calc = 0;
static cell_list *last_dep;

// resolve a cell reference to its numeric value (variable callback)
static double calc_callback(char *str)
{
	int i, j, x, y;
	double hold = 0.0;

	if (abort_calc == 1)
		return 0.0;

	if (*str == '$')
		str++;
	for (i = 0; i < strlen(str); i++)
		if (str[i] >= '0' && str[i] <= '9')
			break;
	if (i > 0 && str[i - 1] == '$')
		i--;
	if (i == strlen(str)) {
		abort_calc = 1;
		serror(ERR_BADVARIABLE);
		return 0.0;
	}
	x = -1;
	for (j = 1; j < col_count; j++)
		if (str[0] == cells[j][0][0] && ((i == 1) || (str[1] == cells[j][0][1]))) {
			x = j;
			break;
		}
	if (str[i] == '$')
		i++;
	y = -1;
	for (j = 1; j < row_count; j++)
		if (strcmp(str + i, cells[0][j]) == 0) {
			y = j;
			break;
		}
	if (x == -1 || y == -1) {
		abort_calc = 1;
		serror(ERR_BADVARIABLE);
		return 0.0;
	}

	if (values[x][y]) {
		if (values[x][y][0] == '#') {
			serror(ERR_BADVARIABLE);
			abort_calc = 1;
		} else
			hold = convert(values[x][y], NULL);
	} else if (cells[x][y]) {
		hold = convert(cells[x][y], NULL);
		if (convert_error == ERROR || convert_error == ERROR_END) {
			serror(ERR_BADVARIABLE);
			abort_calc = 1;
		}
	} else {
		serror(ERR_BADVARIABLE);
		abort_calc = 1;
	}
	return hold;
}

// collect the cells a formula depends on (variable callback)
static double depend_callback(char *str)
{
	cell_list *cur;
	int i, j, x, y;

	if (abort_calc == 1)
		return 0.0;

	if (*str == '$')
		str++;
	for (i = 0; i < strlen(str); i++)
		if (str[i] >= '0' && str[i] <= '9')
			break;
	if (i > 0 && str[i - 1] == '$')
		i--;
	if (i == strlen(str)) {
		abort_calc = 1;
		serror(ERR_BADVARIABLE);
		return 0.0;
	}
	x = -1;
	for (j = 1; j < col_count; j++)
		if (str[0] == cells[j][0][0] && ((i == 1) || (str[1] == cells[j][0][1]))) {
			x = j;
			break;
		}
	if (str[i] == '$')
		i++;
	y = -1;
	for (j = 1; j < row_count; j++)
		if (strcmp(str + i, cells[0][j]) == 0) {
			y = j;
			break;
		}
	if (x == -1 || y == -1) {
		abort_calc = 1;
		serror(ERR_BADVARIABLE);
		return 0.0;
	}
	cur = malloc(sizeof(cell_list));
	cur->x = x;
	cur->y = y;
	cur->next = last_dep;
	last_dep = cur;

	return 0.0;
}

static cell_list *find_depend(char *str)
{
	double hold;
	last_dep = NULL;
	find_var = &depend_callback;
	set_exp(str);
	get_exp(&hold);
	return last_dep;
}

static int is_in_list(cell_list *c1, cell_list *c2)
{
	cell_list *p = c2;
	while (p) {
		if (c1->x == p->x && c1->y == p->y)
			return 1;
		p = p->next;
	}
	return 0;
}

void calculate_values(void)
{
	cell_list ***depend = NULL;
	cell_list *first = NULL;
	cell_list *sorted = NULL, *sorted_last = NULL;
	cell_list *p = NULL;
	int i, j;

	abort_calc = 0;
	depend = malloc(col_count * sizeof(void *));
	for (i = 0; i < col_count; i++) {
		depend[i] = malloc(row_count * sizeof(void *));
		for (j = 0; j < row_count; j++) {
			if (values[i][j])
				free(values[i][j]);
			values[i][j] = NULL;

			if (cells[i][j] && cells[i][j][0] == '=') {
				cell_list *cur;
				depend[i][j] = find_depend(cells[i][j] + 1); // skip '='
				if (abort_calc) {
					values[i][j] = malloc(2);
					values[i][j][0] = '#';
					values[i][j][1] = '\0';
					abort_calc = 0;
					continue;
				}
				cur = malloc(sizeof(cell_list));
				cur->x = i;
				cur->y = j;
				cur->next = first;
				first = cur;
			}
		}
	}

	if (!first || abort_calc)
		goto free_memory;

	// topological sort: repeatedly pull a cell that nothing pending depends on
	while (first) {
		cell_list *prev = NULL, *mn = first;
		int is_min = 1;

		while (mn) {
			cell_list *q = first;
			is_min = 1;
			while (q && is_min) {
				if (is_in_list(q, depend[mn->x][mn->y]))
					is_min = 0;
				q = q->next;
			}
			if (is_min)
				break;
			prev = mn;
			mn = mn->next;
		}
		if (!is_min) {
			abort_calc = 1;
			goto free_memory; // cyclic dependency
		}
		if (prev == NULL)
			first = first->next;
		else
			prev->next = mn->next;

		if (sorted == NULL) {
			sorted = mn;
			sorted_last = mn;
		} else {
			sorted_last->next = mn;
			sorted_last = mn;
			mn->next = NULL;
		}
	}

	// evaluate in dependency order
	p = sorted;
	while (p) {
		double d;
		abort_calc = 0;
		set_exp(cells[p->x][p->y] + 1); // skip '='
		find_var = &calc_callback;
		if (get_exp(&d)) {
			char *new_val = ftoa(d);
			if (values[p->x][p->y] && strcmp(values[p->x][p->y], new_val) == 0) {
				free(new_val);
			} else {
				if (values[p->x][p->y])
					free(values[p->x][p->y]);
				values[p->x][p->y] = new_val;
			}
		} else {
			values[p->x][p->y] = malloc(2);
			values[p->x][p->y][0] = '#';
			values[p->x][p->y][1] = '\0';
		}
		p = p->next;
	}

free_memory:
	p = sorted;
	while (p) {
		cell_list *tmp = p->next;
		cell_list *pp = depend[p->x][p->y];
		while (pp) {
			cell_list *pn = pp->next;
			free(pp);
			pp = pn;
		}
		free(p);
		p = tmp;
	}

	for (i = 0; i < col_count; i++)
		free(depend[i]);
	free(depend);
}
