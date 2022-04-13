#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>
#include <clayer/boxlib.h>
#include <conio.h>

#include "func.h"
#include "parser.h"

#define nullptr 0
#define dword unsigned int

enum BUTTONS {
	BTN_QUIT = 1,
	BTN_EDIT = 5
};

const char STR_PROGRAM_TITLENAME[] = "Graph";
const char empty_text[] = "No function loaded. Type file name and press Enter. ";
const char er_file_not_found[] = "Cannot open file. ";
const char str_filename[] = "Filename:";
const char str_editfile[] = "Edit";
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
// special value to text if enough space
#define FORMAT_TEST "0.00"

#define DELTA_BIG 1.0
#define DELTA_SMALL 0.1

int SysColor = 0;

double* points;
dword point_count = 0;
double x1, y1, x2, y2;
char* funct = nullptr;

char edit_path[256];
edit_box mybox = { 200, 92, WND_H-16-32, 0xffffff, 0x94AECE, 0, 0x808080, 0x10000000,
					sizeof(edit_path)-1, (void*)&edit_path, 0, 0, 0 };

char* full_head;

char* HugeBuf = nullptr;

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

// huge function to draw all the stuff except the function itself
void drawAxis(TCoord scrMin, TCoord scrMax, TCoord mMin, TCoord mMax) {
  TCoord cZero={0.0,0.0},
	   gMin, gMax, gZero, step;
  TCoord from, to;
  double i=0.0;
  int j;
  double xmin, xmin2, ymin, ymin2;
  char buf[30]="";
  int strlentemp;


// scr means Screen(bounding rect)
// m   means Mathematical
// g   means Graphic(real screen position)

  //_ksys_debug_puts("draw axis called\n");

  //sprintf(debuf, "test: %f,%f,%f,%f\n", 123.45, 1.0, -0.9, 12.57);
  //_ksys_debug_puts(debuf);

  gMin = mat2Graf(mMin, scrMin, scrMax, mMin, mMax);
  gMax = mat2Graf(mMax, scrMin, scrMax, mMin, mMax);
  gZero = mat2Graf(cZero, scrMin, scrMax, mMin, mMax);

  // clear
 // SysColor = WHITE;
 //rectangle(di(gMin.x), di(gMin.y), di(gMax.x), di(gMax.y));
  // ftopku

  SysColor = BLACK;
  // osy X
  _ksys_draw_line(gMin.x, gZero.y ,gMax.x, gZero.y, SysColor);
  // osy Y
  _ksys_draw_line(gZero.x, gMin.y, gZero.x, gMax.y, SysColor);
  // bounding rect
  _ksys_draw_line(gMin.x, gMin.y, gMax.x, gMin.y, SysColor);
  _ksys_draw_line(gMin.x, gMax.y, gMax.x, gMax.y, SysColor);

  _ksys_draw_line(gMin.x, gMin.y, gMin.x, gMax.y, SysColor);
  _ksys_draw_line(gMax.x, gMin.y, gMax.x, gMax.y, SysColor);

  // coords of the rect : lower left
  sprintf(buf, FORMAT_COORD, x1, y1, '\0');
  //_ksys_debug_puts(buf);
  strlentemp = strlen(buf);
  _ksys_draw_text(buf, gMin.x, gMin.y + textheight(buf, strlentemp), strlentemp, SysColor);
  // upper left
  sprintf(buf, FORMAT_COORD, x1, y2, '\0');
  strlentemp = strlen(buf);
  _ksys_draw_text(buf, gMin.x, gMax.y - textheight(buf, strlentemp), strlentemp, SysColor);
  // lower right
  sprintf(buf, FORMAT_COORD, x2, y1, '\0');
  strlentemp = strlen(buf);
  _ksys_draw_text(buf, gMax.x - textwidth(buf, strlentemp), gMin.y + textheight(buf, strlentemp), strlentemp, SysColor);
  // upper right
  sprintf(buf, FORMAT_COORD, x2, y2, '\0');
  strlentemp = strlen(buf);
  _ksys_draw_text(buf, gMax.x - textwidth(buf, strlentemp), gMax.y - textheight(buf, strlentemp), strlentemp, SysColor);

  //_ksys_debug_puts("some lines painted\n");


  step.x = (mMax.x - mMin.x) / (scrMax.x - scrMin.x);
  step.y = (mMax.y - mMin.y) / (scrMax.y - scrMin.y);

// round values
  xmin = (int)((mMin.x / DELTA_BIG) * DELTA_BIG);
  ymin = (int)((mMin.y / DELTA_BIG) * DELTA_BIG);

  // (0,0)

  if ((x1 * x2 <= 0.0) && (y1 * y2 <= 0.0))
  {
	  from.x=0.0;
	  from.y=0.0;
	  from = mat2Graf(from, scrMin, scrMax, mMin, mMax);
	  SysColor = BLACK;
	  sprintf(buf, FORMAT, 0.0, '\0');
	  strlentemp = strlen(buf);
	  _ksys_draw_text(buf, from.x - textwidth(buf, strlentemp), from.y + textheight(buf, strlentemp), strlentemp, SysColor);
  }


  // big marks on X
  //settextstyle(0, 0, 1);
  if (DELTA_BIG / step.x > THREE) {
    for (i = xmin; i <= mMax.x; i += DELTA_BIG) {
	  if (i != 0.0) {
		  from.x = i;
		  to.x = from.x;
		  from.y = -BIG_HEIGHT * step.y;
		  to.y = BIG_HEIGHT * step.y;
		  from = mat2Graf(from, scrMin, scrMax, mMin, mMax);
		  to = mat2Graf(to, scrMin, scrMax, mMin, mMax);
		  SysColor = BLACK;
		  _ksys_draw_line(from.x, from.y, to.x, to.y, SysColor);
		  // write number
		  sprintf(buf, FORMAT, i, '\0');
		  strlentemp = strlen(buf);
		  // if it fits in the GAP, then write it
		  if (from.y > scrMin.y && (DELTA_BIG > (textwidth(buf, strlentemp) + 1.0) * step.x)) {
			   SysColor = BIGFONTCOLOR;
			   _ksys_draw_text(buf, from.x - textwidth(buf, strlentemp) / 2.0, to.y - textheight(buf, strlentemp), strlentemp, SysColor);
		  }
	  }
    }
  }
  //_ksys_debug_puts("big marks x painted\n");

  // big marks on Y
  if (DELTA_BIG / step.y > THREE) {
    for (i = ymin; i <= mMax.y; i += DELTA_BIG) {
	  if (i != 0.0) {
		  from.y = i;
		  to.y = from.y;
		  from.x = -BIG_HEIGHT * step.x;
		  to.x = BIG_HEIGHT * step.x;
		  from = mat2Graf(from, scrMin, scrMax, mMin, mMax);
		  to = mat2Graf(to, scrMin, scrMax, mMin, mMax);
		  SysColor = BLACK;
		  _ksys_draw_line(from.x, from.y, to.x, to.y, SysColor);
		  sprintf(buf, FORMAT, i, '\0');
		  strlentemp = strlen(buf);
 		  if (from.x > scrMin.x && (DELTA_BIG > textheight(buf, strlentemp) * step.y)) {
			   SysColor = BIGFONTCOLOR;
			 _ksys_draw_text(buf, from.x + TEXT_X, to.y - textheight(buf, strlentemp) / 2.0, strlentemp, SysColor);
		  }
	  }
    }
  }

  xmin2 = (int)(mMin.x / DELTA_SMALL) * DELTA_SMALL;
  ymin2 = (int)(mMin.y / DELTA_SMALL) * DELTA_SMALL;

  if (DELTA_SMALL / step.x  > THREE) {
    j = (int)(( - xmin + xmin2 ) / DELTA_SMALL);
    for (i = xmin2; i <= mMax.x; i += DELTA_SMALL, j++) {
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
      _ksys_draw_line(from.x, from.y, to.x, to.y, SysColor);
      sprintf(buf, FORMAT, i, '\0');
  	  strlentemp = strlen(buf);
      if (from.y > scrMin.y && (DELTA_SMALL > textwidth(buf, strlentemp) * step.x)) {
		SysColor = SMALLFONTCOLOR;
		_ksys_draw_text(buf, from.x - textwidth(buf, strlentemp) / 2.0, to.y - textheight(buf, strlentemp), strlentemp, SysColor);
      }


    }

  }

  // finally small marks on Y
  if (DELTA_SMALL / step.y > THREE) {
    //_ksys_debug_puts("really small marks y painted\n");
    j = (int)(( - ymin + ymin2) / DELTA_SMALL);
    for (i = ymin2; i <= mMax.y; i += DELTA_SMALL, j++) {
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
      _ksys_draw_line(from.x, from.y, to.x, to.y, SysColor);
      sprintf(buf, FORMAT, i, '\0');
  	  strlentemp = strlen(buf);
      if (from.x > scrMin.x && (DELTA_SMALL > textheight(buf, strlentemp) * step.y)) {
       	SysColor = SMALLFONTCOLOR;
       	_ksys_draw_text(buf, from.x + TEXT_X, from.y - textheight(buf, strlentemp) / 2.0, strlentemp, SysColor);
      }
    }
  }

}

/*
  ends fucking piece of shit
*/

void drawFunction( function_t fi, TCoord scrMin, TCoord scrMax,
		   TCoord mMin, TCoord mMax, int color) {
  double x;
  double y;
  int firstPoint = 1;
  TCoord p, p0 = {0.0, 0.0}, step;

  drawAxis(scrMin, scrMax, mMin, mMax);

  SysColor = color;
  step.x = (mMax.x - mMin.x) / (scrMax.x - scrMin.x);

  for (x = mMin.x; x < mMax.x; x += step.x) {
    y = fi(x);
// function is defined here and gets in the range
    if (1) { // тут было условие, что функция правильно вычислена
      if ((y > mMin.y) && (y < mMax.y)) {
	      p = mat2Graf(coord(x, y), scrMin, scrMax, mMin, mMax);
	// if it's our first point, only remember its coords
	// otherwise, draw a line from prev to current
      	if (firstPoint == 0) {
      	  _ksys_draw_line(p0.x, p0.y, p.x, p.y, SysColor);
        } else firstPoint = 0;
     	p0 = p;
      }
      else {// too big/small
	      firstPoint = 1;
      }
    }
    else { // no value
      firstPoint = 1;
    }
  }

}

// итоговая версия читалки текстовых файлов
int load_points3() {
	ksys_bdfe_t bdfe;
	int filePointer = 0;

	int i,j,k;
	double d;
	dword filesize, num_number;

	double* p2=0;

	// get file size
	int rr = _ksys_file_get_info(edit_path, &bdfe);
	sprintf(debuf, "getsize: %d\n", rr);
	_ksys_debug_puts(debuf);
	if (rr != 0) {
		_ksys_draw_text((char*)er_file_not_found, 10, 10, strlen(er_file_not_found), 0x90FF0000);
		return 0;
	}

	filesize = bdfe.size;
	num_number = filesize / 2;

	HugeBuf = (char*)malloc(filesize + 1); // разбираем как строку, отсюда терминатор \0

	for (i=0;i<filesize+1;i++)
		HugeBuf[i] = 0;

	rr = _ksys_file_read_file(edit_path, 0, filesize, HugeBuf, nullptr);
	sprintf(debuf, "read3: %d\n", rr);
	_ksys_debug_puts(debuf);

	strcpy(full_head, STR_PROGRAM_TITLENAME);
	strcpy(full_head+strlen(full_head), " - ");
	strcpy(full_head+strlen(full_head), edit_path);		// bad code

	// а теперь разобраться в этом

	i=0;
	k=0;
	while (i < filesize) {

		while (isalpha(HugeBuf[i]) && i<filesize) i++;
		if (i == filesize) break;
		if (k==4 && HugeBuf[i] == '=') {
			//sprintf(debuf,"function: %S",HugeBuf + i);
			//_ksys_debug_puts(debuf);
			// we have a function here
			//HugeBuf[0] = ' ';
			funct = HugeBuf + i + 1;
			strcpy(full_head+strlen(full_head), ". Function y=");
			strcpy(full_head+strlen(full_head), funct);
			return 1;
		}

		d = convert(HugeBuf+i, &j);
		if (d == ERROR) {
			sprintf(debuf, "Error in input file, byte %d, count %d\n", i, k);
			_ksys_debug_puts(debuf);
			_ksys_draw_text((char*)debuf, 10, 10, strlen(debuf), 0x000000);
			return 0;
		}
		if (d == ERROR_END) {
			_ksys_debug_puts("EOF :)!\n");
			break;
		}

		i+=j;
		switch (k) {
			case 0:
				x1=d;
				break;
			case 1:
				x2=d;
				break;
			case 2:
				y1=d;
				break;
			case 3:
				y2=d;
				break;
			default: {
				if (p2 == NULL)
					p2 = (double*)malloc(num_number * 8);
				p2[k-4]=d;
			}
		}
		k++;
	}
//	sprintf(debuf, "(%f,%f)-(%f,%f)",x1,y1,x2,y2);
//	_ksys_debug_puts(debuf);
	point_count=(k - 4)/2;

	//
	points = (double*)malloc(point_count * 2 * 8);
	for (i = 0; i < point_count * 2; i++)
		points[i] = p2[i];
	free(p2);
//	sprintf(debuf, "count: %d\n", point_count);
//	_ksys_debug_puts(debuf);
	sprintf(debuf, ". Number of points: %u.", point_count);
	strcpy(full_head+strlen(full_head), debuf);
	free(HugeBuf);
	HugeBuf = NULL;
	return 1;
}

// вычислить заданную функцию или кусочно-линейную между точками
double fu(double x) {
	int i;
	double res;

	if (funct) {
		set_exp(funct,x);
		get_exp(&res);		// парсить для каждого значения х? да я с ума сошел.
		return res;
	}

	if (point_count == 0) {
		return 0.0;
	}

	if (x <= points[0])
		return points[1];
	if (x >= points[(point_count-1) * 2])
		return points[(point_count-1) * 2 + 1];

	for (i = 0; i < point_count; i++) {
		if ((x >= points[2 * i]) && (x < points[2 * (i + 1)]))
			break;
	}

	return (x - points[2 * i]) / (points[2 * (i + 1)] - points[2 * i])
		* (points[2 * (i + 1) + 1] - points[2 * i + 1]) + points[2 * i + 1];

}

void draw_window(void) {
	double xx0=0.0, yy0=0.0;

	_ksys_start_draw();
	_ksys_create_window(100, 80, WND_W, WND_H, full_head, 0xFFFFFF, 0x33);
	_ksys_end_draw();

	ksys_thread_t info;
	_ksys_thread_info(&info, 0xFFFFFFFF);
	int cWidth = info.winx_size - 9;
	int cHeight = info.winy_size - _ksys_get_skin_height() - 4;

	mybox.top = cHeight - 50;
	mybox.width = cWidth - mybox.left - 80;

	if (info.window_state&0x04) return; //draw nothing if window is rolled-up

	if (point_count == 0 && funct == NULL) {
		_ksys_draw_text((char *)empty_text, (cWidth - 8 * strlen(empty_text))/2,cHeight/2-25, strlen(empty_text), 0x90000000);
	} else {
		drawFunction(&fu, coord(10, 20), coord(cWidth - 20, cHeight - 70), coord(x1,y1), coord(x2,y2), 0x00ff0000);
	}

	_ksys_draw_text((char*)str_filename, 15, mybox.top + 4, strlen(str_filename), 0x90000000);

	edit_box_draw(&mybox);

	_ksys_define_button(cWidth - 70, mybox.top, 50, 21, BTN_EDIT, 0xc0c0c0);
	_ksys_draw_text((char*)str_editfile, cWidth - 60, mybox.top + 4, 0, 0x90000000);
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
	full_head = (char*)malloc(300);
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
			if (load_points3())
				draw_window();
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
					case 0x0D:
						if (HugeBuf) {
							//sprintf(debuf, "freemem: HugeBuf = %X", HugeBuf);
							//_ksys_debug_puts(debuf);
							free(HugeBuf); // что за баг - понять не могу.
							HugeBuf = nullptr;
							funct = nullptr;
						}
						if (points) {
							//sprintf(debuf, "freemem: points = %X", points);
							//_ksys_debug_puts(debuf);
							free(points); // и тут. ну не обращаюсь я к этому указателю, только память в него, потом снова выделяю
							points = nullptr;
						}
						point_count = 0;
						_ksys_draw_bar(10, 10, 200, 20, 0xFFFFFF); // фон для сообщений об ошибках
						if (load_points3())
							draw_window();
						break;
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
