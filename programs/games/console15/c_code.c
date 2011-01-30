#include "system/kolibri.h"
#include "system/stdlib.h"
#include "system/string.h"

char CONSOLE[] = "/sys/lib/console.obj";

#define MENTION	_printf("Arrows to move left, up, right or down, or 'Esc' to exit: \n\n");

int** field;
int emptyCell_x, emptyCell_y;

char NOT_VALID_MOVE[] = {"Not valid move.\n\n"};

void (* _stdcall con_init)(unsigned w_w, unsigned w_h, unsigned s_w, unsigned s_h, const char* t);
void (* _cdecl _printf)(const char* format,...);
void (* _stdcall __exit)(char bCloseWindow);
 int (* _stdcall _getch)(void);


//------------------
void init()
{
	int x,y, i,j;
	srand( kol_system_time_get() );
	for(i=1; i<=15;)
	{
		x=rand()%4; y=rand()%4;
		if(field[x][y] == 0) field[x][y] = i++;		
	}

	for(i=0; i<4; i++) //to find the empty cell
		for(j=0; j<4; j++)
			if(field[j][i] == 0) 
			{
				emptyCell_x=j; emptyCell_y = i; return;
			}

}

//---------------------
void printField()
{
	int i,j;
	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
			if(field[j][i]) _printf("%3d", field[j][i]);
			else _printf("  _");
		_printf("\n\n");
	}
	_printf("\n\n");
}

//-----------------------
int notEndYet()
{
	int i,j;
	for(i=0; i<3; i++)
		for(j=0; j<4; j++)
			if(field[j][i] != 4*i+j+1) return 0; //go on play
	if(field[0][3] != 13) return 0;

	return 1; //victory!
}

//---------------   allows move the emply cell
int move()
{
	
unsigned short c;
	
while(1)
	{
	c = _getch(); 
	switch(c)
		{
		case 0x4d00:
			if(emptyCell_x==0)
				{
				_printf(NOT_VALID_MOVE);
				break;
				}
			else
				{
				field[emptyCell_x][emptyCell_y] = field[emptyCell_x-1][emptyCell_y];
				field[emptyCell_x-1][emptyCell_y] = 0;
				emptyCell_x--;
				return 1;
				}
			case 0x5000:
				if(emptyCell_y==0)
					{
					_printf(NOT_VALID_MOVE);
					break;
					}
				else
					{
					field[emptyCell_x][emptyCell_y] = field[emptyCell_x][emptyCell_y-1];
					field[emptyCell_x][emptyCell_y-1] = 0;
					emptyCell_y--;
					return 2;
					}
			case 0x4b00:
				if(emptyCell_x==3)
					{
					_printf(NOT_VALID_MOVE);
					break;
					}
				else
					{
					field[emptyCell_x][emptyCell_y] = field[emptyCell_x+1][emptyCell_y];
					field[emptyCell_x+1][emptyCell_y] = 0;
					emptyCell_x++;
					return 3;
					}
			case 0x4800:
				if(emptyCell_y==3)
					{
					_printf(NOT_VALID_MOVE);
					break;
					}
				else
					{
					field[emptyCell_x][emptyCell_y] = field[emptyCell_x][emptyCell_y+1];
					field[emptyCell_x][emptyCell_y+1] = 0;
					emptyCell_y++;
					return 4;
					}
			case 0x011b: __exit(1);
			default: MENTION
		}
	}
}

//----------------- main function
void kol_main()
{

int i;
kol_struct_import *imp;

imp = kol_cofflib_load(CONSOLE);
if (imp == NULL)
	kol_exit();

con_init = ( _stdcall  void (*)(unsigned, unsigned, unsigned, unsigned, const char*)) 
		kol_cofflib_procload (imp, "con_init");
if (con_init == NULL)
	kol_exit();

_printf = ( _cdecl void (*)(const char*,...))
		kol_cofflib_procload (imp, "con_printf");
if (_printf == NULL)
	kol_exit();

__exit = ( _stdcall void (*)(char))
		kol_cofflib_procload (imp, "con_exit");

if (__exit == NULL)
	kol_exit();


_getch = ( _stdcall int (*)(void))
		kol_cofflib_procload (imp, "con_getch2");

if (_getch == NULL)
	kol_exit();

con_init(-1, -1, -1, -1, "Console15 by O.Bogomaz");

field = (int**)malloc(4 * sizeof(int*));

for( i=0; i<4; i++)
	field[i] = (int*)malloc(4 * sizeof(int));

do 
	init(); 
while(notEndYet());
	
MENTION

printField();
	
while(!notEndYet())
	{
	move();
	printField();
	}

_printf("\nYou win!\n");
__exit(0);
}
