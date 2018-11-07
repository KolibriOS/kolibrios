

#include "func.h"
#include "parser.h"
#include "calc.h"
#include "kosSyst.h"
//#include "KosFile.h"

#define DEFAULT_CELL_W 82
#define DEFAULT_CELL_H 21

extern DWORD col_count, row_count;
extern char ***cells;
extern DWORD *cell_w, *cell_h;
extern char ***values;

extern DWORD *cell_x, *cell_y;

// буфер обмена
extern char ***buffer;
extern DWORD buf_col, buf_row;
extern DWORD buf_old_x, buf_old_y;

extern bool sel_moved;

extern struct GRID
{
	int x,y,w,h;
} grid;

int cf_x0, cf_x1, cf_y0, cf_y1;


#define sign(x) ((x) < 0 ? -1 : ((x) == 0 ? 0 : 1))


//extern const char er_file_not_found[];
//extern const char er_format[];
extern const char *sFileSign;

struct cell_list
{
	int x,y;
	cell_list *next;
};


// получить х-координату ячейки с номером х
int get_x(int x)
{
	int i, r = 0;
	if (x > col_count) 
		x = col_count;
	for (i = 0; i < x; i++)
		r+=cell_w[i];
	return r;
}

// аналог
int get_y(int y)
{
	int i, r = 0;
	if (y > row_count) 
		y = row_count;
	for (i = 0; i < y; i++)
		r+=cell_h[i];
	return r;
}



// сгенерить заголовок столбца
char *make_col_cap(int i)
{
	char *r = (char*)allocmem(3);
	if (i <= 26)
	{
		r[0] = 'A' + i - 1;
		r[1] = '\0';
		return r;
	}
	else if (i % 26 == 0)	// фикс бага который не понял - да простят меня читатели и юзера
	{
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

// -"- строки
char *make_row_cap(int i)
{
	char *r = (char*)allocmem(3);
	if (i <= 9)
	{
		r[0] = '0' + i;
		r[1] = '\0';
		return r;
	}
	r[0] = (i / 10) + '0';
	r[1] = (i % 10) + '0';
	r[2] = '\0';
	return r;
}

// инициализация ячеек
void init()
{
	int i, j;

	cell_w = (DWORD*)allocmem(col_count * sizeof(DWORD));
	cell_h = (DWORD*)allocmem(row_count * sizeof(DWORD));
	cell_x = (DWORD*)allocmem(col_count * sizeof(DWORD));
	cell_y = (DWORD*)allocmem(row_count * sizeof(DWORD));
	for (i = 0; i < col_count; i++)
	{
		cell_w[i] = DEFAULT_CELL_W;
	}
	cell_w[0] = 30; //make row headers smaller

	for (i = 0; i < row_count; i++)
	{
		cell_h[i] = DEFAULT_CELL_H;
	}

	cells = (char***)allocmem(col_count * sizeof(char**));
	values = (char***)allocmem(col_count * sizeof(char**));
	for (i = 0; i < col_count; i++)
	{
		cells[i] = (char**)allocmem(row_count * sizeof(char*));
		values[i] = (char**)allocmem(row_count * sizeof(char*));
		for (j = 0; j < row_count; j++)
		{
			cells[i][j] = NULL;
			if (i == 0 && j)
			{
				cells[i][j] = make_row_cap(j);
			}
			else if (j == 0 && i)
			{
				cells[i][j] = make_col_cap(i);
			}
		}
	}
}

void reinit()
{
	int i, j;

	for (i = 0; i < col_count; i++)
	{
		cell_w[i] = DEFAULT_CELL_W;
	}
	cell_w[0] = 30; //make row headers smaller

	for (i = 0; i < row_count; i++)
	{
		cell_h[i] = DEFAULT_CELL_H;
	}

	for (i = 1; i < col_count; i++)
	{
		for (j = 1; j < row_count; j++)
		{
			if (cells[i][j])
				freemem(cells[i][j]);
			cells[i][j] = NULL;
			if (values[i][j])
				freemem(values[i][j]);
			values[i][j] = NULL;
		}
	}
}

void fill_cells(int sel_x, int sel_y, int sel_end_x, int sel_end_y, int old_end_x, int old_end_y)
{
	// итак, (sel_x, sel_y) :: (old_end_x, old_end_y) - источник
	// результат хранится либо в строке sel_x .. sel_end_x, либо в столбце sel_y .. sel_end_y
	
	int i, start, end, step, gdir = -1;
	int pdir = -1;
	char *source;

	cf_x0 = cf_y0 = 0;
	cf_x1 = col_count;
	cf_y1 = row_count;

	if (sel_end_x == -1)
		sel_end_x = sel_x;
	if (sel_end_y == -1)
		sel_end_y = sel_y;

	// если направления выделений перпендикулярны, то просто в цикле повторяем то же, что для 1 ячейки:

	if (old_end_x == sel_end_x && sel_y == old_end_y)
	{
		gdir = 0;
	}
	else if (old_end_y == sel_end_y && sel_x == old_end_x)
	{
		gdir = 1;
	}

	//sprintf(debuf, "fuck in ass %U %U %U %U %U %U dir %U",sel_x,sel_y,sel_end_x,sel_end_y,old_end_x,old_end_y,gdir);
	//rtlDebugOutString(debuf);
	if (gdir != -1)
	{
		int gstep = gdir ? sign(old_end_y - sel_y) : sign(old_end_x - sel_x);
		if (gstep == 0)
		{
		/*	if (gdir)
			{
				//old_end_y += 1;
			}
			else
			{
				//old_end_x += 1;
			}
		*/
			gstep = 1;
		}

		for (;gdir ? (sel_y != old_end_y + gstep) : (sel_x != old_end_x + gstep); 
			gdir ? (sel_y += gstep) : (sel_x += gstep))
		{
			//sprintf(debuf, "cycle %U %U %U %U %U %U dir %U",sel_x,sel_y,sel_end_x,sel_end_y,old_end_x,old_end_y,gdir);
			//rtlDebugOutString(debuf);
			int dir;
			source = cells[sel_x][sel_y];
			if (gdir == 0)
			{
				start = sel_y;
				end = sel_end_y;
				step = (sel_y < sel_end_y ? 1 : -1);
				dir = 1;
			}
			else
			{
				start = sel_x;
				end = sel_end_x;
				step = (sel_x < sel_end_x ? 1 : -1);
				dir = 0;
			}

			//sprintf(debuf, "cyc %U %U %U %U",start,end,step,dir);
			//rtlDebugOutString(debuf);
			for (i = start + step; i != end + step; i += step)
			{
				//char **p = &cells[dir ? sel_x : i][dir ? i : sel_end_y];
				//sprintf(debuf, "to %U %U dir %U copying '%S'",dir ? sel_x : i,dir ? i : sel_y,dir,source);
				//rtlDebugOutString(debuf);
				if (cells[dir ? sel_x : i][dir ? i : sel_y])
				{
					freemem(cells[dir ? sel_x : i][dir ? i : sel_y]);
				}
				if (source)
				{
					cells[dir ? sel_x : i][dir ? i : sel_y] = change_formula(source, dir ? 0 : (i - start), dir ? (i - start) : 0);
					//cells[dir ? sel_x : i][dir ? i : sel_y] = (char *)allocmem(strlen(source) + 1);
					//strcpy(cells[dir ? sel_x : i][dir ? i : sel_y], source);
				}
				else
					cells[dir ? sel_x : i][dir ? i : sel_y] = NULL;
			}
		}
	}

	// а вот если параллельны...
	/*
	
	if (sel_x == sel_end_x && sel_x == old_end_x)
	{
		pdir = 0;
	}
	if (sel_y == sel_end_y && sel_y == old_end_y)
	{
		pdir = 1;
	}
	if (pdir != -1)
	{
		// арифметическая прогрессия - если числа. и тупо размножитьт последнее, если нет

		sprintf(debuf, "maybe arith dir %U", pdir);
		rtlDebugOutString(debuf);

		int is_arith = 1;
		int gstep = pdir ? sign(old_end_y - sel_y) : sign(old_end_x - sel_x);
		if (gstep == 0)
			gstep = 1;

		for (int i = pdir ? sel_y : sel_x; i != pdir ? (old_end_y + gstep) : (old_end_x + gstep); i++)
		{
			convert_error = 0;
			sprintf(debuf,"cell %U %U", !pdir ? sel_x : i, !pdir ? i : sel_y);
			rtlDebugOutString(debuf);
			if (cells[!pdir ? sel_x : i][!pdir ? i : sel_y])
			{
				double d = atof(cells[!pdir ? sel_x : i][!pdir ? i : sel_y]);
				if (convert_error)
				{
					rtlDebugOutString("failed arith");
					is_arith = 0;
					break;
				}
			}
			else
			{
				is_arith = 0;
				rtlDebugOutString("failed arith in null");
				break;
			}
		}

		double arith_first, arith_step;
		if (is_arith)
		{
			rtlDebugOutString("really arith");
			arith_first = atof(cells[sel_x][sel_y]);
			arith_step = atof(cells[pdir ? sel_x : old_end_x][pdir ? sel_y : old_end_y]) - arith_first;
			arith_first += arith_step * pdir ? abs(sel_end_x - old_end_x) : abs(sel_end_y - old_end_y);
		}
		else
			rtlDebugOutString("none arith");

		// собственно заполнение
		for (i = pdir ? old_end_y : old_end_x; i != pdir ? (sel_end_y + gstep) : (sel_end_x + gstep); i++)
		{
			if (cells[pdir ? sel_x : i][pdir ? i : sel_y])
				freemem(cells[pdir ? sel_x : i][pdir ? i : sel_y]);
			if (is_arith)
			{
				cells[pdir ? sel_x : i][pdir ? i : sel_y] = ftoa(arith_first);
				arith_first += arith_step;
			}
			else
			{
				if (cells[sel_x][sel_y])
				{
					cells[pdir ? sel_x : i][pdir ? i : sel_y] = (char*)allocmem(strlen(cells[sel_x][sel_y]) + 1);
					strcpy(cells[pdir ? sel_x : i][pdir ? i : sel_y], cells[sel_x][sel_y]);
				}
			}
		}
	}
	*/

	calculate_values();
}

int Kos_FileWrite(kosFileInfo &fileInfo, char *line, int mode = 3) // если mode = 2 - перезаписать файл
{
	int res = 0;
	fileInfo.dataCount = strlen(line);
	fileInfo.bufferPtr = (Byte*)line;
	fileInfo.rwMode = mode;
	res = kos_FileSystemAccess(&fileInfo);
	if (res != 0)
		return 0;
	fileInfo.OffsetLow += fileInfo.dataCount;
	return 1;
}

int SaveCSV(char *fname)
{
	int i, j;
	int min_col = col_count, min_row = row_count, max_row = -1, max_col = -1;
	int first = 1;

	kosFileInfo fileInfo;
	memset((Byte*)&fileInfo, 0, sizeof(fileInfo));
	strcpy(fileInfo.fileURL,fname);
	fileInfo.OffsetLow = 0;
	fileInfo.OffsetHigh = 0;
	fileInfo.rwMode = 8;	// delete

	rtlDebugOutString("savecsv: old file deleted");

	for (i = 1; i < col_count; i++)
	{
		for (j = 1; j < row_count; j++)
		{
			if (cells[i][j])
			{
				min_col = min(min_col, i);
				min_row = min(min_row, j);
				max_col = max(max_col, i);
				max_row = max(max_row, j);
			}
		}
	}

	sprintf(debuf, "col %U %U row", min_col, max_col, min_row, max_row);
	rtlDebugOutString(debuf);

	for (j = min_row; j <= max_row; j++)
	{
		char buffer[1024]; // не надо так делать
		int buf_len = 0;

		memset((Byte*)buffer, 0, 1024);

		for (i = min_col; i <= max_col; i++)
		{
			char *cur = values[i][j] ? values[i][j] : cells[i][j];
			if (cur)
			{
				buffer[buf_len++] = '\"';
				for (int k = 0; k < strlen(cur); k++)
				{
					if (cur[k] == '\"')
						buffer[buf_len++] = '\"';	// кавычек - по две
					buffer[buf_len++] = cur[k];
				}
				buffer[buf_len++] = '\"';
			}
			buffer[buf_len++] = ',';
		}
		rtlDebugOutString(buffer);
		// очередная строка теперь в буфере
		buffer[buf_len++] = '\n';
		if (!Kos_FileWrite(fileInfo, buffer, first ? (first = 0, 2) : 3))
			return 0;
	}
	return 1;
}

int str_is_csv(char *str)
{
	int str_len = strlen(str);
	if (str_len >= 5) {
		if ( strnicmp(str + str_len - 4, ".CSV", 4) == 0) return 1; 
	}
	return 0;
}

#define BUF_FOR_ALL 5000
int SaveFile(char *fname)
{
	kosFileInfo fileInfo;
	char *buffer = (char*)allocmem(BUF_FOR_ALL);	// ужас! но пока что достаточно
	int filePointer = 0;

	int i,j;
	Dword res;

	if (str_is_csv(fname))
		return SaveCSV(fname);


	//rtlDebugOutString(fname);

	memset((Byte*)&fileInfo, 0, sizeof(fileInfo));
	strcpy(fileInfo.fileURL,fname);
	fileInfo.OffsetLow = 0;
	fileInfo.OffsetHigh = 0;
	fileInfo.rwMode = 8;
	res = kos_FileSystemAccess(&fileInfo);	// удалить
	fileInfo.dataCount = strlen(sFileSign);
	fileInfo.bufferPtr = (Byte*)sFileSign;
	fileInfo.rwMode = 2;
	res = kos_FileSystemAccess(&fileInfo);
	if (res != 0)
		return 0;
	//sprintf(debuf, "create %U",res);
	//rtlDebugOutString(debuf);
	fileInfo.OffsetLow += fileInfo.dataCount;

	// ширину столбцов сохраняем
	memset((Byte*)buffer,0,BUF_FOR_ALL);
	for (i = 1; i < col_count; i++)
	{
		char smalbuf[32];
		memset((Byte*)smalbuf,0,32);
		sprintf(smalbuf, "%U,", cell_w[i]);
		strcpy(buffer+strlen(buffer),smalbuf);
	}
	buffer[strlen(buffer)-1] = '\n';	// заменили последнюю запятую на перевод строки
	//rtlDebugOutString(buffer);
	fileInfo.dataCount = strlen(buffer);
	fileInfo.bufferPtr = (Byte*)buffer;
	fileInfo.rwMode = 3;
	res = kos_FileSystemAccess(&fileInfo);
	if (res != 0)
		return 0;

	// перемотать забыл я этот файл
	// но уж теперь не попадусь на это!
	fileInfo.OffsetLow += fileInfo.dataCount;

	// высоту строк сохраняем в файле мы
	memset((Byte*)buffer,0,BUF_FOR_ALL);
	for (i = 1; i < row_count; i++)
	{
		char smalbuf[32];
		memset((Byte*)smalbuf,0,32);
		sprintf(smalbuf, "%U,", cell_h[i]);
		strcpy(buffer+strlen(buffer),smalbuf);
	}
	buffer[strlen(buffer)-1] = '\n';	// заменили последнюю запятую на перевод строки
	//rtlDebugOutString(buffer);
	fileInfo.dataCount = strlen(buffer);
	fileInfo.bufferPtr = (Byte*)buffer;
	fileInfo.rwMode = 3;
	res = kos_FileSystemAccess(&fileInfo);
	if (res != 0)
		return 0;

	// и вновь перемотаю я сей файл
	
	fileInfo.OffsetLow += fileInfo.dataCount;
	memset((Byte*)buffer,0,BUF_FOR_ALL);

	// сохранили параметры ячеек мы, сохраняем содержимое их теперь

	for (i = 1; i < row_count; i++)
	{
		for (j = 1; j < col_count; j++)
			if (cells[j][i])
			{
				memset((Byte*)buffer,0,512);
				sprintf(buffer, "%U %U:%S\n", j, i, cells[j][i]);
				fileInfo.dataCount = strlen(buffer);
				fileInfo.bufferPtr = (Byte*)buffer;
				fileInfo.rwMode = 3;
				res = kos_FileSystemAccess(&fileInfo);
				if (res != 0)
					return 0;
				//sprintf(debuf, "create %U",res);
				//rtlDebugOutString(debuf);
				fileInfo.OffsetLow += fileInfo.dataCount;
			}
	}

	//rtlDebugOutString("saving finished");

	freemem(buffer);
	return 1;
}

char *Kos_FileRead(kosFileInfo &fileInfo, int &code)
{
	char buffer[512], *p, *r;
	fileInfo.dataCount = 512;
	fileInfo.rwMode = 0;
	fileInfo.bufferPtr = (Byte *)buffer;
	memset((Byte*)buffer, 0, 512);
	int z = kos_FileSystemAccess(&fileInfo);
	code = z;

	//sprintf(debuf, "kos file read %U", code);
	//rtlDebugOutString(debuf);

	if (z != 0 && z != 6)
		return NULL;

	p = buffer;
	while (*p && *p++ != '\n');

	if (p == buffer)
		return NULL;

	r = (char*)allocmem(p - buffer);
	memset((Byte*)r, 0, p - buffer);
	//strncpy(r, buffer, p - buffer);
	for (int l = 0; l < p - buffer - 1; l++)
		r[l] = buffer[l];
	fileInfo.OffsetLow += p - buffer;
	return r;
}

char GetCsvSeparator(char *fname)
{
	char buffer[512];
	kosFileInfo fileInfo;

	rtlDebugOutString(fname);

	strcpy(fileInfo.fileURL, fname);
	fileInfo.OffsetLow = 0;
	fileInfo.OffsetHigh = 0;
	fileInfo.dataCount = 512;
	fileInfo.rwMode = 0;
	fileInfo.bufferPtr = (Byte *)buffer;
	
	if (kos_FileSystemAccess(&fileInfo) == 0) {
		int separ_coma = chrnum(buffer, ',');
		int separ_semicolon = chrnum(buffer, ';');
		//kos_DebugValue(",", separ_coma);
		//kos_DebugValue(";", separ_semicolon);
		if (separ_semicolon>separ_coma) return ';';
	}
	return ',';
}

int LoadCSV(char *fname)
{
	// clear the table
	reinit();

	kosFileInfo fileInfo;
	strcpy(fileInfo.fileURL,fname);
	fileInfo.OffsetLow = 0;
	fileInfo.OffsetHigh = 0;

	char separator = GetCsvSeparator(fileInfo.fileURL);

	char *line;

	int col = 1, row = 1;
	int code = 0;
	do 
	{
		line = Kos_FileRead(fileInfo, code);
		if (!line || *line == '\0' || (code != 0 && code != 6))
		{
			sprintf(debuf, "read end, line not null = %U, code = %U", !line, code);
			rtlDebugOutString(debuf);
			break;
		}
		sprintf(debuf, "read '%S' len %U", line, strlen(line));
		rtlDebugOutString(debuf);

		// разборать строку
		// выделить ;, причем вне "
		int i = 0;
		while (i <= strlen(line))
		{
			int inPar = 0;
			// inPar: 0 - не кавычки, 1 - только что была кавычка, 2 - кавычка была, но давно
			int start = i;
			while (i <= strlen(line))
			{
				char c = line[i];
				if (!c)
					c = separator; 
				int yes_semicolon = 0;

				switch (inPar)
				{
					case 0:
						if (c == '\"')
						{
							inPar = 1;
						}
						else
						{
							if (c == separator)
								yes_semicolon = 1;
						}
						break;
					case 1:
						inPar = 2;
						break;
					case 2:
						if (c == '\"')	// она закрылась
						{
							inPar = 0;
						}
						/*else
						{
							if (c == separator)
								yes_semicolon = 1;

						}*/
						break;
				}
				if (yes_semicolon)
				{
					// итак, line[i] = separator
					int tmp = line[start] == '"' ? 1 : 0;
					int sz = i - start - tmp * 2;
					if (sz > 0)
					{
						cells[col][row] = (char *)allocmem(sz + 1);
						memset((Byte*)cells[col][row], 0, sz + 1);
						int m = 0;
						for (int l = 0; l < sz; l++)
						{
							if (line[start + tmp + l] == '\"')
							{
								cells[col][row][m++] = '\"';
								l++;	// пропустить следующую кавычку
							}
							else
								cells[col][row][m++] = line[start + tmp + l];
						}
						sprintf(debuf, "set %U %U = '%S'", col, row, cells[col][row]);
						rtlDebugOutString(debuf);
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

	} while(line);

	return 1;
}


int LoadFile(char *fname)
{
	kosFileInfo fileInfo;
	kosBDVK bdvk;
	int filePointer = 0, i, j;
	Dword res, filesize;
	char buffer[512 + 1];
	char *d, *s, *k;
	int step = 0, items;

	strcpy(fileInfo.fileURL,fname);
	fileInfo.OffsetLow = 0;
	fileInfo.OffsetHigh = 0;

	fileInfo.rwMode = 5;
	fileInfo.bufferPtr = (Byte *)&bdvk;
	Dword rr = kos_FileSystemAccess(&fileInfo); // в CKosFile нет определения размера
	//sprintf(debuf, "getsize: %U\n", rr);
	//rtlDebugOutString(debuf);
	if (rr != 0)
	{
		return -1;
	}

	if (str_is_csv(fname))
		return LoadCSV(fname);


	// clear the table
	reinit();

	filesize = bdvk.size_low;

	fileInfo.rwMode = 0;
	fileInfo.dataCount = strlen(sFileSign);
	fileInfo.bufferPtr = (Byte*)buffer;
	kos_FileSystemAccess(&fileInfo);
	s = (char*)sFileSign;
	d = buffer;
	while (*s && *d && *s++==*d++);		// застрелите меня
	if (*s != '\0' || *d != '\0')
	{
		return -2;
	}
	fileInfo.OffsetLow += fileInfo.dataCount;
	items = 1;
	while (fileInfo.OffsetLow < filesize)
	{
		// так прочитали ли мы ширину всех стоблцов, и длину всех строк прочитали ли мы?
		fileInfo.dataCount = 512;
		memset((Byte*)buffer, 0, 512);
		kos_FileSystemAccess(&fileInfo);
		//sprintf(debuf, "%U", fileInfo.OffsetLow);
		//rtlDebugOutString(debuf);
		//sprintf(debuf, "buffer: %S", buffer);
		//rtlDebugOutString(debuf);
		// что я увижу на доске отладки
		// то мне поможет в жизненном пути
		// смогу тогда своей ошибки гадкой
		// причину непосредственно найти

		switch (step)
		{
		case 0:			// стоблцы
			d = buffer;
			while (*d && *d != ',' && *d != '\n') d++;	
			//d--;
			if (!*d)
			{	
				return -2;
			}
			*d = '\0';
			i = atoi(buffer);
			cell_w[items++] = i;
			if (items == col_count)
			{
				step++;
				items = 1;	//	теперь высоты строк читать мы будем смело
							//  чтоб их восстановить и было как всегда
				//sprintf(debuf, "col_count read done last buf %S file pos %U",buffer,fileInfo.OffsetLow);
				//rtlDebugOutString(debuf);
			}
			d+=2;
			break;

		case 1:			// строки, коих высота записана
			d = buffer;
			while (*d && *d != ',' && *d != '\n') d++;	
			//d--;
			if (!*d)
			{	
				//sprintf(debuf,"oh shit, error at %U",items);
				//rtlDebugOutString(debuf);
				return -2;
			}
			*d = '\0';
			i = atoi(buffer);
			cell_h[items++] = i;
			/*if (items > 5)
			{
				sprintf(debuf, "set row from %S hei %U %U",buffer,items-1,i);
				rtlDebugOutString(debuf);
			}*/

			if (items == row_count)
			{
				step++;		// а далее лежат ячейки в файле
							// записаны они в кривом формате
							// ибо писал сей код я темной ночью
							// но не курил травы, клянусь я вам
							// иначе бы и этого не скодил

							// дебажить сей мне код премного впадлу
							// но помню правило - коль написал дебажь
							// немедленно - а то нах все забудешь.
							// вот выпью - а там сразу за отладку.
				//sprintf(debuf, "before read cells offset %U %X",fileInfo.OffsetLow,fileInfo.OffsetLow);
				//rtlDebugOutString(debuf);
			}
			d+=2;
			break;

			// о, бряки я забыл забить. о ужас.
			// позор мне, на костре меня сожгите
			// ведь тот, кто break не ставит после casa
			// подобен ламеру, что си не знает
			// смогу ли я такое пережить?

		case 2:			// ячейки, ибо их содержимое сохранено здесь от исчезновения
			d = buffer;
			while (*d && *d++ != ' ');	// оужжас. зачем только я писал этот бред....
			d--;
			if (!*d)
			{	
				return -2;
			}
			*d = '\0';
			i = atoi(buffer);
			d++;
			s=d;
			while (*d && *d++ != ':');	// когда-то я удивлялся, как люди могут такую херню писать... дожил
			d--;
			if (!*d)
			{	
				return -2;
			}
			*d = '\0';
			j = atoi(s);
			//rtlDebugOutString(s);
			d++;
			k = d;
			while (*d && *d++ != '\n');
			d--;
			*d = '\0';
			d+=2;
			//sprintf(debuf, "i:%U j:%U d:%S\n",i,j,k);
			//rtlDebugOutString(debuf);
			cells[i][j] = (char*)allocmem(strlen(k) + 1);
			//memset(cells[i][j], 0, strlen(k) + 1);
			strcpy(cells[i][j], k);
			//sprintf(debuf, "offset: %U", fileInfo.OffsetLow);
			//rtlDebugOutString(debuf);
		}
		fileInfo.OffsetLow += d - (char*)buffer - 1;
	}
	//rtlDebugOutString("loading finished");
	return 1;
}

// очистить буфер обмена
void freeBuffer()
{
	int i, j;

	if (!buffer)
		return;
	for (i = 0; i < buf_col; i++)
	{
		for (j = 0; j < buf_row; j++)
			if (buffer[i][j])
				freemem(buffer[i][j]);
		freemem(buffer[i]);
	}
	freemem(buffer);
	buffer = NULL;
	buf_row = buf_col = 0;

}


// далее - вычисление по формулам

int abort_calc = 0;
cell_list *last_dep;

// ппц, где то баг, а это типа фикс
//#define allocmem2(x) allocmem(x+1000)

double calc_callback(char *str)
{
	int i,j,x,y;

	if (abort_calc == 1)
		return 0.0;

	//rtlDebugOutString(str);
	if (*str == '$') str++;
	for (i = 0; i < strlen(str); i++)
		if (str[i] >= '0' && str[i] <= '9')
			break;
	if (str[i-1] == '$')
		i--;
	if (i == strlen(str))
	{
		abort_calc = 1;
		serror(ERR_BADVARIABLE);
		return 0.0;
	}
	x = -1;
	for (j = 0; j < col_count; j++)
//		if (strnicmp(str,cells[j][0],i-1)==0)
		if (str[0] == cells[j][0][0] && ((i == 1) || (str[1] == cells[j][0][1])))
		{
			x = j;
			break;
		}
	if (str[i] == '$')
		i++;
	y = -1;
	for (j = 0; j < row_count; j++)
		if (strcmp(str+i,cells[0][j])==0)
		{
			y = j;
			break;
		}
	if (x == -1 || y == -1)
	{
		abort_calc = 1;
		serror(ERR_BADVARIABLE);
		return 0.0;
	}

	double hold;
	if (values[x][y])
		if (values[x][y][0] == '#')
		{
			serror(ERR_BADVARIABLE);
			abort_calc = 1;
		}
		else
		{
			hold = atof(values[x][y]);
			//if (convert_error)				// нереальный случай...
			//{
			//	serror(ERR_BADVARIABLE);
			//	abort_calc = 1;
			//}
		}
	else
	{
		if (cells[x][y])
		{
			hold = atof(cells[x][y]);
			if (convert_error == ERROR || convert_error == ERROR_END)
			{
				serror(ERR_BADVARIABLE);
				abort_calc = 1;
			}
		}
		else
		{
			sprintf(debuf, "bad var %S", str);
			rtlDebugOutString(debuf);
			serror(ERR_BADVARIABLE);
			abort_calc = 1;
		}
	}
	return hold;
}

double depend_callback(char *str)
{
	cell_list *cur;
	// надо выдрать из АВ47 значения х и у.
	int i,j,x,y;

	if (abort_calc == 1)
		return 0.0;

	if (*str == '$') str++;
	for (i = 0; i < strlen(str); i++)
		if (str[i] >= '0' && str[i] <= '9')
			break;
	if (str[i-1] == '$')
		i--;
	if (i == strlen(str))
	{
		abort_calc = 1;
		serror(ERR_BADVARIABLE);
		return 0.0;
	}
	x = -1;
	for (j = 1; j < col_count; j++)
		//if (strncmp(str,cells[j][0],i)==0)
		if (str[0] == cells[j][0][0] && ((i == 1) || (str[1] == cells[j][0][1])))
		{
			x = j;
			break;
		}
	if (str[i] == '$')
		i++;

	y = -1;
	for (j = 1; j < row_count; j++)
		if (strcmp(str+i,cells[0][j])==0)
		{
			y = j;
			break;
		}
	if (x == -1 || y == -1)
	{
		abort_calc = 1;
		serror(ERR_BADVARIABLE);
		return 0.0;
	}
	cur = (cell_list*)allocmem(sizeof(cell_list));
	cur->x = x;
	cur->y = y;
	cur->next = last_dep;
	last_dep = cur;

	return 0.0;
}

cell_list *find_depend(char *str)
{
	double hold;
	last_dep = NULL;
	find_var = &depend_callback;
	set_exp(str);
	get_exp(&hold);

	return last_dep;
}

bool is_in_list(cell_list *c1, cell_list *c2)
{
	cell_list *p = c2;
	while (p)
	{
		if (c1->x == p->x && c1->y == p->y)
			return 1;
		p = p->next;
	}
	return 0;
}

void calculate_values()
{
	cell_list ***depend = NULL;
	cell_list *first = NULL;
	cell_list *sorted = NULL, *sorted_last = NULL;
	cell_list *p = NULL;
	int i,j;

	//rtlDebugOutString("calc");

	abort_calc = 0;
	depend = (cell_list***)allocmem(col_count * sizeof(void*));
	for (i = 0; i < col_count; i++)
	{
		depend[i] = (cell_list**)allocmem(row_count * sizeof(void*));
		for (j = 0; j < row_count; j++)
		{
			if (values[i][j])
				freemem(values[i][j]);
			values[i][j] = NULL;

			if (cells[i][j] && cells[i][j][0] == '=')
			{
				depend[i][j] = find_depend(cells[i][j] + 1);		// после =
				if (abort_calc)
				{
					values[i][j] = (char*)allocmem(2);
					values[i][j][0] = '#';
					values[i][j][1] = '\0';
					abort_calc = 0;
					continue;
				}
				cell_list *cur;
				cur = (cell_list*)allocmem(sizeof(cell_list));
				cur->x = i;
				cur->y = j;
				cur->next = first;	// вставили тек. ячейку в начало списка ячеек с формулами
				first = cur;
			}
		}
	}

	//rtlDebugOutString("depend end");
	// топологическая сортировка
	if (!first)
		goto free_memory;

	if (abort_calc)
		goto free_memory;

	while (first)
	{
		// найти наименьший элемент. если его нет - ошибка, т.к. циклическая зависимость
		cell_list *prev = NULL,*min = first;

		bool is_min;
		while (min)
		{
			cell_list *p = first;
			is_min = 1;
			while (p && is_min)
			{
				if (is_in_list(p,depend[min->x][min->y]))
					is_min = 0;
				p = p->next;
			}
			if (is_min)
				break;
			prev = min;
			min = min->next;
		}
		if (!is_min)
		{
			abort_calc = 1;
			goto free_memory;		// все плохо. ужасно. я плакаю, но пишу goto
		}
		// надо убрать минимум во второй список
		if (prev == NULL)
		{
			first = first->next;
		}
		else
		{
			prev->next = min->next;
		}
		/*
		min->next = sorted;
		sorted = min;
		*/
		if (sorted == NULL)
		{
			sorted = min;
			sorted_last = min;
		}
		else
		{
			sorted_last->next = min;
			sorted_last = min;
			min->next = NULL;
		}
	}

	// вычисление значений
	//rtlDebugOutString("sort end");

	p = sorted;
	while (p)
	{
		double d;
		abort_calc = 0;
		set_exp(cells[p->x][p->y]+1);	// все что после "="
		find_var = &calc_callback;
		if (get_exp(&d))
		{
			char *new_val = ftoa(d);
			if (values[p->x][p->y] && strcmp(values[p->x][p->y],new_val) == 0)
			{
				freemem(new_val);
			}
			else
			{
				if (values[p->x][p->y]) 
					freemem(values[p->x][p->y]);
				values[p->x][p->y] = new_val;
				sel_moved = 0;
			}
			//sprintf(debuf,"calc %U %U formula %S result %f",p->x,p->y,cells[p->x][p->y]+1,d);
			//rtlDebugOutString(debuf);
		}
		else
		{
			values[p->x][p->y] = (char*)allocmem(2);
			values[p->x][p->y][0] = '#';
			values[p->x][p->y][1] = '\0';
			//sprintf(debuf,"calc %U %U formula %S result #",p->x,p->y,cells[p->x][p->y]+1);
			//rtlDebugOutString(debuf);
		}
		p = p->next;
	}

	if (abort_calc)
		goto free_memory;

	//rtlDebugOutString("calc end");


	// освобождение памяти

free_memory:
	
	p = sorted;
	while (p)
	{
		cell_list *tmp = p->next;
		cell_list *pp = depend[p->x][p->y];
		while (pp)
		{
			cell_list *tmp = pp->next;
			freemem(pp);
			pp = tmp;
		}
		freemem(p);
		p = tmp;
	}

	for (i = 0; i < col_count; i++)
		freemem(depend[i]);
	freemem(depend);

	//rtlDebugOutString("freemem end");

	
}

int parse_cell_name(char *str, int *px, int *py, int *xd, int *yd)
{
	// надо выдрать из АВ47 значения х и у.
	int i,j,x,y,dx = 0,dy = 0;

	if (*str == '$') 
	{
		str++;
		dx = 1;
	}
	for (i = 0; i < strlen(str); i++)
		if (str[i] >= '0' && str[i] <= '9')
			break;
	if (str[i-1] == '$')
	{
		i--;
		dy = 1;
	}
	if (i == strlen(str))
	{
		return 0;
	}
	x = -1;
	for (j = 1; j < col_count; j++)
		if (strncmp(str,cells[j][0],i)==0)
	{
		/*int p = 0, z = 1;
		for (p = 0; p < i; p++)
			if (!str[p] || str[p] != cells[j][0][p])
			{
				z = 0;
				break;
			}
		if (z)
		*/
		{
			x = j;
			break;
		}
	}
	if (str[i] == '$')
		i++;
	y = -1;
	for (j = 1; j < row_count; j++)
		if (strcmp(str+i,cells[0][j])==0)
	{
			/*
		int p = 0, z = 1;
		for (p = 0;; p++)
		{
			if (str[i + p] != cells[0][j][p])
			{
				z = 0;
				break;
			}
			if (cells[0][j][p] == '\0')
				break;
		}
		if (z)
		*/
		{
			y = j;
			break;
		}
	}
	if (x == -1 || y == -1)
	{
		return 0;
	}
	*px = x;
	*py = y;
	if (xd)
		*xd = dx;
	if (yd)
		*yd = dy;
	return 1;
}

char *make_cell_name(int x, int y, int xd, int yd)
{
	char *col_cap = make_col_cap(x);
	char *row_cap = make_row_cap(y);

	if (x <= 0 || x > col_count || y <= 0 || y > row_count)
		return NULL;

	char *res = (char*)allocmem(strlen(col_cap) + strlen(row_cap) + xd ? 1 : 0 + yd ? 1 : 0 + 1);
	int i = 0;
	if (xd)
	{
		res[i] = '$';
		i++;
	}
	strcpy(res + i, col_cap);
	i += strlen(col_cap);
	if (yd)
	{
		res[i] = '$';
		i++;
	}
	strcpy(res + i, row_cap);
	i += strlen(row_cap);
	res[i] = '\0';
	freemem(col_cap);
	freemem(row_cap);
	return res;
}

// замены ссылки на одну ячейку
char *change_cell_ref(char *name, int sx, int sy)
{
	int x0, y0, xd, yd;

	parse_cell_name(name, &x0, &y0, &xd, &yd);

	//sprintf(debuf, "parsed cell name %S to %U %U", name, x0, y0);
	//rtlDebugOutString(debuf);

	// у нас есть х0 и у0.
	//sprintf(debuf, "%U in %U %U, %U in %U %U",x0, cf_x0, cf_x1, y0, cf_y0, cf_y1);
	//rtlDebugOutString(debuf);
	if (x0 >= cf_x0 && x0 <= cf_x1 && y0 >= cf_y0 && y0 <= cf_y1)
	{
		if (!xd)
		{
			x0 += sx;
			if (x0 <= 0 || x0 > col_count)
				x0 -= sx;
		}
		if (!yd)
		{
			y0 += sy;
			if (y0 <= 0 || y0 > row_count)
				y0 -= sy;
		}
	}

	return make_cell_name(x0, y0, xd, yd);
}

// замена всех ссылок на ячейки
char *change_formula(char *name, int sx, int sy)
{
	int i = 0;
	int in_name = 0;	// 1 - читаем буквенную часть. 2 - читаем цифровую. 0 - читаем разделители и т.д.
	int alp_len = 0, dig_len = 0;
	int buf_i = 0;

	char buffer[256]; // очень плохо
	memset((Byte*)buffer, 0, 256);

	//sprintf(debuf, "change formula %S by %U %U", name, sx, sy);
	//rtlDebugOutString(debuf);

	while (i < strlen(name) + 1)
	{
		char c;
		if (i == strlen(name))
			c = ' ';
		else
			c = name[i];
		buffer[buf_i++] = c;

		switch (in_name)
		{
			case 0:
			{
				if (isalpha2(c) || c == '$')
				{
					in_name = 1;
					alp_len = 1;
					dig_len = 0;
				}
			}
			break;
			case 1:
			{
				if (isalpha2(c))
				{
					alp_len++;
				}
				else if (c == '$' || isdigit(c))
				{
					in_name = 2;
					dig_len++;
				}
				else
				{
					// незавершенное имя ячейки - не имя
					in_name = 0;
					alp_len = dig_len = 0;
				}
			}
			break;
			case 2:
			{
				if (isdigit(c))
				{
					dig_len++;
				}
				else
				{
					if (alp_len > 0 && dig_len > 0)
					{
						// вот нормальная ячейка
						int idx = i - alp_len - dig_len;
						int len = alp_len + dig_len;
						char *cell = (char*)allocmem(len + 1);
						//strncpy(cell, name + i, alp_len + dig_len);
						for (int l = 0; l < len; l++)
							cell[l] = name[idx + l];
						cell[len] = '\0';

						//sprintf(debuf, "found cell name '%S' alp %U dig %U", cell, alp_len, dig_len);
						//rtlDebugOutString(debuf);
						char *cell_new = change_cell_ref(cell, sx, sy);
						//sprintf(debuf, "rename to '%S'", cell_new);
						//rtlDebugOutString(debuf);
						if (cell_new)
						{
							char cc = buffer[buf_i - 1];
							strcpy(buffer + buf_i - len - 1, cell_new);
							buf_i += strlen(cell_new) - len;
							buffer[buf_i - 1] = cc;
						}
						//freemem(cell);
						//freemem(cell_new);
						alp_len = dig_len = 0;
						in_name = 0;
					}
				}
			}
		}
		i++;
	}
	//sprintf(debuf, "change formula done");
	//rtlDebugOutString(debuf);
	char *res = (char*)allocmem(strlen(buffer) + 1);
	strcpy(res, buffer);
	return res;
}




