#ifndef FILES_CSV_H
#define FILES_CSV_H

// CSV, the table in a file: the cells from A1 on as they were typed, the
// formulas too; a field in quotes may hold the separator, a line end and
// "" for a quote. file.c reads and writes the files.

// text built up for a file, in UTF-8 or cp866; failed when memory ran out
typedef struct {
	char *data;
	int len, size, failed;
	int utf8; // the texts of the cells, cp866, written in UTF-8
} Out;

void out_text(Out *o, const char *s);

// the cells from p, which it changes, separated by sep; 0 if some of
// them did not fit: past CZ or row 100, or longer than a cell holds
int csv_load(char *p, char sep);
// the separator of a file at p: ';' if its first line has one out of
// quotes, else ','. A CSV of Excel in a language with a decimal comma
// separates by ';' and leaves the commas of its numbers unquoted
char csv_separator(const char *p);
// the cells, separated by sep, each line ended by eol
void csv_save(Out *o, char sep, const char *eol);

#endif
