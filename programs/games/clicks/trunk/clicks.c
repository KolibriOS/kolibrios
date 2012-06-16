//Leency 10.10.2011, JustClicks v2.0, GPL

#include "lib\kolibri.h" 
#include "lib\random.h"
#include "lib\boxes.txt"
system_colors sc;

//уровни сложности
int DIFFICULTY_LEVEL;
char *BOARD_SIZES[]={ "S", "M", "L", 0 };
int DIFFICULTY_LEV_PARAMS[]={ 9, 12, 16 };

int BLOCKS_NUM; //количество квадратиков по Х и по Y
int BLOCKS_LEFT; //блоков осталось
int blocks_matrix[28*28]; //цвета для поля с квадратиками

#define USER_PANEL_HEIGHT 35
#define BLOCK_SIZE 21 //размер квадратика
#define MARKED 7
#define DELETED_BLOCK 6
#define HEADER "Just Clicks v2.0"

#ifndef AUTOBUILD
#include "lang.h--"
#endif

#ifdef LANG_RUS
	char NEW_GAME_TEXT[]=" ‡ ­®ў® [F2]"; 
	char REZULT_TEXT[]="ђҐ§г«мв в: ";
#else
	char NEW_GAME_TEXT[]="New Game [F2]"; 
	char REZULT_TEXT[]="Rezult: ";
#endif


void main()
{   
	int key, id;
	
	BLOCKS_NUM=DIFFICULTY_LEV_PARAMS[0]; //по-умолчанию самое маленькое поле
	
	new_game();
   
	loop()
		switch(WaitEvent()) 
		{
			case evButton:
				id = GetButtonID(); 
				if (id==1) ExitProcess();
				if (id==2) goto _NEW_GAME_MARK;
				if (id>=100)
				{
					if (check_for_end()) break; //если игра закончена
					
					move_blocks(id-100);
					draw_field();
					
					draw_clicks_num();
					
					break;
				}
				if (id==10) //изменяем размер поля
				{
					if (DIFFICULTY_LEVEL<2) DIFFICULTY_LEVEL++; else DIFFICULTY_LEVEL=0;
					
					BLOCKS_NUM = DIFFICULTY_LEV_PARAMS[DIFFICULTY_LEVEL]; //количество квадратиков по Х и по Y
					
					new_game();
					
					MoveSize(-1, -1, BLOCK_SIZE*BLOCKS_NUM +9, BLOCK_SIZE*BLOCKS_NUM +GetSkinWidth()+4+USER_PANEL_HEIGHT);
					break;
				}
				break;
			case evKey:
				key = GetKey();
				if (key==027) //Escape
					 ExitProcess();
				if (key==051) //F2
				{
					_NEW_GAME_MARK:
						new_game();
						draw_clicks_num();
						draw_field();
				}
				break;
			case evReDraw:
				draw_window();
		}
}


void move_blocks(int button_id) //если фишка одна, то не удаляем
{
	int i, j,
	marked_num=1,
	old_marker=blocks_matrix[button_id],
	restart;

	blocks_matrix[button_id]=MARKED;

	//выделяем все фишки того же чвета
	_RESTART_MARK:
	restart=0;
	for (i=0;i<BLOCKS_NUM;i++)
		for (j=0;j<BLOCKS_NUM;j++)
		{
			if (blocks_matrix[i*BLOCKS_NUM+j]<>old_marker) continue; //если фишка не нужного цвета идём дальше
			if (blocks_matrix[i*BLOCKS_NUM+j]==MARKED) continue; //если фишка уже отмечена, идём далее
			
			if (j>0) && (blocks_matrix[i*BLOCKS_NUM+j-1]==MARKED) blocks_matrix[i*BLOCKS_NUM+j]=MARKED; //смотрим левый
			if (i>0) && (blocks_matrix[i-1*BLOCKS_NUM+j]==MARKED) blocks_matrix[i*BLOCKS_NUM+j]=MARKED; //смотрим верхний
			if (j<BLOCKS_NUM-1) && (blocks_matrix[i*BLOCKS_NUM+j+1]==MARKED) blocks_matrix[i*BLOCKS_NUM+j]=MARKED; //смотрим правый
			if (i<BLOCKS_NUM-1) && (blocks_matrix[i+1*BLOCKS_NUM+j]==MARKED) blocks_matrix[i*BLOCKS_NUM+j]=MARKED; //смотрим нижний
			
			if (blocks_matrix[i*BLOCKS_NUM+j]==MARKED) //если фишку отметили, то потом цикл нужно будет прокрутить сначала - мож ещё чё отметим
			{
				restart=1;
				marked_num++;
			}
		}
	if (restart) goto _RESTART_MARK;
	
	if (marked_num==1) //если блок только один, уходим
	{
		blocks_matrix[button_id]=old_marker;
		return;
	}

	//двигаем блоки по вертикали
	_2_RESTART_MARK:
	restart=0;
	for (i=BLOCKS_NUM;i>0;i--)
		for (j=BLOCKS_NUM;j>=0;j--)
		{
			if (blocks_matrix[i*BLOCKS_NUM+j]==MARKED) && (blocks_matrix[i-1*BLOCKS_NUM+j]<>blocks_matrix[i*BLOCKS_NUM+j])
			{
				blocks_matrix[i*BLOCKS_NUM+j]><blocks_matrix[i-1*BLOCKS_NUM+j];
				restart=1;
			}
		}
	if (restart) goto _2_RESTART_MARK;
	
	//отмечаем фишки, как удалённые
	for (i=0;i<BLOCKS_NUM*BLOCKS_NUM;i++) 
			if (blocks_matrix[i]==MARKED)
				blocks_matrix[i]=DELETED_BLOCK;
				
	//двигаем блоки влево, если есть пустой столбец
	restart=BLOCKS_NUM; //не придумал ничего лучше :(
	
	_3_RESTART_MARK:
	for (j=0;j<BLOCKS_NUM-1;j++)
		if (blocks_matrix[BLOCKS_NUM-1*BLOCKS_NUM+j]==DELETED_BLOCK)
		{
			for (i=0;i<BLOCKS_NUM;i++)
				blocks_matrix[i*BLOCKS_NUM+j]><blocks_matrix[i*BLOCKS_NUM+j+1];
		}
	restart--;
	if (restart) goto _3_RESTART_MARK;
}


void draw_window()
{
	int j, PANEL_Y;
	proc_info Form;
	
	sc.get();
	DefineAndDrawWindow(300,176, BLOCK_SIZE*BLOCKS_NUM +9, BLOCK_SIZE*BLOCKS_NUM +GetSkinWidth()+4+USER_PANEL_HEIGHT,
		0x74,sc.work,0,0,HEADER); 
	
	//проверяем не схлопнуто ли окно в заголовок
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return;


	PANEL_Y=BLOCK_SIZE*BLOCKS_NUM;

	DrawBar(0,PANEL_Y, PANEL_Y, USER_PANEL_HEIGHT, sc.work); //панель снизу
		
	//новая игра
	DefineButton(10,PANEL_Y+7, 13*6+6, 20, 2,sc.work_button);
	WriteText(10+4,PANEL_Y+14,0x80,sc.work_button_text,#NEW_GAME_TEXT,0);


	//кнопочкa выбора уровня сложности
	DefineButton(95,PANEL_Y+7, 20,20, 10,sc.work_button);
	WriteText(95+8,PANEL_Y+14,0x80,sc.work_button_text,BOARD_SIZES[DIFFICULTY_LEVEL],0);
	
	draw_field();
	
	draw_clicks_num();
}


int check_for_end()
{
	int i, j, button_id;

	if (!BLOCKS_LEFT) return 1; //epic win

	for (i=0;i<BLOCKS_NUM;i++)
		for (j=0;j<BLOCKS_NUM;j++)
		{
			button_id=blocks_matrix[i*BLOCKS_NUM+j];
			
			if (button_id==DELETED_BLOCK) continue;
			
			if (j>0) && (blocks_matrix[i*BLOCKS_NUM+j-1]==button_id) return 0;
			if (i>0) && (blocks_matrix[i-1*BLOCKS_NUM+j]==button_id) return 0;
			if (j<BLOCKS_NUM-1) && (blocks_matrix[i*BLOCKS_NUM+j+1]==button_id) return 0;
			if (i<BLOCKS_NUM-1) && (blocks_matrix[i+1*BLOCKS_NUM+j]==button_id) return 0;
		}
	return 2; 
}


void draw_clicks_num()
{
	char rezult[15];
	int i;
	int TEXT_Y=BLOCK_SIZE*BLOCKS_NUM+14;
	int TEXT_X=TEXT_Y/2+48; //130;

	BLOCKS_LEFT=0;
	
	for (i=0;i<BLOCKS_NUM*BLOCKS_NUM;i++) 
		if (blocks_matrix[i]<>DELETED_BLOCK) BLOCKS_LEFT++;	
	
	DrawBar(TEXT_X, TEXT_Y, 18,9, sc.work);
	
	WriteText(TEXT_X,TEXT_Y,0x80,sc.work_button_text,IntToStr(BLOCKS_LEFT),0);
	if (check_for_end())
	{
		copystr(#REZULT_TEXT, #rezult);
		copystr(IntToStr(BLOCKS_LEFT), #rezult+strlen(#rezult));
		if (check_for_end()==1) copystr("Epic WIN!!1", #rezult);
		DrawFlatButton(BLOCK_SIZE*BLOCKS_NUM/2-70, BLOCK_SIZE*BLOCKS_NUM/2-20, 140, 40, 2, #rezult);
	}
}


void new_game()
{
	int i;

	//пять цветов используется в игре для квадратиков, MARKED для того,
	//чтобы отметить квадратики в процессе заливки и DELETED_BLOCK  для их удаления
	for (i=0;i<BLOCKS_NUM*BLOCKS_NUM;i++)
		blocks_matrix[i] = random(5);
}


void draw_field()
{
	int i, j;
	int current_id;
	
	for (i=0;i<BLOCKS_NUM;i++)
		for (j=0;j<BLOCKS_NUM;j++)
		{
			current_id = i*BLOCKS_NUM+j;
			DeleteButton(current_id+100);
			if (blocks_matrix[current_id]==DELETED_BLOCK)
			{
				DrawBar(j*BLOCK_SIZE,i*BLOCK_SIZE, BLOCK_SIZE,BLOCK_SIZE, 0xB2B4BF);
			}
			else
			{
				DefineButton(j*BLOCK_SIZE,i*BLOCK_SIZE,BLOCK_SIZE-1,BLOCK_SIZE-1, current_id+100+BT_HIDE,0);
				PutImage(blocks_matrix[current_id]*1323+#img,21,21,j*BLOCK_SIZE,i*BLOCK_SIZE);
			}
		}
}


void DrawFlatButton(dword x,y,width,height,id,text)
{
	DrawRegion_3D(x,y,width,height,sc.work_graph,sc.work_graph);
	DrawRegion_3D(x+1,y+1,width-2,height-2,0xFFFFFF,sc.work);
	DrawBar(x+2,y+2,width-3,height-3,sc.work);
	IF (id)	DefineButton(x,y,width,height,id+BT_HIDE,sc.work);
	WriteText(-strlen(text)*6+width/2+x+1,height/2-3+y,0x80,sc.work_text,text,0);
}

stop: