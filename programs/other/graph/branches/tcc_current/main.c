#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>
#include <clayer/boxlib.h>
#include <conio.h>
#include <math.h>

#include "func.h"
#include "parser.h"

#define nullptr 0

enum BUTTONS {
	BTN_QUIT = 1,
	BTN_EDIT = 5
};

const char STR_PROGRAM_TITLENAME[] = "Graph";
const char empty_text[] = "No function loaded. Type file name and press Enter. ";
const char er_file_not_found[] = "Couldn't open file.";
const char er_wrong_syntax[] = "Syntax error in file: at line %d, at byte %d.";
const char er_invalid_graph_size[] = "Invalid graph size (look at passed x1,x2,y1,y2 graph's border coords).";
const char str_filename[] = "Filename:";
const char str_editfile[] = "Edit";
const char f_str[] = ". Function y=";
const char STR_ERR_CONSOLEINIT[] = "Unable to initialize console.";
const char STR_MSG_HELP_USAGE[] = "Usage: %s [OPTION] [FILENAME]...\n";
const char* STR_MSG_HELP[] = {
	"Draws a graph by calculating mathematical function or using list of points.\n\n",

	"\t--help\t\tdisplay this help and exit\n\n",

	"You can provide path to file if you want it to be opened immediately.\n",
};

// начальные размеры
#define WND_W 600
#define WND_H 470

#define LIGHTGREEN 0xff0000
#define WHITE 0xffffff
#define BLACK 0x0
#define LIGHTBLUE 0x0000ff
#define LIGHTRED 0xff0000

// font colors
#define BIGFONTCOLOR BLACK
#define SMALLFONTCOLOR BLACK

#define THREE 3.0
// minimum space: 3 pixels

#define BIG_HEIGHT 4.0
#define SMALL_HEIGHT 2.0
#define TEXT_X 15.0
// numeric format for output
#define FORMAT "%.2f%c"
// format for two coords
#define FORMAT_COORD "(%.2f, %.2f)%c"

int SysColor = 0;
double grx1, gry1, grx2, gry2;

#define FUNCTS_MAXLEN_INITIAL 1
void** functs; // Array of addresses, first byte of struct always should describe its type
unsigned int functs_maxlen = 0;
unsigned int functs_count = 0;

#define FUNCTS_PTYPE 0
#define PFUNCT_POINTS_MAXLEN_INITIAL 16
typedef struct {
	char type;
	double* points;
	unsigned int points_maxlen;
	unsigned int points_count;
} PFunct;

#define FUNCTS_ETYPE 1
typedef struct {
	char type;
	char* expr;
} EFunct;

char edit_path[256];
edit_box mybox = { 200, 92, WND_H-16-32, 0xffffff, 0x94AECE, 0, 0x808080, 0x10000000,
					sizeof(edit_path)-1, (void*)&edit_path, 0, 0, 0 };

int full_head_size;
char* full_head;

char* lasterror_text = nullptr;
int lasterror_color;

// constructor of TCoord
TCoord coord(double x, double y) {
	TCoord r;
	r.x = x;
	r.y = y;
	return r;
}

// move and scale mathematical coords to fit screen coords
TCoord mat2Graf(TCoord c, TCoord scrMin, TCoord scrMax, TCoord mMin, TCoord mMax) {
	TCoord r;
  if (c.x > mMax.x)
    c.x = mMax.x;
  if (c.x < mMin.x)
    c.x = mMin.x;
  if (c.y > mMax.y)
    c.y = mMax.y;
  if (c.y < mMin.y)
    c.y = mMin.y;
  r.x = (scrMax.x - scrMin.x) / (mMax.x - mMin.x) * (c.x - mMin.x) + scrMin.x;
  r.y = (scrMax.y - scrMin.y) / (mMax.y - mMin.y) * (mMax.y - c.y) + scrMin.y;

  return r;
}

//just for rounding...
void DrawLine(double xs, double ys, double xe, double ye, ksys_color_t color) {
	_ksys_draw_line(roundi(xs), roundi(ys), roundi(xe), roundi(ye), color);
}
void DrawText(const char* text, double x, double y, uint32_t len, ksys_color_t color) {
	_ksys_draw_text(text, roundi(x), roundi(y), len, color);
}

double delta_getoptsize(int n) {
	return pow(2, n % 3) * pow(10, n / 3);
}
//Returns res: delta_getoptsize(res) <= size < delta_getoptsize(res+1)
int delta_getn(double size) {
	int res = floor(log10(size) * 3);
	double t = delta_getoptsize(res + 1);
	if (t < size || isequal(t, size)) ++res;
	return res;
}

// huge function to draw all the stuff except the function itself
void drawAxis(TCoord scrMin, TCoord scrMax, TCoord mMin, TCoord mMax) {
  TCoord cZero = {0.0,0.0}, gMin, gMax, gZero, step;
  TCoord from, to;
  double i = 0.0;
  char buf[30]="";
  int strlentemp;

  double deltaBigX = delta_getoptsize(delta_getn(mMax.x - mMin.x)) / 20;
  double deltaBigY = delta_getoptsize(delta_getn(mMax.y - mMin.y)) / 20;
  double deltaSmallX = deltaBigX / 5;
  double deltaSmallY = deltaBigY / 5;

// scr means Screen(bounding rect)
// m   means Mathematical
// g   means Graphic(real screen position)

  gMin = mat2Graf(mMin, scrMin, scrMax, mMin, mMax);
  gMax = mat2Graf(mMax, scrMin, scrMax, mMin, mMax);
  gZero = mat2Graf(cZero, scrMin, scrMax, mMin, mMax);

  SysColor = BLACK;

  DrawLine(gMin.x, gZero.y ,gMax.x, gZero.y, SysColor); // osy X
  DrawLine(gZero.x, gMin.y, gZero.x, gMax.y, SysColor); // osy Y

  // bounding rect
  DrawLine(gMin.x, gMin.y, gMax.x, gMin.y, SysColor);
  DrawLine(gMin.x, gMax.y, gMax.x, gMax.y, SysColor);
  DrawLine(gMin.x, gMin.y, gMin.x, gMax.y, SysColor);
  DrawLine(gMax.x, gMin.y, gMax.x, gMax.y, SysColor);

  // coords of the rect : lower left
  sprintf(buf, FORMAT_COORD, grx1, gry1, '\0');
  //_ksys_debug_puts(buf);
  strlentemp = strlen(buf);
  DrawText(buf, gMin.x, gMin.y + textheight(buf, strlentemp), strlentemp, SysColor);
  // upper left
  sprintf(buf, FORMAT_COORD, grx1, gry2, '\0');
  strlentemp = strlen(buf);
  DrawText(buf, gMin.x, gMax.y - textheight(buf, strlentemp), strlentemp, SysColor);
  // lower right
  sprintf(buf, FORMAT_COORD, grx2, gry1, '\0');
  strlentemp = strlen(buf);
  DrawText(buf, gMax.x - textwidth(buf, strlentemp), gMin.y + textheight(buf, strlentemp), strlentemp, SysColor);
  // upper right
  sprintf(buf, FORMAT_COORD, grx2, gry2, '\0');
  strlentemp = strlen(buf);
  DrawText(buf, gMax.x - textwidth(buf, strlentemp), gMax.y - textheight(buf, strlentemp), strlentemp, SysColor);

  //_ksys_debug_puts("some lines painted\n");


  step.x = (mMax.x - mMin.x) / (scrMax.x - scrMin.x);
  step.y = (mMax.y - mMin.y) / (scrMax.y - scrMin.y);

	//roundi values
	double xmin = round(mMin.x / deltaBigX) * deltaBigX;
	double ymin = round(mMin.y / deltaBigY) * deltaBigY;

  // (0,0)

  if ((grx1 * grx2 <= 0.0) && (gry1 * gry2 <= 0.0))
  {
	  from.x=0.0;
	  from.y=0.0;
	  from = mat2Graf(from, scrMin, scrMax, mMin, mMax);
	  SysColor = BLACK;
	  sprintf(buf, FORMAT, 0.0, '\0');
	  strlentemp = strlen(buf);
	  DrawText(buf, from.x - textwidth(buf, strlentemp), from.y + textheight(buf, strlentemp), strlentemp, SysColor);
  }


  // big marks on X
  //settextstyle(0, 0, 1);
  if (deltaBigX / step.x > THREE) {
    for (i = xmin; i <= mMax.x; i += deltaBigX) {
	  if (i != 0.0) {
		  from.x = i;
		  to.x = from.x;
		  from.y = -BIG_HEIGHT * step.y;
		  to.y = BIG_HEIGHT * step.y;
		  from = mat2Graf(from, scrMin, scrMax, mMin, mMax);
		  to = mat2Graf(to, scrMin, scrMax, mMin, mMax);
		  SysColor = BLACK;
		  DrawLine(from.x, from.y, to.x, to.y, SysColor);
		  // write number
		  sprintf(buf, FORMAT, i, '\0');
		  strlentemp = strlen(buf);
		  // if it fits in the GAP, then write it
		  if (from.y > scrMin.y && (deltaBigX > (textwidth(buf, strlentemp) + 1.0) * step.x)) {
			   SysColor = BIGFONTCOLOR;
			   DrawText(buf, from.x - textwidth(buf, strlentemp) / 2.0, to.y - textheight(buf, strlentemp), strlentemp, SysColor);
		  }
	  }
    }
  }

  // big marks on Y
  if (deltaBigY / step.y > THREE) {
    for (i = ymin; i <= mMax.y; i += deltaBigY) {
	  if (i != 0.0) {
		  from.y = i;
		  to.y = from.y;
		  from.x = -BIG_HEIGHT * step.x;
		  to.x = BIG_HEIGHT * step.x;
		  from = mat2Graf(from, scrMin, scrMax, mMin, mMax);
		  to = mat2Graf(to, scrMin, scrMax, mMin, mMax);
		  SysColor = BLACK;
		  DrawLine(from.x, from.y, to.x, to.y, SysColor);
		  sprintf(buf, FORMAT, i, '\0');
		  strlentemp = strlen(buf);
 		  if (from.x > scrMin.x && (deltaBigY > textheight(buf, strlentemp) * step.y)) {
			   SysColor = BIGFONTCOLOR;
			 DrawText(buf, from.x + TEXT_X, to.y - textheight(buf, strlentemp) / 2.0, strlentemp, SysColor);
		  }
	  }
    }
  }

	double xmin2 = round(mMin.x / deltaSmallX) * deltaSmallX;
	double ymin2 = round(mMin.y / deltaSmallY) * deltaSmallY;

  if (deltaSmallX / step.x > THREE) {
    int j = roundi((-xmin + xmin2) / deltaSmallX);
    for (i = xmin2; i <= mMax.x; i += deltaSmallX, j++) {
      if (j % 10 == 0) {
      // we need to skip every tenth mark, to avoid overwriting big marks
		j = 0;
		continue;
      }
      from.x = i;
      to.x = from.x;
      from.y = -SMALL_HEIGHT * step.y;
      to.y = SMALL_HEIGHT * step.y;
      from = mat2Graf(from, scrMin, scrMax, mMin, mMax);
	  to = mat2Graf(to, scrMin, scrMax, mMin, mMax);
      SysColor = BLACK;
      DrawLine(from.x, from.y, to.x, to.y, SysColor);
      sprintf(buf, FORMAT, i, '\0');
  	  strlentemp = strlen(buf);
      if (from.y > scrMin.y && (deltaSmallX > textwidth(buf, strlentemp) * step.x)) {
		SysColor = SMALLFONTCOLOR;
		DrawText(buf, from.x - textwidth(buf, strlentemp) / 2.0, to.y - textheight(buf, strlentemp), strlentemp, SysColor);
      }


    }

  }

  // finally small marks on Y
  if (deltaSmallY / step.y > THREE) {
    //_ksys_debug_puts("really small marks y painted\n");
    int j = roundi((-ymin + ymin2) / deltaSmallY);
    for (i = ymin2; i <= mMax.y; i += deltaSmallY, j++) {
      if (j % 10 == 0) {
      // we need to skip every tenth, to avoid overwriting
		j = 0;
		continue;
      }
      from.y = i;
      to.y = from.y;
      from.x = -SMALL_HEIGHT * step.x;
      to.x = SMALL_HEIGHT * step.x;
      from = mat2Graf(from, scrMin, scrMax, mMin, mMax);
      to = mat2Graf(to, scrMin, scrMax, mMin, mMax);
      SysColor = BLACK;
      DrawLine(from.x, from.y, to.x, to.y, SysColor);
      sprintf(buf, FORMAT, i, '\0');
  	  strlentemp = strlen(buf);
      if (from.x > scrMin.x && (deltaSmallY > textheight(buf, strlentemp) * step.y)) {
       	SysColor = SMALLFONTCOLOR;
       	DrawText(buf, from.x + TEXT_X, from.y - textheight(buf, strlentemp) / 2.0, strlentemp, SysColor);
      }
    }
  }

}

// returns idx: a[idx] <= x element; -1 can be returned
int binsearchRB_xInPoints(double* a, int a_size, double x) {
	int L = -1;
	int R = a_size;
	while (R - L > 1) {
		int M = (R + L) / 2;
		if (a[2*M] <= x) // 2*M because in points array there are 2 coords: x and y
			L = M;
		else
			R = M;
	}
	return L;
}

int calcFunct_idx;

int calcFunct(double x, double* res) {
	if (*(char*)functs[calcFunct_idx] == FUNCTS_ETYPE) {
		set_exp(((EFunct*)functs[calcFunct_idx])->expr, x);
		get_exp(res); // парсить для каждого значения х? да я с ума сошел.
		return 0;
	}

	PFunct* pf = functs[calcFunct_idx];

	if (x <= pf->points[0]) {
		return 1; // Or it better return something??
 		//*res = pf->points[1];
	}
	if (x >= pf->points[(pf->points_count - 1) * 2]) {
		return 1; // Or it better return something??
 		//*res = pf->points[(pf->points_count - 1) * 2 + 1];
	}

	int i = binsearchRB_xInPoints(pf->points, pf->points_count, x);
	*res = (x - pf->points[2 * i]) / (pf->points[2 * (i + 1)] - pf->points[2 * i])
		* (pf->points[2 * (i + 1) + 1] - pf->points[2 * i + 1]) + pf->points[2 * i + 1];
	return 0;
}

void drawFunction(TCoord scrMin, TCoord scrMax, TCoord mMin, TCoord mMax, int color) {
  SysColor = color;
  TCoord p, p0 = {0.0, 0.0}, step;
  step.x = (mMax.x - mMin.x) / (scrMax.x - scrMin.x);
  int firstPoint = 1;
  double y;
  for (double x = mMin.x; x < mMax.x; x += step.x) { // function is defined here and gets in the range
    if (!calcFunct(x, &y)) { // условие, что функция правильно вычислена
      if (y > mMin.y && y < mMax.y) {
      	p = mat2Graf(coord(x, y), scrMin, scrMax, mMin, mMax);
		// if it's our first point, only remember its coords
		// otherwise, draw a line from prev to current
      	if (firstPoint == 0) {
      	  DrawLine(p0.x, p0.y, p.x, p.y, SysColor);
        } else firstPoint = 0;
     	p0 = p;
      } else {// too big/small
 	      firstPoint = 1;
      }
    } else { // no value
       firstPoint = 1;
    }
  }
}

int pointsCompare(const void* a, const void* b) {
	const double aval = *(const double*)a; // first element of a
	const double bval = *(const double*)b; // first element of b
	if (isequal(aval, bval)) return 0;
	if (aval < bval) return -1;
	return 1;
}

void freeMemoryForGraphs() {
	if (functs_count) {
		for (int i = 0; i < functs_count; ++i) {
			switch (*(char*)functs[i]) {
				case FUNCTS_PTYPE:
					free(((PFunct*)functs[i])->points);
  					break;
				case FUNCTS_ETYPE:
					free(((EFunct*)functs[i])->expr);
					break;
 			}
			free(functs[i]);
		}
		free(functs);
		functs = nullptr;
		functs_maxlen = 0;
		functs_count = 0;
	}
}

void functsSizeChecker() {
	if (functs_maxlen <= functs_count + 1) {
		if (!functs_maxlen) {
			functs_maxlen = FUNCTS_MAXLEN_INITIAL;
			functs = (void**)malloc(sizeof(void*) * functs_maxlen);
		} else {
			functs_maxlen *= 2;
			functs = (void**)realloc(functs, sizeof(void*) * functs_maxlen);
		}
		if (!functs) exit(3);
	}
}

// итоговая версия читалки текстовых файлов
// NOTICE for devs: full_head must be resetted after fail of this function (on 0 ret. code)
int load_points3() {
	freeMemoryForGraphs();

	// get file size
	ksys_bdfe_t bdfe;
	int rr = _ksys_file_get_info(edit_path, &bdfe);
	if (rr) {
		if (lasterror_text) free(lasterror_text);
		lasterror_text = (char*)malloc(sizeof(er_file_not_found));
		if (!lasterror_text) exit(3);
		strcpy(lasterror_text, er_file_not_found);
		lasterror_color = 0x90000000;
		return 0;
	}
	unsigned int filesize = bdfe.size;

	char* HugeBuf = (char*)malloc(filesize + 1); // разбираем как строку, отсюда терминатор '\0'
	if (!HugeBuf) exit(3);
	unsigned int countRead;
	rr = _ksys_file_read_file(edit_path, 0, filesize, HugeBuf, &countRead);
	if (countRead != filesize) exit(4); // in case if something corrupted
	HugeBuf[filesize] = '\0';

	int lineNum = 1;
	int lineStartIdx = 0;
	int borderNextCoord = 0;
 	int funct_status = 0; // 0 - empty, 1 - points (can be filled), 2 - expr. (definitely finished)
 	int i = 0;
	while (i < filesize) {
		if (isalpha(HugeBuf[i])) {
			if (HugeBuf[i] == '\n') {
				++lineNum;
				lineStartIdx = i + 1;
			}
			++i;
			continue;
		}

		if (HugeBuf[i] == ';') {
			if (funct_status) {
				++functs_count;
				funct_status = 0;
			}
			++i;
			continue;
		}

		if (borderNextCoord == 4 && HugeBuf[i] == '=' && !funct_status) {
			functsSizeChecker();
			funct_status = 2;
			functs[functs_count] = malloc(sizeof(EFunct));
			if (!functs[functs_count]) exit(3);
			*(char*)functs[functs_count] = FUNCTS_ETYPE;

			EFunct* ef = functs[functs_count];
			int efunct_len;
			for (efunct_len = 0; HugeBuf[i + 1 + efunct_len] != ';' && HugeBuf[i + 1 + efunct_len] != '\0'; ++efunct_len) {}
			ef->expr = (char*)malloc(efunct_len+1);
			if (!ef->expr) exit(3);
			strncpy(ef->expr, HugeBuf + i + 1, efunct_len);
			ef->expr[efunct_len] = '\0';
			i += efunct_len + 1;
			continue;
		}

		int j;
		double d = convert(HugeBuf + i, &j);
		if (d == ERROR) {
			sprintf(debuf, er_wrong_syntax, lineNum, i-lineStartIdx+1);
			if (lasterror_text) free(lasterror_text);
			lasterror_text = (char*)malloc(strlen(debuf)+1);
			if (!lasterror_text) exit(3);
			strcpy(lasterror_text, debuf);
			lasterror_color = 0x90FF0000;
			free(HugeBuf);
			freeMemoryForGraphs();
			return 0;
		}
		if (d == ERROR_END) {
			_ksys_debug_puts("[");
			_ksys_debug_puts(STR_PROGRAM_TITLENAME);
			_ksys_debug_puts("] EOF reached.\n");
			break;
		}
		i += j;

		if (borderNextCoord < 4) {
			switch (borderNextCoord) {
				case 0:
					grx1 = d;
					break;
				case 1:
					grx2 = d;
					break;
				case 2:
					gry1 = d;
					break;
				case 3:
					gry2 = d;
					if (grx1 > grx2 || isequal(grx1, grx2) || gry1 > gry2 || isequal(gry1, gry2)) {
						if (lasterror_text) free(lasterror_text);
						lasterror_text = (char*)malloc(sizeof(er_invalid_graph_size));
						if (!lasterror_text) exit(3);
						strcpy(lasterror_text, er_invalid_graph_size);
						lasterror_color = 0x90FF0000;
						free(HugeBuf);
						return 0;
					}
					break;
			}
			++borderNextCoord;
		} else {
			if (!funct_status) {
				functsSizeChecker();
				funct_status = 1;
				functs[functs_count] = malloc(sizeof(PFunct));
				if (!functs[functs_count]) exit(3);
				*(char*)functs[functs_count] = FUNCTS_PTYPE;
 				((PFunct*)functs[functs_count])->points = nullptr;
  				((PFunct*)functs[functs_count])->points_maxlen = 0;
  				((PFunct*)functs[functs_count])->points_count = 0;
  			}

			PFunct* pf = functs[functs_count];
			if (pf->points_maxlen <= pf->points_count + 1) {
				if (!pf->points_maxlen) {
					pf->points_maxlen = PFUNCT_POINTS_MAXLEN_INITIAL;
					pf->points = (double*)malloc(sizeof(double) * pf->points_maxlen);
				} else {
					pf->points_maxlen *= 2;
					pf->points = (double*)realloc(pf->points, sizeof(double) * pf->points_maxlen);
				}
				if (!pf->points) exit(3);
			}
			pf->points[pf->points_count++] = d;
		}
	}
	free(HugeBuf);
	if (funct_status) ++functs_count;

	for (int f_idx = 0; f_idx < functs_count; ++f_idx) {
		if (*(char*)functs[f_idx] != FUNCTS_PTYPE) continue;
		PFunct* pf = functs[f_idx];
		pf->points_count /= 2;
		qsort(pf->points, pf->points_count, sizeof(double)*2, pointsCompare);
	}

	int full_head_new_size = sizeof(STR_PROGRAM_TITLENAME)-1 + 3 + strlen(edit_path) + 1; // 3 = strlen(" - ")
	if (functs_count == 1) {
		if (*(char*)functs[0] == FUNCTS_PTYPE) {
			sprintf(debuf, ". Number of points: %u.", ((PFunct*)functs[0])->points_count);
			full_head_new_size += strlen(debuf);
		} else
			full_head_new_size += sizeof(f_str)-1 + strlen(((EFunct*)functs[0])->expr);
	} else {
		sprintf(debuf, ". Functions loaded: %u.", functs_count);
		full_head_new_size += strlen(debuf);
	}

	if (full_head_new_size > full_head_size) {
		free(full_head);
		full_head_size = full_head_new_size;
		full_head = (char*)malloc(full_head_size);
		if (!full_head) exit(3);
	}
	char* full_head_addr = full_head;
	strcpy(full_head_addr, STR_PROGRAM_TITLENAME);
	full_head_addr += sizeof(STR_PROGRAM_TITLENAME)-1;
	strcpy(full_head_addr, " - ");
	full_head_addr += 3; // 3 = strlen(" - ")
	strcpy(full_head_addr, edit_path);
	full_head_addr += strlen(edit_path);
	if (functs_count == 1) {
		if (*(char*)functs[0] == FUNCTS_PTYPE)
			strcpy(full_head_addr, debuf);
		else {
			strcpy(full_head_addr, f_str);
			full_head_addr += sizeof(f_str)-1;
	 		strcpy(full_head_addr, ((EFunct*)functs[0])->expr);
	 	}
 	} else {
		strcpy(full_head_addr, debuf);
 	}

	if (lasterror_text) {
		free(lasterror_text);
		lasterror_text = nullptr;
	}
	return 1;
}

void draw_window() {
	_ksys_start_draw();
	_ksys_create_window(100, 80, WND_W, WND_H, full_head, 0xFFFFFF, 0x33);
	_ksys_end_draw();

	ksys_thread_t info;
	_ksys_thread_info(&info, 0xFFFFFFFF);
	int cWidth = info.winx_size - 9;
	int cHeight = info.winy_size - _ksys_get_skin_height() - 4;

	mybox.top = cHeight - 50;
	mybox.width = cWidth - mybox.left - 80;

	if (info.window_state & 0x04) return; //draw nothing if window is rolled-up

	if (!functs_count) {
		_ksys_draw_text(empty_text, (cWidth - 8 * strlen(empty_text)) / 2, cHeight / 2 - 25, sizeof(empty_text)-1, 0x90000000);
	} else {
		TCoord scrMin = coord(10, 20),
		       scrMax = coord(cWidth - 20, cHeight - 70),
		       mMin = coord(grx1, gry1),
		       mMax = coord(grx2, gry2);
		drawAxis(scrMin, scrMax, mMin, mMax);
		for (calcFunct_idx = 0; calcFunct_idx < functs_count; ++calcFunct_idx)
			drawFunction(scrMin, scrMax, mMin, mMax, 0x00ff0000);
	}

	_ksys_draw_text((char*)str_filename, 15, mybox.top + 4, strlen(str_filename), 0x90000000);

	edit_box_draw(&mybox);

	_ksys_define_button(cWidth - 70, mybox.top, 50, 21, BTN_EDIT, 0xc0c0c0);
	_ksys_draw_text((char*)str_editfile, cWidth - 60, mybox.top + 4, 0, 0x90000000);

	if (lasterror_text) {
		_ksys_draw_bar(10, 10, 200, 20, 0xFFFFFF); // фон для сообщений об ошибках
		_ksys_draw_text(lasterror_text, 10, 10, strlen(lasterror_text), lasterror_color);
	}
}

void consoleInit() {
	if (con_init()) { // Init fail
		_ksys_debug_puts("[");
		_ksys_debug_puts(STR_PROGRAM_TITLENAME);
		_ksys_debug_puts("] ");
  		_ksys_debug_puts(STR_ERR_CONSOLEINIT);
		_ksys_debug_puts("\n");
  		exit(2);
	}
	(*con_set_title)(STR_PROGRAM_TITLENAME);
}

void consoleExit() {
	(*con_exit)(0);
}

int main(int argc, char** argv) {
	full_head_size = sizeof(STR_PROGRAM_TITLENAME); // also with '\0'
	full_head = (char*)malloc(full_head_size);
	if (!full_head) return 3;
	strcpy(full_head, STR_PROGRAM_TITLENAME);

	if (argc == 2) {
		if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) {
			consoleInit();
			printf(STR_MSG_HELP_USAGE, argv[0]);
			for (int j = 0; j < sizeof(STR_MSG_HELP)/sizeof(STR_MSG_HELP[0]); ++j) puts(STR_MSG_HELP[j]);
			consoleExit();
			return 0;
		} else {
			strcpy(edit_path, argv[1]);
			mybox.size = mybox.pos = strlen(edit_path);
			load_points3();
		}
	} else if (argc > 2) {
		consoleInit();
		printf(STR_MSG_HELP_USAGE, argv[0]);
		consoleExit();
		return 1;
	}

	_ksys_set_event_mask(0xC0000027);
	int event;
	while (1) {
		edit_box_mouse(&mybox);
		event = _ksys_get_event();
		switch (event) {
			case KSYS_EVENT_REDRAW:
				draw_window();
				break;
			case KSYS_EVENT_KEY: {
				ksys_oskey_t kc = _ksys_get_key();
				switch (kc.code) {
					case 0x0D: {
						int res = load_points3();
						if (!res) strcpy(full_head, STR_PROGRAM_TITLENAME);
						draw_window();
						break;
					}
					default:
						edit_box_key_safe(&mybox, kc);
				}
				break;
			}
			case KSYS_EVENT_BUTTON: {
				// button pressed; we have only one button, close
				uint32_t button = _ksys_get_button();
				if (button == BTN_QUIT)
					_ksys_exit();
				else if (button == BTN_EDIT)
					_ksys_exec("/sys/develop/cedit", edit_path);
			}
		}
	}
	return 0;
}
