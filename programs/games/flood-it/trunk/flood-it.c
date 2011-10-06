//Leency 06.10.2011, Flood-it! v2.0, GPL

#include "lib\kolibri.h" 
#include "lib\random.h"

#ifndef AUTOBUILD
#include "lang.h--"
#endif

system_colors sc;
proc_info Form;
dword stak[100]; //окно помощи

//уровни сложности
int DIFFICULTY_LEV_PARAMS[]={ 28, 14, 25,  //легко
                              17, 28, 50}; //т€жело
							  
//по-умолчанию "легко"
int BLOCK_SIZE = 28; //размер квадратика
int BLOCKS_NUM = 14; //количество квадратиков по ’ и по Y
int MAX_CLICKS = 25; //максимальное количество кликов до выигрыша

int CLICKS = 0;      //сколько ходов уже сделал игрок

#define USER_PANEL_WIDTH 119

//шесть цветов используетс€ в игре дл€ квадратиков, седьмой же (последний) дл€ того,
//чтобы отметить квадратики в процессе заливки
dword FIELD_COLORS[]= {0xf18db6, 0x605ca8, 0xfddc80, 0xdc4a20, 0x46b1e2, 0x7e9d1e,   0x232323, 0};
char *BOARD_SIZES[]={ "S", "L", 0 };


#ifdef LANG_RUS
	char *BUTTON_CAPTIONS[]={ " НЃҐ†п  [F2]", " ПЃђЃйм [F1]", " ВлеЃ§ [Esc]", 0}; 
	char CLICKS_TEXT[]=" КЂ®™®:   /";
	char LEVELS_TEXT[]="ПЃЂ•:";
	
	char HELP_WINDOW_CAPTION[]="ПЃђЃйм";
	char *HELP_TEXT[]={	"К†™ ®£а†вм ҐЃ Flood-it?",
	"",
	"З†ѓЃЂ≠®в• ѓЃЂ• Ѓ§≠®ђ жҐ•вЃђ І† Ѓ£а†≠®з•≠≠Ѓ• з®бЂЃ еЃ§ЃҐ.",
	"И£а† ≠†з®≠†•вбп б Ґ•ае≠•© Ђ•ҐЃ© ™Ђ•в™®. Вл°•а®в• жҐ•в, ≠†¶†Ґ ≠† Ѓ§®≠ ®І",
	"™Ґ†§а†в®™ЃҐ бЂ•Ґ†, ® ™Ђ•в™® Ѓ™а†бпвбп нв®ђ жҐ•вЃђ - в†™ Ґл ѓа®бЃ•§®≠®в•",
	"бЃб•§≠®• ™Ђ•в™® вЃ© ¶• Ѓ™а†б™®. З†еҐ†в®вм ѓЃЂ• ≠г¶≠Ѓ І† ђ®≠®ђ†Ђм≠Ѓ•",
	"з®бЂЃ еЃ§ЃҐ. ПЃи†£ЃҐ†п бва†в•£®п б ®≠в•а•б≠лђ ѓа®≠ж®ѓЃђ - ®Іђ•≠®бм,",
	"звЃ°л ѓЃ°•§®вм!", 0}; 
#else
	char *BUTTON_CAPTIONS[]={ "New Game [F2]", "Help     [F1]", "Exit    [Esc]", 0}; 
	char CLICKS_TEXT[]="Clicks:   /";
	char LEVELS_TEXT[]="Board:";
	
	char HELP_WINDOW_CAPTION[]="Help";
	char *HELP_TEXT[]={	"How to play Flood-it?",
	"",
	"Flood the whole board with one color within the allowed steps.",
	"You start from the top left corner and progress by selecting one",
	"of the colored balls on the left. When you change your current area",
	"color, every adjacent square with the same color also changes, that",
	"way you can flood other areas of the board. Select from 3 sizes of",
	"the board and try to flood-it in the least amount of steps!",
	"Addictive and Fun!", 0};
#endif


int color_matrix[28*28]; //цвета дл€ пол€ с квадратиками

int loose_matrix[14*14]={
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 3, 3, 3, 2, 3, 3, 3, 2, 3, 2, 3, 2, 2,
2, 3, 2, 2, 2, 3, 2, 3, 2, 2, 2, 3, 2, 2,
2, 3, 2, 2, 2, 3, 2, 3, 2, 3, 2, 3, 2, 2,
2, 3, 2, 2, 2, 3, 2, 3, 2, 3, 2, 3, 2, 2,
2, 3, 2, 2, 2, 3, 2, 3, 2, 3, 2, 3, 2, 2,
2, 3, 3, 3, 2, 3, 3, 3, 2, 3, 2, 3, 2, 2,
2, 3, 2, 2, 2, 3, 2, 3, 2, 3, 2, 3, 2, 2,
2, 3, 2, 2, 2, 3, 2, 3, 2, 3, 2, 3, 2, 2,
2, 3, 2, 2, 2, 3, 2, 3, 2, 3, 2, 3, 2, 2,
2, 3, 2, 2, 2, 3, 2, 3, 2, 3, 2, 3, 3, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

int win_matrix[14*14]={
4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 4, 1, 4,
4, 1, 4, 1, 4, 1, 4, 4, 4, 1, 4, 4, 1, 4,
4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 1, 4, 1, 4,
4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 1, 4,
4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 4, 1, 4,
4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 4, 1, 4,
4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 4, 1, 4,
4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 4, 1, 4,
4, 1, 4, 1, 4, 1, 4, 1, 4, 1, 4, 4, 1, 4,
4, 4, 1, 4, 1, 4, 4, 1, 4, 1, 4, 4, 1, 4,
4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
};

void main()
{   
	int key, id;
	
	new_game();
   
	loop()
	{
		switch(WaitEvent()) 
		{
			case evButton:
				id = GetButtonID(); 
				if (id==1) || (id==4) ExitProcess();
				if (id==2) goto _NEW_GAME_MARK;
				if (id==3) goto _HELP_MARK;
				if (id>=100)
				{
					if (color_matrix[0]==id-100) break; //если цвет первой фишки такой же, игнорируем бессмысленный ход
					if (CLICKS>=MAX_CLICKS) break; //если игра закончена
					
					CLICKS++;
					draw_clicks_num();
					
					fill_field(id-100);
					draw_field();
					check_for_end(); //если игра закончена
					break;
				}
				if (id>=10)
				{
					id=id-10*3;
					
					if (BLOCK_SIZE == DIFFICULTY_LEV_PARAMS[id]) break; //выбран тот же размер
					
					BLOCK_SIZE = DIFFICULTY_LEV_PARAMS[id]; //размер квадратика
					BLOCKS_NUM = DIFFICULTY_LEV_PARAMS[id+1]; //количество квадратиков по ’ и по Y
					MAX_CLICKS = DIFFICULTY_LEV_PARAMS[id+2]; //максимальное количество кликов до выигрыша
					
					new_game();
					
					MoveSize(-1, -1, BLOCK_SIZE*BLOCKS_NUM +14+USER_PANEL_WIDTH, BLOCK_SIZE*BLOCKS_NUM +GetSkinWidth()+14);
					break;
				}
				break;
			case evKey:
				key = GetKey();
				if (key==027) //Escape
					 ExitProcess();
				if (key==050) //F1
				{
					_HELP_MARK:
						CreateThread(#help,#stak); 
				}
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
}


void draw_window()
{
	int i, j;
	#define BUTTON_SIZE 28
	
	sc.get();
		
	DefineAndDrawWindow(300,176, BLOCK_SIZE*BLOCKS_NUM +14+USER_PANEL_WIDTH, BLOCK_SIZE*BLOCKS_NUM +GetSkinWidth()+14, 0x74,sc.work,0,0,"Flood-it!"); 
	
	//провер€ем не схлопнуто ли окно в заголовок
	GetProcessInfo(#Form, SelfInfo);
	if (Form.height==GetSkinWidth()+3) return;
	
	//закрашиваем фон -> уменьшает перерисовку
	for (i=0;i<=4;i++)
	{
		if (i<>4)
			DrawRegion(USER_PANEL_WIDTH+i-5,i, BLOCK_SIZE*BLOCKS_NUM +9-i-i, BLOCK_SIZE*BLOCKS_NUM+9-i-i, sc.work);
		else
			DrawRegion(USER_PANEL_WIDTH+i-5,i, BLOCK_SIZE*BLOCKS_NUM +9-i-i, BLOCK_SIZE*BLOCKS_NUM+9-i-i, sc.work_graph); //ободок
	}
	DrawBar(0,0, USER_PANEL_WIDTH-5, BLOCK_SIZE*BLOCKS_NUM+10, sc.work);
	
	//кнопочки заливки
	for (i=0;i<2;i++)
		for (j=0;j<3;j++)
			DefineButton(j*BUTTON_SIZE+17,i*BUTTON_SIZE+15,BUTTON_SIZE,BUTTON_SIZE, i*3+j+100,FIELD_COLORS[i*3+j]);

	//кнопочки действий
	for (j=0;j<3;j++)
	{
		DefineButton(17,j*25+120, 13*6+6, 20, j+2,sc.work_button);
		WriteText(17+4,j*25+127,0x80,sc.work_button_text,BUTTON_CAPTIONS[j],0);
	}

	//кнопочки выбора уровн€ сложности
	WriteText(17,BLOCKS_NUM*BLOCK_SIZE-20+7,0x80,sc.work_text,#LEVELS_TEXT,0);
	for (j=0;j<2;j++)
	{
		DefineButton(j*25+56,BLOCKS_NUM*BLOCK_SIZE-20, 20,20, j+10,sc.work_button);
		
		if (BLOCK_SIZE == DIFFICULTY_LEV_PARAMS[j*3]) EDI=0x800080;
			else EDI=sc.work_button_text;
		
		WriteText(j*25+56+8,BLOCKS_NUM*BLOCK_SIZE-20+7,0x80,EDI,BOARD_SIZES[j],0);
		WriteText(j*25+56+9,BLOCKS_NUM*BLOCK_SIZE-20+7,0x80,EDI,BOARD_SIZES[j],0);
	}
	
	draw_clicks_num();
	
	draw_field();

}


void new_game()
{
	int i, j;
	
	//randomize();

	CLICKS = 0;
	
	for (i=0;i<BLOCKS_NUM;i++)
		for (j=0;j<BLOCKS_NUM;j++)
			color_matrix[i*BLOCKS_NUM+j] = random(6);
}


void fill_field(int new_color_id)
{
	int i=0, j=0,
	old_color_id=color_matrix[0],
	restart;
	#define MARKED 6
	
	color_matrix[0]=MARKED;
	
	_RESTART_MARK:
	
	restart=0;
	
	for (i=0;i<BLOCKS_NUM;i++)
		for (j=0;j<BLOCKS_NUM;j++)
		{
			if (color_matrix[i*BLOCKS_NUM+j]<>old_color_id) continue; //если фишка не нужного цвета идЄм дальше
			if (color_matrix[i*BLOCKS_NUM+j]==MARKED) continue; //если фишка уже отмечена, идЄм далее
			
			if (j>0) && (color_matrix[i*BLOCKS_NUM+j-1]==MARKED) color_matrix[i*BLOCKS_NUM+j]=MARKED; //смотрим левый
			if (i>0) && (color_matrix[i-1*BLOCKS_NUM+j]==MARKED) color_matrix[i*BLOCKS_NUM+j]=MARKED; //смотрим верхний
			if (j<BLOCKS_NUM-1) && (color_matrix[i*BLOCKS_NUM+j+1]==MARKED) color_matrix[i*BLOCKS_NUM+j]=MARKED; //смотрим правый
			if (i<BLOCKS_NUM-1) && (color_matrix[i+1*BLOCKS_NUM+j]==MARKED) color_matrix[i*BLOCKS_NUM+j]=MARKED; //смотрим нижний
			
			if (color_matrix[i*BLOCKS_NUM+j]==MARKED) restart=1; //если фишку отметили, то потом цикл нужно будет прокрутить сначала - мож ещЄ чЄ отметим
		}
	if (restart) goto _RESTART_MARK;

	for (i=0;i<BLOCKS_NUM;i++)
		for (j=0;j<BLOCKS_NUM;j++)
			if (color_matrix[i*BLOCKS_NUM+j]==MARKED)	color_matrix[i*BLOCKS_NUM+j]=new_color_id;
}


int check_for_end()
{
	int i, j, ii, jj;

	if (CLICKS>=MAX_CLICKS) //если проигрыш
	{
		if (CLICKS==MAX_CLICKS) //выигрышь на последнем ходе
		{
			for (i=0;i<BLOCKS_NUM;i++) //провер€ем всЄ ли поле одного цвета, если нет уходим
				for (j=0;j<BLOCKS_NUM;j++)
					if (color_matrix[i*BLOCKS_NUM+j]<>color_matrix[0]) goto _LOOSE_MARK;
			goto _WIN_MARK;
		}
		
		_LOOSE_MARK:
		
		for (i=0;i<14;i++)
			for (j=0;j<14;j++)
			{
				if (BLOCK_SIZE == DIFFICULTY_LEV_PARAMS[0])
				{
					ii=i;
					jj=j;
				}
				else
				{
					ii=i*2;
					jj=j*2;
				}
				color_matrix[ii*BLOCKS_NUM+jj]=loose_matrix[i*14+j];
				color_matrix[ii+1*BLOCKS_NUM+jj]=loose_matrix[i*14+j];
				color_matrix[ii*BLOCKS_NUM+jj+1]=loose_matrix[i*14+j];
				color_matrix[ii+1*BLOCKS_NUM+jj+1]=loose_matrix[i*14+j];
				draw_field();
				//Pause(5);				
			}	
			
		return 1;
	}
	
	for (i=0;i<BLOCKS_NUM;i++) //провер€ем всЄ ли поле одного цвета, если нет уходим
		for (j=0;j<BLOCKS_NUM;j++)
			if (color_matrix[i*BLOCKS_NUM+j]<>color_matrix[0]) return 0;

	//всЄ поле одного цвета и фишек меньше MAX_CLICKS -> победа
	
	_WIN_MARK:
	
	for (i=0;i<25;i++)
	{
		new_game();
		draw_field();
		Pause(7);
	}

	CLICKS=MAX_CLICKS;
	
	for (i=0;i<14;i++)
		for (j=0;j<14;j++)
		{
			if (BLOCK_SIZE == DIFFICULTY_LEV_PARAMS[0]) //заливка дл€ полей разного размера разна€
			{
				ii=i;
				jj=j;
			}
			else
			{
				ii=i*2;
				jj=j*2;
			}
			color_matrix[ii*BLOCKS_NUM+jj]=win_matrix[i*14+j];
			color_matrix[ii+1*BLOCKS_NUM+jj]=win_matrix[i*14+j];
			color_matrix[ii*BLOCKS_NUM+jj+1]=win_matrix[i*14+j];
			color_matrix[ii+1*BLOCKS_NUM+jj+1]=win_matrix[i*14+j];
			draw_field();
			//Pause(5);				
		}	
	return 1;
}


void draw_clicks_num()
{
	#define TEXT_X 21
	#define TEXT_Y 92
	
	DrawBar(8*6+TEXT_X, TEXT_Y, 6*2,9, sc.work);
	
	WriteText(TEXT_X,TEXT_Y,0x80,sc.work_text,#CLICKS_TEXT,0);

	if (CLICKS<10) EBX=9*6+TEXT_X;
		else EBX=8*6+TEXT_X;
	
	WriteText(EBX,TEXT_Y,0x80,sc.work_text,IntToStr(CLICKS),0);
	
	WriteText(11*6+TEXT_X,TEXT_Y,0x80,sc.work_text,IntToStr(MAX_CLICKS),0);
}


void draw_field()
{
	int i, j;
	int color_id;
	
	for (i=0;i<BLOCKS_NUM;i++)
		for (j=0;j<BLOCKS_NUM;j++)
		{
			color_id = color_matrix[i*BLOCKS_NUM+j];
			DrawBar(j*BLOCK_SIZE+USER_PANEL_WIDTH, i*BLOCK_SIZE+5, BLOCK_SIZE,BLOCK_SIZE, FIELD_COLORS[color_id]);
		}
}


void help()
{  
	int i;
	
	loop()
	switch (WaitEvent())
	{
		CASE evButton: 
				if (GetButtonID()==1) ExitProcess();
				break;
		CASE evKey:
				if (GetKey()==27) ExitProcess(); //Esc
				break;
		CASE evReDraw:
				for (i=0; HELP_TEXT[i]<>0; i++;) {};
				
				DefineAndDrawWindow(500,200,450,i*13+50,0x34,sc.work,0,0,#HELP_WINDOW_CAPTION);
				
				WriteText(6,12,0x80,sc.work_text,HELP_TEXT[0],0); //это дл€ жирного шрифта
				for (i=0; HELP_TEXT[i]<>0; i++;) WriteText(5,i*13+12,0x80,sc.work_text,HELP_TEXT[i],0);
	}
}


stop: