// SPDX-License-Identifier: GPL-2.0-only
// Flood-it! - Strategy game: Flood the board with one color, within a step limit.
// Copyright (C) 2011-2025  Leency <lipatov.kiril@gmail.com>

#include "lib\kolibri.h" 
#include "lib\random.h"

system_colors sc;
proc_info Form;
dword help_window_stak[100];

#define DEFAULT_BLOCK_COUNT 14
#define DEFAULT_MAX_CLICKS 25
#define MAX_BLOCK_SIZE 28
char board_size = -1;
int BLOCK_SIZE; //cell size
int BLOCKS_NUM; //number of cells by X and Y
int MAX_CLICKS; //max clicks for win
int CLICKS;     //how many clicks user already did
int game_end;

#define USER_PANEL_WIDTH 144

//six colors are used in a game for a cells
//and seventh color is used to mark a cell during filling process
dword FIELD_COLORS[] = {0xf18db6, 0x605ca8, 0xfddc80, 0xdc4a20, 0x46b1e2, 0x7e9d1e, 0x232323};
char BOARD_SIZES[] = "S\0L";


#ifdef LANG_RUS
	char *BUTTON_CAPTIONS[]={ " ‡ ­®¢® [F2]", " ®¬®éì [F1]", " ‚ëå®¤ [Esc]", 0}; 
	char CLICKS_TEXT[]="Š«¨ª®¢:   /";
	char LEVELS_TEXT[]=" ®«¥:";
	
	char HELP_WINDOW_CAPTION[]="®¬®éì";
	char *HELP_TEXT[]={	"Š ª ¨£à âì ¢® Flood-it?",
	"",
	"‚ë¡¥à¨â¥ æ¢¥â, ­ ¦ ¢ ­  ®¤¨­ ¨§ ª¢ ¤à â¨ª®¢. Š«¥âª¨ ®ªà áïâáï",
	"íâ¨¬ æ¢¥â®¬ €—ˆ€Ÿ ‘ ‚…•…‰ ‹…‚Ž‰ - â ª ¢ë ¯à¨á®¥¤¨­¨â¥",
	"á®á¥¤­¨¥ ª«¥âª¨ â®© ¦¥ ®ªà áª¨. ‡ å¢ â¨âì ¯®«¥ ­ã¦­® § ",
	"®£à ­¨ç¥­­®¥ ç¨á«® å®¤®¢. „®áâã¯­® ¤¢  à §¬¥à  ¤®áª¨.",
	"",
	"ˆ£à âì â ª¦¥ ¬®¦­® ª« ¢¨è ¬¨:",
	"[Q] [W] [E]",
	"[A] [S] [D]",
	0}; 
#elif LANG_EST
	char *BUTTON_CAPTIONS[]={ "Uus mäng [F2]", "Abi      [F1]", "Välju   [Esc]", 0}; 
	char CLICKS_TEXT[]="Klikki:   /";
	char LEVELS_TEXT[]="Väli:";
	
	char HELP_WINDOW_CAPTION[]="Help";
	char *HELP_TEXT[]={	"Kuidas mängida mängu Flood-it?",
	"",
	"Ujuta kogu mänguväli üle ühe värviga lubatud käikude arvuga.",
	"Mängu alustad ülemisest vasakust nurgast ja edened valides ühe värvi",
	"vajutades nuppudele vasakul. Kui sa muudad värvi pragusel alal,",
	"siis iga kokkupuutuv sama värv muutub samaks. Nii saad ujutada",
	"teised alad mänguväljal üle. Valida saad 2 mänguvälja suuruse",
	"vahel.",
	"",
	"Mängida saab ka klaviatuuriga:",
	"[Q] [W] [E]",
	"[A] [S] [D]",
	0}; 
#else
	char *BUTTON_CAPTIONS[]={ "Restart [F2]", " Help  [F1]", " Exit  [Esc]", 0}; 
	char CLICKS_TEXT[]="Clicks:   /";
	char LEVELS_TEXT[]="Board:";
	
	char HELP_WINDOW_CAPTION[]="Help";
	char *HELP_TEXT[]={	"How to play Flood-it?",
	"",
	"You start from the TOP LEFT corner and progress by selecting one",
	"of the colored buttons on the left. When you change your current area",
	"color, every adjacent square with the same color also changes, that",
	"way you can flood other areas of the board. Select from 2 sizes of",
	"the board and try to flood-it in the least amount of steps!",
	"",
	"You can also play with keyboard:",
	"[Q] [W] [E]",
	"[A] [S] [D]",
	0}; 
#endif


unsigned char color_matrix[28*28]; //our field

unsigned char loss_matrix[14*14]={
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

unsigned char win_matrix[14*14]={
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
	
	set_board_size(0); //small board by default
	new_game();
   
	loop() switch(WaitEvent()) 
	{
		case evButton:
			id = GetButtonID(); 
			IF (id==1) || (id==4) ExitProcess();
			IF (id==2) goto _NEW_GAME_MARK;
			IF (id==3) goto _HELP_MARK;
			IF (id>=100) {
				make_turn(id-100);
			}
			if (id==10) set_board_size(0);
			if (id==11) set_board_size(1);
			break;
		case evKey:
			key = GetKeyScancode();
			IF (key==01) //Escape
				 ExitProcess();
			IF (key==59) //F1
			{
				_HELP_MARK:
					CreateThread(#help_thread,#help_window_stak);
					break;
			}
			IF (key==60) //F2
			{
				_NEW_GAME_MARK:
					new_game();
					draw_clicks_num();
					draw_field();
					break;
			}
			IF (key==16) make_turn(0); //Q
			IF (key==17) make_turn(1); //W
			IF (key==18) make_turn(2); //E
			IF (key==30) make_turn(3); //A
			IF (key==31) make_turn(4); //S
			IF (key==32) make_turn(5); //D
			break;
		case evReDraw:
			draw_window();
	}
}

void set_board_size(char s)
{
	if (board_size != s) {
		board_size = s;
		
		BLOCKS_NUM = board_size + 1 * DEFAULT_BLOCK_COUNT;
		MAX_CLICKS = board_size + 1 * DEFAULT_MAX_CLICKS;
		
		BLOCK_SIZE = GetScreenHeight() - 70 / BLOCKS_NUM; 
		if (BLOCK_SIZE > MAX_BLOCK_SIZE) BLOCK_SIZE = MAX_BLOCK_SIZE;
		
		new_game();
		
		MoveSize(-1, -1, BLOCK_SIZE*BLOCKS_NUM +14+USER_PANEL_WIDTH, 
		BLOCK_SIZE*BLOCKS_NUM +GetSkinHeight()+14);		
	}
}


void make_turn(int turn_id)
{
	IF (color_matrix[0]==turn_id) return; //ignore no-sence click: first item color is equal to a new color
	IF (!game_is_ended()) {
		CLICKS++;
		draw_clicks_num();
		fill_field(turn_id);
		if (!game_is_ended()) draw_field();
	}
}

void draw_window()
{
	int i;
	#define BUTTON_SIZE 28
	
	sc.get();
		
	DefineAndDrawWindow(300,176, BLOCK_SIZE*BLOCKS_NUM +14+USER_PANEL_WIDTH, 
	BLOCK_SIZE*BLOCKS_NUM +GetSkinHeight()+14, 0x74,0,"Flood-it!"); 
	
	// Fix rolled-up bug
	GetProcessInfo(#Form, SelfInfo);
	IF (Form.status_window==4) return;
	
	// Fill background to reduce window redraw
	for (i=0;i<=4;i++)
	{
		ESI = sc.work; 
		IF (i==4) ESI = sc.work_graph;
		DrawRegion(USER_PANEL_WIDTH+i-5,i, BLOCK_SIZE*BLOCKS_NUM +9-i-i, ESI);
	}
	DrawBar(0,0, USER_PANEL_WIDTH-5, BLOCK_SIZE*BLOCKS_NUM+10, sc.work);

	// Main buttons to fill the board
	#define FILL_BUTTON_SIZE BUTTON_SIZE+8
	for (i=0;i<6;i++) {
		DefineButton(i%3*FILL_BUTTON_SIZE+17,calc(i/3)*FILL_BUTTON_SIZE+15,
			FILL_BUTTON_SIZE,FILL_BUTTON_SIZE, i+100,FIELD_COLORS[i]);
	}

	// Menu buttons
	for (i=0;i<3;i++)
	{
		DefineButton(17,i*31+140, 13*8+6, 25, i+2,sc.work_button);
		WriteText(17+4,i*31+146,0x90,sc.work_button_text,BUTTON_CAPTIONS[i],0);
	}

	// Board size
	WriteText(17,BLOCKS_NUM*BLOCK_SIZE-25+7,0x90,sc.work_text,#LEVELS_TEXT,0);
	for (i=0;i<2;i++)
	{
		IF (board_size == i) {
			ESI=sc.work_button;
			EDI=sc.work_button_text;
		} ELSE {
			ESI = sc.work;
			EDI = sc.work_text;
		}

		DefineButton(i*32+69,BLOCKS_NUM*BLOCK_SIZE-24, 26,25, i+10,ESI);		
		WriteText(i*32+69+9,BLOCKS_NUM*BLOCK_SIZE-24+6,0x90,EDI,#BOARD_SIZES+i+i,0);
		$add ebx, 1<<16 //bold
		$int 0x40
	}
	
	draw_clicks_num();	
	draw_field();
}

void randomly_fill_the_board()
{
	int i;
	for (i=0;i<BLOCKS_NUM*BLOCKS_NUM;i++) {
		color_matrix[i] = random(6);
	}		
}

void new_game()
{
	CLICKS = 0;
	game_end = false;
	randomly_fill_the_board();	
}

void fill_field(int new_color_id)
{
	int i, j,
	old_color_id=color_matrix[0],
	restart;
	int cur_cell;
	#define MARKED 6
	
	color_matrix[0]=MARKED;
	
	_RESTART_MARK:
	
	restart=0;
	
	for (i=0;i<BLOCKS_NUM;i++)
		for (j=0;j<BLOCKS_NUM;j++)
		{
			cur_cell = i*BLOCKS_NUM+j;
			IF (color_matrix[cur_cell]<>old_color_id) continue; //if not a needed color then continue
			IF (color_matrix[cur_cell]==MARKED) continue; //if already marked then continue
			
			IF (j>0) && (color_matrix[i*BLOCKS_NUM+j-1]==MARKED) color_matrix[cur_cell]=MARKED; //left
			IF (i>0) && (color_matrix[i-1*BLOCKS_NUM+j]==MARKED) color_matrix[cur_cell]=MARKED; //top
			IF (j<BLOCKS_NUM-1) && (color_matrix[i*BLOCKS_NUM+j+1]==MARKED) color_matrix[cur_cell]=MARKED; //right
			IF (i<BLOCKS_NUM-1) && (color_matrix[i+1*BLOCKS_NUM+j]==MARKED) color_matrix[cur_cell]=MARKED; //bottom
			
			IF (color_matrix[cur_cell]==MARKED) restart=1;
		}
	IF (restart) goto _RESTART_MARK;

	for (i=0;i<BLOCKS_NUM*BLOCKS_NUM;i++) 
			IF (color_matrix[i]==MARKED) color_matrix[i]=new_color_id;
}

void draw_win_or_loose_animation(dword matrix)
{
	int i, j, ii, jj;
	for (i=0;i<14;i++) {
		for (j=0;j<14;j++)
		{
			ii = board_size * 2 + i;
			jj = board_size * 2 + j;
			color_matrix[ii*BLOCKS_NUM+jj]=
			color_matrix[ii+1*BLOCKS_NUM+jj]=
			color_matrix[ii*BLOCKS_NUM+jj+1]=
			color_matrix[ii+1*BLOCKS_NUM+jj+1]=ESBYTE[i*14+j+matrix];
			draw_field();			
		}
	}
}

int field_is_solid()
{
	int i;
	if (game_end) return 1;
	game_end = 1;
	for (i=0;i<BLOCKS_NUM*BLOCKS_NUM;i++) {
		IF (color_matrix[i]<>color_matrix[0]) game_end = 0;
	}
	return game_end;
}

int game_is_ended()
{
	int i;
	
	if (game_end) return 1;

	if (CLICKS>=MAX_CLICKS) //check for game end via max_clicks
	{
		IF (CLICKS==MAX_CLICKS) //probably user won on the last step
		{
			if (field_is_solid()) goto _WIN_MARK;
		}
		draw_win_or_loose_animation(#loss_matrix);			
		return 1;
	} else {
		if (!field_is_solid()) return 0;
		
		//field is solid and CLICKS<MAX_CLICKS -> win
		
		_WIN_MARK:
		
		for (i=0;i<25;i++)
		{
			randomly_fill_the_board();
			draw_field();
			Pause(7);
		}
		
		draw_win_or_loose_animation(#win_matrix);	
		return 1;
	}
}


void draw_clicks_num()
{
	#define TEXT_X 19
	#define TEXT_Y 100
	
	DrawBar(TEXT_X, TEXT_Y, USER_PANEL_WIDTH-TEXT_X-5,16, sc.work);
	
	WriteText(TEXT_X,TEXT_Y,0x90,sc.work_text,#CLICKS_TEXT,0);

	IF (CLICKS<10) EBX=9*8+TEXT_X;
	          else EBX=8*8+TEXT_X;
	
	WriteText(EBX,TEXT_Y,0x90,sc.work_text,itoa_nosign(CLICKS),0);
	
	WriteText(11*8+TEXT_X,TEXT_Y,0x90,sc.work_text,itoa_nosign(MAX_CLICKS),0);
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


void help_thread()
{  
	int i;
	
	loop() switch (WaitEvent())
	{
		case evButton: 
				ExitProcess();
		case evKey:
				IF (GetKeyScancode()==001) ExitProcess(); //Esc
				break;
		case evReDraw:
				//for (i=0; HELP_TEXT[i]<>0; i++;) {}; //calculate line numbers, predefined i=12 used to reduce size
				DefineAndDrawWindow(400,200,612,12*19+25+GetSkinHeight(),0x34,sc.work,#HELP_WINDOW_CAPTION);
				WriteText(6,12,0x90,sc.work_text,HELP_TEXT[0],0); //for a bold text
				for (i=0; HELP_TEXT[i]<>0; i++;) WriteText(7,i*19+12,0x90,sc.work_text,HELP_TEXT[i],0);
	}
}


stop: