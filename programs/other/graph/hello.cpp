#include "func.h"
#include "parser.h"
#include "kolibri.h"
#include "use_library.h"

const char header[] = "Graph";
const char empty_text[] = "No function loaded. Type file name and press Enter. ";
const char er_file_not_found[] = "Cannot open file. ";
const char str_filename[]="Filename:";
const char str_editfile[]="Edit";

// начальные размеры
#define WND_W 400
#define WND_H 300

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
#define FORMAT "%f"
// format for two coords
#define FORMAT_COORD "(%f,%f)"
// special value to text if enough space
#define FORMAT_TEST "0.00"

#define DELTA_BIG 1.0
#define DELTA_SMALL 0.1

double *points;
Dword point_count = 0;
double x1,y1,x2,y2;
char *funct = NULL;

char edit_path[1024];
//Dword editbox_y = WND_H - 16, editbox_w = WND_W - 70;
edit_box mybox = {0,9*8-5,WND_H - 16-32,0xffffff,0x6a9480,0,0x808080,0,99,(dword)&edit_path,0};

char *full_head;

char *HugeBuf = NULL;

//char fuck[64] = "$this is a fucking marker$";
// параметры командной строки
#ifdef AUTOBUILD
extern char params[1024];
char params[1024];
#else
char params[1024] = "_FIND_ME_";
#endif

/*

  fucking piece of shit

  */

// constructor of TCoord
TCoord coord(double x, double y)
{
  TCoord r;
  r.x = x;
  r.y = y;
  return r;
}

// move and scale mathematical coords to fit screen coords
TCoord mat2Graf(TCoord c, TCoord scrMin, TCoord scrMax, TCoord mMin, TCoord mMax)
{
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

// double-обертки
void line_d( double x1, double y1, double x2, double y2)
{
   line(di(x1), di(y1), di(x2), di(y2));
}

void outtextxy_d( double x, double y, char * text, int len)
{
   outtextxy(di(x), di(y), text, len);
}

// huge function to draw all the stuff except the function itself
void drawAxis( TCoord scrMin, TCoord scrMax, TCoord mMin, TCoord mMax)
{
  TCoord cZero={0.0,0.0},
	   gMin, gMax, gZero, step;
  TCoord from, to;
  double i=0.0;
  int j;
  double xmin, xmin2, ymin, ymin2;
  char buf[30]="";


// scr means Screen(bounding rect)
// m   means Mathematical
// g   means Graphic(real screen position)

  //rtlDebugOutString("draw axis called\n");

  //format(debuf, 30, "test: %f,%f,%f,%f\n", 123.45, 1.0, -0.9, 12.57);
  //rtlDebugOutString(debuf);

  gMin = mat2Graf(mMin, scrMin, scrMax, mMin, mMax);
  gMax = mat2Graf(mMax, scrMin, scrMax, mMin, mMax);
  gZero = mat2Graf(cZero, scrMin, scrMax, mMin, mMax);

  // clear
 // setcolor(WHITE);
 //rectangle(di(gMin.x), di(gMin.y), di(gMax.x), di(gMax.y));
  // ftopku

  setcolor(BLACK);
  // osy X
  line_d(gMin.x, gZero.y ,gMax.x, gZero.y);
  // osy Y
  line_d(gZero.x, gMin.y, gZero.x, gMax.y);
  // bounding rect
  line_d(gMin.x, gMin.y, gMax.x, gMin.y);
  line_d(gMin.x, gMax.y, gMax.x, gMax.y);

  line_d(gMin.x, gMin.y, gMin.x, gMax.y);
  line_d(gMax.x, gMin.y, gMax.x, gMax.y);

  // coords of the rect : lower left
  format(buf, 30, FORMAT_COORD, x1, y1);
  //rtlDebugOutString(buf);
  outtextxy_d(gMin.x, gMin.y + textheight(buf, 20), buf, 20);
  // upper left
  format(buf, 30, FORMAT_COORD, x1, y2);
  outtextxy_d(gMin.x, gMax.y - textheight(buf, 20), buf, 20);
  // lower right
  format(buf, 30, FORMAT_COORD, x2, y1);
  outtextxy_d(gMax.x - textwidth(buf, 20), gMin.y + textheight(buf, 20), buf, 20);
  // upper right
  format(buf, 30, FORMAT_COORD, x2, y2);
  outtextxy_d(gMax.x - textwidth(buf, 20), gMax.y - textheight(buf, 20), buf, 20);

  //rtlDebugOutString("some lines painted\n");


  step.x = (mMax.x - mMin.x) / (scrMax.x - scrMin.x);
  step.y = (mMax.y - mMin.y) / (scrMax.y - scrMin.y);

// round values
  xmin = id(di((mMin.x / DELTA_BIG) * DELTA_BIG));
  ymin = id(di((mMin.y / DELTA_BIG) * DELTA_BIG));

  // (0,0)

  if ((x1 * x2 <= 0.0) && (y1 * y2 <= 0.0))
  {
	  from.x=0.0;
	  from.y=0.0;
	  from = mat2Graf(from, scrMin, scrMax, mMin, mMax);
	  setcolor(BLACK);
	  format(buf, 30, FORMAT, 0.0);
	  outtextxy_d(from.x - textwidth(buf, 20), from.y + textheight(buf, 20), buf, 20);
  }


  // big marks on X
  //settextstyle(0, 0, 1);
  if (DELTA_BIG / step.x > THREE)
  {
    for (i = xmin; i <= mMax.x; i += DELTA_BIG)
    {
	  if (i != 0.0)
	  {
		  from.x = i;
		  to.x = from.x;
		  from.y = -BIG_HEIGHT * step.y;
		  to.y = BIG_HEIGHT * step.y;
		  from = mat2Graf(from, scrMin, scrMax, mMin, mMax);
		  to = mat2Graf(to, scrMin, scrMax, mMin, mMax);
		  setcolor(BLACK);
		  line_d(from.x, from.y, to.x, to.y);
		  // write number
		  format(buf, 30, FORMAT, i);
		  // if it fits in the GAP, then write it
		  if (from.y > scrMin.y && (DELTA_BIG > (textwidth(buf, 20) + 1.0) * step.x))
		  {
			   setcolor(BIGFONTCOLOR);
			   outtextxy_d(from.x - textwidth(buf, 20) / 2.0, to.y - textheight(buf, 20), buf, 20);
		  }
	  }
    }
  }
  //rtlDebugOutString("big marks x painted\n");

  // big marks on Y
  if (DELTA_BIG / step.y > THREE)
  {
    for (i = ymin; i <= mMax.y; i += DELTA_BIG)
    {
	  if (i != 0.0)
	  {
		  from.y = i;
		  to.y = from.y;
		  from.x = -BIG_HEIGHT * step.x;
		  to.x = BIG_HEIGHT * step.x;
		  from = mat2Graf(from, scrMin, scrMax, mMin, mMax);
		  to = mat2Graf(to, scrMin, scrMax, mMin, mMax);
		  setcolor(BLACK);
		  line_d(from.x, from.y, to.x, to.y);
		  format(buf, 30, FORMAT, i);
		  if (from.x > scrMin.x && (DELTA_BIG > textheight(buf, 20) * step.y))
		  {
			   setcolor(BIGFONTCOLOR);
			 outtextxy_d(from.x + TEXT_X, to.y - textheight(buf, 20) / 2.0, buf, 20);
		  }
	  }
    }
  }

  xmin2 = id(di(mMin.x / DELTA_SMALL)) * DELTA_SMALL;
  ymin2 = id(di(mMin.y / DELTA_SMALL)) * DELTA_SMALL;

  if (DELTA_SMALL / step.x  > THREE)
  {
    j = di((( - xmin + xmin2 ) / DELTA_SMALL));
    for (i = xmin2; i <= mMax.x; i += DELTA_SMALL, j++)
    {
      if (j % 10 == 0)
      {
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
      setcolor(BLACK);
      line_d(from.x, from.y, to.x, to.y);
      format(buf, 30, FORMAT, i);
	  
      if (from.y > scrMin.y && (DELTA_SMALL > textwidth(buf, 20) * step.x))
      {
		setcolor(SMALLFONTCOLOR);
		outtextxy_d(from.x - textwidth(buf, 20) / 2.0, to.y - textheight(buf, 20), buf, 20);
      }
	  

    }
      
  }

  // finally small marks on Y
  if (DELTA_SMALL / step.y > THREE)
  {
    //rtlDebugOutString("really small marks y painted\n");
    j = di((( - ymin + ymin2) / DELTA_SMALL));
    for (i = ymin2; i <= mMax.y; i += DELTA_SMALL, j++)
    {
      if (j % 10 == 0)
      {
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
      setcolor(BLACK);
      line_d(from.x, from.y, to.x, to.y);
      format(buf, 30, FORMAT, i);
      if (from.x > scrMin.x && (DELTA_SMALL > textheight(buf, 20) * step.y))
      {
       	setcolor(SMALLFONTCOLOR);
       	outtextxy_d(from.x + TEXT_X, from.y - textheight(buf, 20) / 2.0, buf, 20);
      }
    }
  }

}

/*
  ends fucking piece of shit
*/

void drawFunction( function_t fi, TCoord scrMin, TCoord scrMax,
		   TCoord mMin, TCoord mMax, DWORD color)
{
  double x;
  double y;
  int firstPoint = 1;
  TCoord p, p0 = {0.0, 0.0}, step;

  drawAxis(scrMin, scrMax, mMin, mMax);

  setcolor(color);
  step.x = (mMax.x - mMin.x) / (scrMax.x - scrMin.x);

  for (x = mMin.x; x < mMax.x; x += step.x)
  {
    y = fi(x);
// function is defined here and gets in the range
    if (1) // тут было условие, что функция правильно вычислена
    {
      if ((y > mMin.y) && (y < mMax.y))
      {
	      p = mat2Graf(coord(x, y), scrMin, scrMax, mMin, mMax);
	// if it's our first point, only remember its coords
	// otherwise, draw a line_d from prev to current
      	if (firstPoint == 0)
        {
      	  line_d(p0.x, p0.y, p.x, p.y);
        }
      	else
    	  firstPoint = 0;

     	p0 = p;
      }
      else // too big/small
      {
	      firstPoint = 1;
      }
    }
    else // no value
    {
      firstPoint = 1;
    }
  }

}

struct kosBDVK 
{
	Dword attrib;
	Dword name_type;
	Dword create_time;
	Dword create_date;
	Dword access_time;
	Dword access_date;
	Dword modify_time;
	Dword modify_date;
	Dword size_low;
	Dword size_high;	
};

// итоговая версия читалки текстовых файлов
int load_points3()
{
	kosFileInfo fileInfo;
	kosBDVK bdvk;
	int filePointer = 0;

	int i,j,k;
	double d;
	Dword filesize, num_number;

	double *p2;

	if (edit_path[0] == '\0')
		return 0;

	// get file size
	strcpy(fileInfo.fileURL,edit_path);
	fileInfo.OffsetLow = 0;
	fileInfo.OffsetHigh = 0;
	fileInfo.dataCount = 0;
	fileInfo.rwMode = 5;
	fileInfo.bufferPtr = (Byte *)&bdvk;
	Dword rr = kos_FileSystemAccess( &(fileInfo) ); // в CKosFile нет определения размера
	sprintf(debuf, "getsize: %U\n", rr);
	rtlDebugOutString(debuf);
	if (rr != 0)
	{
		kos_WriteTextToWindow(10,10,0,0x00,(char*)er_file_not_found,strlen(er_file_not_found));
		return 0;
	}

	filesize = bdvk.size_low;
	num_number = filesize / 2;

	HugeBuf = (char *)allocmem(filesize + 1); // разбираем как строку, отсюда терминатор \0

	for (i=0;i<filesize+1;i++)
		HugeBuf[i] = 0;

	strcpy(fileInfo.fileURL,edit_path);
	fileInfo.OffsetLow = 0;

	fileInfo.OffsetHigh = 0;
	fileInfo.dataCount = filesize;
	fileInfo.rwMode = 0;
	fileInfo.bufferPtr = (Byte *)HugeBuf;
	rr = kos_FileSystemAccess( &(fileInfo) );	// какая-то проблема с hands.dll, CKosFile не работал
		
	sprintf(debuf, "read3: %U\n", rr);
	rtlDebugOutString(debuf);

	strcpy(full_head, header);
	strcpy(full_head+strlen(full_head), " - ");
	strcpy(full_head+strlen(full_head), edit_path);		// bad code

	// а теперь разобраться в этом

	i=0;
	k=0;
	while (i < filesize)
	{

		while (isalpha(HugeBuf[i]) && i<filesize) i++;
		if (i == filesize) break;
		if (k==4 && HugeBuf[i] == '=')
		{
			//sprintf(debuf,"function: %S",HugeBuf + i);
			//rtlDebugOutString(debuf);
			// we have a function here
			//HugeBuf[0] = ' ';
			funct = HugeBuf + i + 1;
			strcpy(full_head+strlen(full_head), ". Function y=");
			strcpy(full_head+strlen(full_head), funct);
			return 1;
		}

		d = convert(HugeBuf+i, &j);
		if (d == ERROR)
		{
			sprintf(debuf, "Error in input file, byte %U, count %U\n", i, k);
			rtlDebugOutString(debuf);
			kos_WriteTextToWindow(10, 10, 0, 0x00, (char*)debuf, strlen(debuf));
			return 0;
		}
		if (d == ERROR_END)
		{
			rtlDebugOutString("EOF :)!\n");
			break;
		}
		
		i+=j;
		switch (k)
		{
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
		default:
			{
				if (p2 == NULL)
					p2 = (double *)allocmem(num_number * 8);
				p2[k-4]=d;
			}
		}
		k++;
	}
//	format(debuf, 30, "(%f,%f)-(%f,%f)",x1,y1,x2,y2);
//	rtlDebugOutString(debuf);
	point_count=(k - 4)/2;

	//
	points = (double *)allocmem(point_count * 2 * 8);
	for (i = 0; i < point_count * 2; i++)
		points[i] = p2[i];
	freemem(p2);
//	sprintf(debuf, "count: %U\n", point_count);
//	rtlDebugOutString(debuf);
	sprintf(debuf, ". Number of points: %U.", point_count);
	strcpy(full_head+strlen(full_head), debuf);
	freemem(HugeBuf);
	HugeBuf = NULL;
	return 1;
}

void LaunchTinypad()
{
	kosFileInfo fileInfo;

	strcpy(fileInfo.fileURL,"/sys/tinypad");
	fileInfo.OffsetLow = 0;
	fileInfo.OffsetHigh = (Dword)edit_path;
	fileInfo.rwMode = 7;	// launch
	kos_FileSystemAccess(&fileInfo);

}

// вычислить заданную функцию или кусочно-линейную между точками
double fu(double x)
{
	int i;
	double res;

	
	if (funct)
	{
		set_exp(funct,x); 
		get_exp(&res);		// парсить для каждого значения х? да я с ума сошел.
		return res;
	}

	if (point_count == 0)
	{
		return 0.0;
	}

	if (x <= points[0]) 
		return points[1];
	if (x >= points[(point_count-1) * 2])
		return points[(point_count-1) * 2 + 1];

	for (i = 0; i < point_count; i++)
	{
		if ((x >= points[2 * i]) && (x < points[2 * (i + 1)]))
			break;
	}

	return (x - points[2 * i]) / (points[2 * (i + 1)] - points[2 * i])
		* (points[2 * (i + 1) + 1] - points[2 * i + 1]) + points[2 * i + 1];

}

void draw_window(void)
{
	double xx0=0.0, yy0=0.0;
	sProcessInfo info;
	Dword wi, he;
	void *p;

	for (int i = 0; i < 1024; i++)
		info.rawData[i] = 0;
	kos_ProcessInfo(&info, 0xFFFFFFFF);

	p = info.rawData + 42;			// magic
	wi = *(Dword *)(p);
	he = *(Dword *)((Byte *)p + 4);

	if (wi == 0) 
		wi = WND_W;
	if (he == 0)
		he = WND_H;

	mybox.top = he - 45;
	mybox.width = wi - mybox.left - 80;

	kos_WindowRedrawStatus(1);
	kos_DefineAndDrawWindow(10,40,WND_W,WND_H, 0x33,0xFFFFFF,0,0,(Dword)full_head);
	kos_WindowRedrawStatus(2);

	if (info.rawData[70]&0x04) return; //ничего не делать если окно схлопнуто в заголовок

	if (point_count == 0 && funct == NULL)
	{
		kos_WriteTextToWindow((wi - 6 * strlen(empty_text))/2,he/2,0,0x000000,(char *)empty_text,strlen(empty_text));
	}
	else
	{
		drawFunction(&fu, coord(10, 20), coord(id(wi - 20), id(he - 70)),
							coord(x1,y1), coord(x2,y2), 0x00ff0000);

	}

	kos_WriteTextToWindow(4, mybox.top + 4, 0, 0, (char*)str_filename, strlen(str_filename));

	if ((void*)edit_box_draw != NULL)
		edit_box_draw((DWORD)&mybox);

	kos_DefineButton(wi - 70, mybox.top, 50, 12, 5, 0xc0c0c0);
	kos_WriteTextToWindow(wi - 58, mybox.top + 4, 0, 0, (char*)str_editfile, strlen(str_editfile));

}

void kos_Main()
{
	kos_InitHeap();
	full_head = (char*)allocmem(300);
	strcpy(full_head, "Graph");
	load_edit_box();
	if (params[0]) // fuck[0] for debug
	{
		rtlDebugOutString("launched with params");
		rtlDebugOutString((char*)params);
		strcpy(edit_path, params);
		//rtlDebugOutString((char*)edit_path);
		load_points3();
	}
	rtlDebugOutString("data loaded.\n");
	draw_window();
	for (;;)
	{
		edit_box_mouse((dword)&mybox);
		switch (kos_WaitForEvent())
		{
		case 1:
			draw_window();
			break;
		case 2:
			// key pressed, read it
			Byte keyCode;
			kos_GetKey(keyCode);

			switch (keyCode)
				{
					case 0x0D:
							if (HugeBuf!=NULL)
							{
								//sprintf(debuf, "freemem: HugeBuf = %X", HugeBuf);
								//rtlDebugOutString(debuf);
								freemem((void*)HugeBuf);		// что за баг - понять не могу.
								HugeBuf = NULL;
								funct = NULL;
							}
							if (points!=NULL)
							{
								//sprintf(debuf, "freemem: points = %X", points);
								//rtlDebugOutString(debuf);
								freemem((void*)points);		// и тут. ну не обращаюсь я к этому указателю, только память в него
														// потом снова выделяю
								points = NULL;
							}
							point_count = 0;
							kos_DrawBar(10,10,200,20,0xFFFFFF); // фон для сообщений об ошибках
							if (load_points3())
								draw_window();
							break;
					default:
						{
							__asm
							{
								mov ah, keyCode
							}
							edit_box_key((dword)&mybox);
						}
				}
			break;


		case 3:
			// button pressed; we have only one button, close
			Dword button;
			kos_GetButtonID(button);
			if (button == 1)
				kos_ExitApp();
			if (button == 5)
				LaunchTinypad();
		}
	}
}

