/*	------- KOSILKA FOR KOLIBI -------
Written in C++ (MS Visual C++ 6.0+).
Version 1.16.
  
					 Andrey Mihaylovich aka Dron2004
*/


#include "kosSyst.h"
#include "kosFile.h"
#include "images.cpp"


//External levels
bool external_levels_available=false;
bool external_levels = false;
int level_read_result;
Byte * externallevels;
Byte external_levels_count[1] = {0};
/////////////////

int lastkosilkadirection=1;
int laststep=0; //Last step. 0-does not exists, 1-bottom, 2-up, 3-left, 4-right


Byte skindata[13824];
int read_result=0;
bool skin_available=false;
bool w_redraw=true;

#define RUS 1
#define ENG 2
#ifndef LANG
#define LANG RUS
#endif

#if LANG == RUS
	const char windowTitle[] = "Косилка для Колибри";
	const char version[]     = "Версия 1.16";
	const char win[]         = "Вы выиграли!";
	const char press_q[]     = "<нажмите <q> для возврата в меню>";
	const char fail[]        = "Игра окончена...";
	const char press_r[]     = "<нажмите <r> для того, чтобы сыграть ещё раз>";
	const char k_o_s_i_l_k[] = "К   О   С   И   Л   К   А";
	const char k_o_s_i_l_2[] = "К  О  С  И  Л  К  А";
	const char for_kolibri[] = "для Колибри ОС";
	const char press_enter[] = "<для начала игры нажмите ENTER>";
	const char press_g[]     = "<нажмите <g> для переключения режима графики>";
	const char press_a[]     = "<нажмите <a> для включения/выключения анимации>";
	const char press_h[]     = "<нажмите <h> для просмотра сведений о программе>";
	const char press_s[]     = "<нажмите <s> чтобы выключить/включить скин>";
	const char level_label[] = "Уровень:";
	const char attempts_left[] = "Осталось попыток:";
	const char grass_left[]  = "Осталось травы:";

	const char level_select_1[] = "Выберите набор уровней (нажмите <1> или <2>):";
	const char level_select_2[] = "1. Встроенные уровни";
	const char level_select_3[] = "2. Внешний набор уровней";

	const char about1[] = "Коллектив разработчиков:";
	const char about2[] = "Андрей Михайлович aka Dron2004 - программирование, встроенная графика (без скина)";
	const char about3[] = "Mario79 - тестирование, помощь в разработке, важные идеи";
	const char about4[] = "Ataualpa - тестирование, помощь в разработке";
	const char about5[] = "Leency - тестирование, помощь в разработке, замечательные скины, важные идеи";
	const char about6[] = "Mike - тестирование, помощь в разработке";
	const char about7[] = "bw - тестирование, помощь в разработке, важные идеи";
	const char about8[] = "diamond - идея отмены хода, тестирование";
	const char about9[] = "Отдельное спасибо:";
	const char about10[] = "Всем, кто играет в эту игру :-) !";
	const char about11[] = "нажмите <BACKSPACE> для возврата в меню";
#else
	const char windowTitle[] = "Kosilka for Kolibri";
	const char version[]     = "  Ver. 1.16";
	const char win[]         = "  You won!  ";
	const char press_q[]     = "<press <q> to return to the menu>";
	const char fail[]        = "Game over...";
	const char press_r[]     = "<press r to try again>";
	const char k_o_s_i_l_k[] = "K   O   S   I   L   K   A";
	const char k_o_s_i_l_2[] = "K  O  S  I  L  K  A";
	const char for_kolibri[] = "for Kolibri OS";
	const char press_enter[] = "press ENTER to start the game";
	const char press_g[]     = "<press <g> to switch graphics mode>";
	const char press_a[]     = "<press <a> to turn on/off animation>";
	const char press_h[]     = "<press <h> to view information about the program>";
	const char press_s[]     = "<press <s> to turn on/off external skin>";
	const char level_label[] = "Level:";
	const char attempts_left[] = "Attempts left:";
	const char grass_left[]  = "Grass left:";

	const char level_select_1[] = "Choose level pack (press <1> or <2>):";
	const char level_select_2[] = "1. Built-in levels";
	const char level_select_3[] = "2. External levels";

	const char about1[] = "Our team:";
	const char about2[] = "Andrei Mikhailovich aka Dron2004 - development, built-in graphics (excluding skin)";
	const char about5[] = "Leency - help in development and bugfixing, English translation, testing, beautiful skins";
	const char about3[] = "Mario79 - testing, help in development, ideas";
	const char about4[] = "Ataualpa - testing, help in development";
	const char about6[] = "Mike - testing, help in development";
	const char about7[] = "bw - testing, help in development, ideas";
	const char about8[] = "diamond - idea of turn back, testing";
	const char about9[] = "Special thanks to:";
	const char about10[] = "Everyone who plays this game :-) !";
	const char about11[] = "press <BACKSPACE> to go back to menu";
#endif

int levelcount=7;

char gamearea[20][20]; //The map


short int kosilkax; // Kosilka position
short int kosilkay;
short int kosilkadirection=1; //Last step direction. 1-bottom, 2-up, 3-left, 4-right
short int lives=2;
short int level=1;
short int status=0; //Where 0 - welcome screen
					// 1 - the game
					// 2 - win
					// 3 - fail
					// 4 - select the level (internal or external)
					// -1 - about
bool gamestarted=false; //Key locker. If "false" then not possible to play

bool drawgraphics=true; //Draw detailed graphics or just colored rectangles
bool drawanimation=true;
int grassLeft();  //Notify about "Grass Left" function

RGB kosilka_d[576];
RGB kosilka_l[576];
RGB kosilka_r[576];
RGB kosilka_u[576];
RGB grass[576];
RGB stone[576];
RGB tree[576];
RGB skos[576];

//Palette. Decoding to 0xRRGGBB
char * apppath;
char * levpath;


char * getLevelsPathName(){

	int lastslashindex=0;
	static char levfilename[]="koslevel.pak";
	int tempslfnd=0;

	for (tempslfnd=0; tempslfnd < strlen(kosExePath); tempslfnd++){
			if (kosExePath[tempslfnd]=='/'){lastslashindex=tempslfnd;}
	}

	levpath = new char[lastslashindex+strlen(levfilename)+1];

	for (tempslfnd=0; tempslfnd <= lastslashindex; tempslfnd++){
		levpath[tempslfnd]=kosExePath[tempslfnd];
	}
	for (tempslfnd=0; tempslfnd < strlen(levfilename); tempslfnd++){
		levpath[tempslfnd+lastslashindex+1]=levfilename[tempslfnd];
	}

	return levpath;
}


void interlevelpause(){ //The pause between levels
	Byte tempCode;
	RGB tmprgb;
	int tmpa=0;
	laststep=0;
	static int yellow_pal[] = {0xA8A93D,0xBEBF4C,0xD6D856,0xDFE15A,0xECEE5B,
						    0xECEE5B,0xDFE15A,0xD6D856,0xBEBF4C,0xA8A93D};
		for (int iic=0;iic<240;iic++){
			kos_WaitForEvent(1); //Pause()?
			kos_GetKey(tempCode);

			kos_DrawBar(iic*2,0,2,480,yellow_pal[tmpa]);

			tmpa++;
			if (tmpa>9) tmpa=0;
		}
}

void draw_element(int elx, int ely){ //Draw map element
switch (gamearea[elx][ely]){
			case 'g':
				if (drawgraphics==true){
					kos_PutImage((RGB*)grass,24,24,elx*24,ely*24);
				}
				else
				{
					kos_DrawBar(elx*24,ely*24,24,24,0xAAAA00);
				}
			
				break;
			case 'k':
				if (drawgraphics==true){
					switch(kosilkadirection){
					case 1:
						kos_PutImage((RGB*)kosilka_d,24,24,elx*24,ely*24);
						break;
					case 2:
						kos_PutImage((RGB*)kosilka_u,24,24,elx*24,ely*24);
						break;
					case 3:
						kos_PutImage((RGB*)kosilka_l,24,24,elx*24,ely*24);
						break;
					case 4:
						kos_PutImage((RGB*)kosilka_r,24,24,elx*24,ely*24);
						break;
					}
				}
				else
				{
					kos_DrawBar(elx*24,ely*24,24,24,0x00AAAA);
				}


				break;
			case 'n':
				if (drawgraphics==true){
					kos_PutImage((RGB*)skos,24,24,elx*24,ely*24);
				}
				else
				{
					kos_DrawBar(elx*24,ely*24,24,24,0xAAAAAA);
				}


				break;
			case 's':
				if (drawgraphics==true){
					kos_PutImage((RGB*)stone,24,24,elx*24,ely*24);
				}
				else
				{
					kos_DrawBar(elx*24,ely*24,24,24,0x555555);
				}


				break;

			case 't':
				if (drawgraphics==true){
					kos_PutImage((RGB*)tree,24,24,elx*24,ely*24);
				}
				else
				{
					kos_DrawBar(elx*24,ely*24,24,24,0x005500);
				}


				break;
			}

}


void display_grass_left(){
	kos_DrawBar(605,120,20,10,0xEEEEEE);
	kos_DisplayNumberToWindow(grassLeft(),3,605,120,0x0000FF,nbDecimal,false);
}


void animate(int initcellx, int initcelly, int direction){ //kosilka movement animation
	int tmpp=0;
	
	switch (direction){
		case 1:
			for (tmpp=0; tmpp<23;tmpp++){
				if (drawgraphics==true){
					kos_PutImage((RGB*)skos,24,24,initcellx*24,initcelly*24);
					kos_PutImage((RGB*)kosilka_d,24,24,initcellx*24,initcelly*24+tmpp);
				} else {
					kos_DrawBar(initcellx*24,initcelly*24,24,24,0xAAAAAA);
					kos_DrawBar(initcellx*24,initcelly*24+tmpp,24,24,0x00AAAA);

				}
				kos_Pause(1);
			}
			break;
		case 2:
			for (tmpp=0; tmpp<23;tmpp++){
				if (drawgraphics==true){
					kos_PutImage((RGB*)skos,24,24,initcellx*24,initcelly*24);
					kos_PutImage((RGB*)kosilka_u,24,24,initcellx*24,initcelly*24-tmpp);
				} else {
					kos_DrawBar(initcellx*24,initcelly*24,24,24,0xAAAAAA);
					kos_DrawBar(initcellx*24,initcelly*24-tmpp,24,24,0x00AAAA);
				}
				kos_Pause(1);
			}
			break;
		case 3:
			for (tmpp=0; tmpp<23;tmpp++){
				if (drawgraphics==true){
					kos_PutImage((RGB*)skos,24,24,initcellx*24,initcelly*24);
					kos_PutImage((RGB*)kosilka_r,24,24,initcellx*24+tmpp,initcelly*24);
				} else {
					kos_DrawBar(initcellx*24,initcelly*24,24,24,0xAAAAAA);
					kos_DrawBar(initcellx*24+tmpp,initcelly*24,24,24,0x00AAAA);

				}
				kos_Pause(1);
			}
			break;
		case 4:
			for (tmpp=0; tmpp<23;tmpp++){
				if (drawgraphics==true){
					kos_PutImage((RGB*)skos,24,24,initcellx*24,initcelly*24);
					kos_PutImage((RGB*)kosilka_l,24,24,initcellx*24-tmpp,initcelly*24);
				} else {
					kos_DrawBar(initcellx*24,initcelly*24,24,24,0xAAAAAA);
					kos_DrawBar(initcellx*24-tmpp,initcelly*24,24,24,0x00AAAA);

				}
				kos_Pause(1);
			}
			break;
	}
}


void draw_window(void){
	sProcessInfo sPI;
	
	if (w_redraw)
	{
		kos_WindowRedrawStatus(1);
		kos_DefineAndDrawWindow(50,50,640,506-22+kos_GetSkinHeight(),0x74,0xEEEEEE,0,0,(Dword)windowTitle);
		kos_WindowRedrawStatus(2);
	}
	w_redraw=false;

	kos_ProcessInfo( &sPI );
	if (sPI.rawData[70]&0x04) return; //do nothing if window is rolled-up into title

	//Map redraw
	if ((status!=0)&&(status!=-1)&&(status!=4))
	{
		kos_DrawBar(631-151,0,151,480,0xEEEEEE);

		kos_WriteTextToWindow(500,30,0x80, 0, (char*)k_o_s_i_l_2,19);
		kos_WriteTextToWindow(517,40,0x80, 0, (char*)for_kolibri,14);

		kos_WriteTextToWindow(495,80,0x80, 0, (char*)level_label,6);
		kos_DisplayNumberToWindow(level,3,605,80,0x0000FF,nbDecimal,false);

		kos_WriteTextToWindow(495,95,0x80, 0, (char*)attempts_left,11);
		kos_DisplayNumberToWindow(lives,1,605,95,0x0000FF,nbDecimal,false);

		kos_WriteTextToWindow(495,120,0x80, 0, (char*)grass_left,11);
		display_grass_left();

		kos_WriteTextToWindow(526,450,0x80, 0,(char*)version,12);

		for (int cy=0;cy<20;cy++) for (int cx=0;cx<20;cx++)	draw_element(cx,cy);
	}

	if (status==0){
		kos_DrawBar(0,0,4,480,0x000000);
		kos_DrawBar(628,0,3,480,0x000000);

		
		for (int tmpppy=0;tmpppy<20;tmpppy++){
			for (int tmpppx=0;tmpppx<26;tmpppx++){
				if ((tmpppx==0) || (tmpppx==25) || (tmpppy==0) || (tmpppy==19)){
					kos_PutImage((RGB*)stone,24,24,4+tmpppx*24,tmpppy*24);
				}
				else
				{
					kos_PutImage((RGB*)skos,24,24,4+tmpppx*24,tmpppy*24);
				}
			}
		}

		if (drawgraphics==true){
			kos_PutImage((RGB*)kosilka_d,24,24,305,150);
		}
		else
		{
			kos_DrawBar(305,150,24,24,0x00AAAA);
		}


		if (drawanimation==true){
			kos_DrawBar(335,150,24,24,0x00AA00);
		}
		else
		{
			kos_DrawBar(335,150,24,24,0xAA0000);
		}

                kos_WriteTextToWindow(255, 200,0x80, 0xFFFFFF, (char*)k_o_s_i_l_k, 19);
                kos_WriteTextToWindow(290, 220,0x80, 0xFFFFFF, (char*)for_kolibri, 14);
                kos_WriteTextToWindow(239, 240,0x80, 0xFFFFFF, (char*)press_enter, 30);
                kos_WriteTextToWindow(30,  380,0x80, 0xFFFFFF, (char*)press_g, 53);
                kos_WriteTextToWindow(30,  400,0x80, 0xFFFFFF, (char*)press_a, 41);
                kos_WriteTextToWindow(30,  420,0x80, 0xFFFFFF, (char*)press_h, 32);
                if (skin_available==true) kos_WriteTextToWindow(30, 440,0x80, 0xFFFFFF, (char*)press_s, 27);

        //      kos_WriteTextToWindow(470, 440,0x80, 0xFFFFFF ,"нажмите <ESC> для выхода",27);
	//	kos_DisplayNumberToWindow(external_levels_count[0],3,200,340,0x0000FF,nbDecimal,false);



		kos_WriteTextToWindow(533, 440,0x80, 0xFFFFFF ,(char*)version,0);
	}
	if (status==2){
	   kos_DrawBar(10,150,610,200,0x528B4C);	
	   kos_DrawBar(15,155,601,190,0x3BCF46);	
           kos_WriteTextToWindow(240,230,0x80, 0xFFFFFF ,(char*)win,13);
           kos_WriteTextToWindow(240,250,0x80, 0xFFFFFF ,(char*)press_q,17);
	}
	if (status==3){
	   kos_DrawBar(10,150,610,200,0x8B4C4C);	
	   kos_DrawBar(15,155,601,190,0xCF3B3B);	
           kos_WriteTextToWindow(220,220,0x80, 0xFFFFFF ,(char*)fail,13);
           kos_WriteTextToWindow(220,240,0x80, 0xFFFFFF ,(char*)press_r,23);
           kos_WriteTextToWindow(220,260,0x80, 0xFFFFFF ,(char*)press_q,17);
	}
	if (status==-1){

		kos_DrawBar(0,0,4,480,0x000000);
		kos_DrawBar(631-3,0,3,480,0x000000);

	   for (int tmpppy=0;tmpppy<20;tmpppy++){
			for (int tmpppx=0;tmpppx<26;tmpppx++){
				if ((tmpppx==0) || (tmpppx==25) || (tmpppy==0) || (tmpppy==19)){
					kos_PutImage((RGB*)stone,24,24,4+tmpppx*24,tmpppy*24);
				}
				else
				{
					kos_PutImage((RGB*)skos,24,24,4+tmpppx*24,tmpppy*24);
				}
			}
		}

		kos_WriteTextToWindow(40,40,0x80, 0xFFFFFF, (char*)windowTitle,22);
		kos_WriteTextToWindow(40,60,0x80, 0xFFFFFF, (char*)version,12);
		kos_WriteTextToWindow(40,75,0x80, 0xFFFFFF, "________________________________________",40);
	   
		kos_WriteTextToWindow(40,120,0x80, 0xFFFFFF, (char*)about1, 18);
		kos_WriteTextToWindow(40,150,0x80, 0xEEFFEE, (char*)about2, 32);
		kos_WriteTextToWindow(40,170,0x80, 0xDDFFDD, (char*)about3, 35);
		kos_WriteTextToWindow(40,190,0x80, 0xCCFFCC, (char*)about4, 36);
		kos_WriteTextToWindow(40,210,0x80, 0xBBFFBB, (char*)about5, 62);
		kos_WriteTextToWindow(40,230,0x80, 0xAAFFAA, (char*)about6, 34);
		kos_WriteTextToWindow(40,250,0x80, 0x99FF99, (char*)about7, 49);
		kos_WriteTextToWindow(40,270,0x80, 0x99FF99, (char*)about8, 49);

		kos_WriteTextToWindow(40,300,0x80, 0x88FF88, (char*)about9, 16);
		kos_WriteTextToWindow(40,330,0x80, 0x77FF77, (char*)about10, 50);

		kos_WriteTextToWindow(40,430,0x80, 0x66FF66, (char*)about11, 35);
	}


	if (status==4){
		kos_DrawBar(0,0,631,480,0x000000);	
		

		for (int tmpppy=0;tmpppy<20;tmpppy++){
			for (int tmpppx=0;tmpppx<26;tmpppx++){
				if ((tmpppx==0) || (tmpppx==25) || (tmpppy==0) || (tmpppy==19)){
					kos_PutImage((RGB*)stone,24,24,4+tmpppx*24,tmpppy*24);
				}
				else
				{
					kos_PutImage((RGB*)skos,24,24,4+tmpppx*24,tmpppy*24);
				}
			}
		}

                kos_WriteTextToWindow(215, 200,0x80, 0xFFFFFF, (char*)level_select_1,0);
                kos_WriteTextToWindow(215, 220,0x80, 0xFFFFFF, (char*)level_select_2,0);
                kos_WriteTextToWindow(215, 240,0x80, 0xFFFFFF, (char*)level_select_3,0);

	}

}



//Game level description,
//where k - kosilka
//		g - grass
//		n - cut grass
//		s - stone
//		t - tree
void initializeLevel(int levnum){
	laststep=0;
	if (external_levels==false){

	kosilkadirection=1;
	if (levnum==1){
		static char tmparea[20][20]={{'k','t','g','g','g','g','g','s','g','g','g','g','g','g','g','g','g','g','g','g'},
								{'g','s','g','s','g','g','g','s','g','s','g','g','s','g','g','g','g','g','g','g'},
								{'g','t','g','s','g','g','g','s','g','s','g','g','s','g','g','g','g','g','g','g'},
								{'g','s','g','s','g','g','g','s','g','s','g','g','s','g','g','g','g','g','g','g'},
								{'g','t','g','s','g','g','g','s','g','s','g','g','s','g','g','g','g','g','g','g'},
								{'g','s','g','s','g','g','g','s','g','g','g','g','s','g','g','g','g','g','g','g'},
								{'g','t','g','s','g','g','g','s','g','g','g','g','s','g','g','s','s','s','g','g'},
								{'g','s','g','s','g','g','g','s','g','g','g','g','s','g','g','s','s','s','g','g'},
								{'g','t','g','s','g','g','g','s','g','g','g','g','s','g','g','s','s','s','g','g'},
								{'g','s','g','s','g','g','g','s','g','g','g','g','s','g','g','s','g','g','g','g'},
								{'g','t','g','s','g','g','g','s','g','g','g','g','s','g','g','g','g','g','g','g'},
								{'g','s','g','s','g','g','g','s','g','g','g','g','s','g','g','g','g','g','g','g'},
								{'g','t','g','s','g','g','g','s','g','g','g','g','s','g','g','g','g','g','g','g'},
								{'g','s','g','s','g','g','g','s','g','g','g','g','s','g','g','g','g','g','g','g'},
								{'g','t','g','s','g','g','g','s','g','g','g','g','s','g','g','g','g','g','g','g'},
								{'g','s','g','s','g','g','g','s','g','g','g','g','s','g','g','g','g','g','g','g'},
								{'g','t','g','s','g','g','g','s','g','g','g','g','s','g','g','g','g','g','g','g'},
								{'g','s','g','s','g','g','g','s','g','g','g','g','s','g','g','g','g','g','g','g'},
								{'g','t','g','s','g','g','g','s','g','g','g','g','s','g','g','g','g','g','g','g'},
								{'g','g','g','s','g','g','g','g','g','s','g','g','s','g','g','g','g','g','g','g'}};
			for (int tyy=0;tyy<20;tyy++){
				for (int txx=0;txx<20;txx++){
				
					if (tmparea[txx][tyy]=='k'){
						kosilkax=tyy;
						kosilkay=txx;
					}

					gamearea[txx][tyy]=tmparea[tyy][txx];
				}
			}
	}
	if (levnum==2){
		static char tmparea[20][20]={{'s','s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g'},
								{'s','s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s','s','g'},
								{'g','k','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s','s','g'},
								{'g','g','g','g','g','g','g','g','g','g','g','g','s','s','g','g','g','g','g','g'},
								{'g','g','g','g','g','g','g','g','g','g','g','g','s','s','g','g','g','g','g','g'},
								{'g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g'},
								{'g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g'},
								{'g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g'},
								{'s','s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g'},
								{'s','s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g'},
								{'g','g','g','g','g','g','g','g','s','s','g','g','g','g','g','g','g','s','s','g'},
								{'g','g','g','g','g','g','g','g','s','s','g','g','g','g','g','g','g','s','s','g'},
								{'g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g'},
								{'g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g'},
								{'g','g','g','g','g','g','g','g','g','g','g','g','g','s','s','g','g','g','g','g'},
								{'g','g','g','g','g','g','g','g','g','g','g','g','g','s','s','g','g','g','g','g'},
								{'g','g','g','s','s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g'},
								{'g','g','g','s','s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g'},
								{'g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g'},
								{'g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g'}};
			for (int tyy=0;tyy<20;tyy++){
				for (int txx=0;txx<20;txx++){
				
					if (tmparea[txx][tyy]=='k'){
						kosilkax=tyy;
						kosilkay=txx;
					}

					gamearea[txx][tyy]=tmparea[tyy][txx];
				}
			}
	}

	if (levnum==3){
		static char tmparea[20][20]={{'t','n','t','n','t','n','t','n','t','n','t','n','t','n','t','n','t','n','t','k'},
								{'n','t','g','g','g','g','g','g','g','g','g','g','s','n','n','s','n','s','t','g'},
								{'t','n','g','g','g','g','g','g','g','g','g','g','n','t','n','n','n','t','t','g'},
								{'n','t','g','g','g','g','g','g','g','g','g','g','n','s','n','s','n','s','t','g'},
								{'t','n','g','g','g','g','g','s','s','g','g','g','n','n','n','n','n','t','t','g'},
								{'n','t','g','g','g','g','g','g','g','g','g','g','t','n','s','n','n','s','t','g'},
								{'t','n','g','g','g','g','g','g','g','g','g','g','n','n','n','n','n','t','t','g'},
								{'n','t','g','g','g','g','g','g','g','g','g','g','n','n','s','n','n','s','t','g'},
								{'t','n','g','g','g','g','g','g','g','g','g','g','n','s','n','n','n','t','t','g'},
								{'n','t','g','g','g','g','g','g','g','g','g','g','s','n','n','n','n','t','g','g'},
								{'t','n','g','g','g','s','g','g','g','g','g','g','t','t','t','t','t','t','g','g'},
								{'n','t','g','g','g','s','g','g','g','g','g','g','g','g','g','g','g','t','g','g'},
								{'t','n','g','g','g','g','g','g','g','g','g','g','t','t','g','g','g','t','g','g'},
								{'n','t','g','g','g','g','g','g','t','t','g','g','g','t','g','g','g','t','g','g'},
								{'t','n','g','g','g','g','g','g','t','t','g','g','g','t','g','g','g','t','g','g'},
								{'n','t','g','g','g','g','g','g','g','g','g','g','g','t','g','g','g','t','g','g'},
								{'t','n','g','g','g','g','g','g','g','g','g','g','g','t','t','g','g','t','g','g'},
								{'n','t','g','g','g','g','g','g','g','g','g','g','g','g','t','g','g','t','g','g'},
								{'t','n','g','g','g','g','g','g','g','g','g','g','g','g','t','g','g','t','g','g'},
								{'n','t','s','s','s','s','s','s','s','s','s','s','s','s','t','g','g','g','g','g'}};
			for (int tyy=0;tyy<20;tyy++){
				for (int txx=0;txx<20;txx++){
				
					if (tmparea[txx][tyy]=='k'){
						kosilkax=tyy;
						kosilkay=txx;
					}

					gamearea[txx][tyy]=tmparea[tyy][txx];
				}
			}
	}

		if (levnum==4){
		static char tmparea[20][20]={{'t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t'},
								{'t','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s','n','t'},
								{'t','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s','n','t'},
								{'t','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s','n','t'},
								{'t','g','g','g','g','g','g','g','t','t','g','g','g','g','g','g','g','s','n','t'},
								{'t','g','g','g','g','g','g','g','t','t','g','g','g','g','g','g','g','s','n','t'},
								{'t','g','g','g','g','g','g','g','t','t','g','g','g','g','g','g','g','s','n','t'},
								{'t','g','g','g','g','g','g','g','t','t','g','g','g','g','g','g','g','s','n','t'},
								{'t','g','g','g','g','g','g','g','t','t','g','g','g','g','g','g','g','s','s','t'},
								{'t','g','g','g','g','t','t','t','t','t','t','t','t','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','t','t','t','t','t','t','t','t','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','g','g','t','t','g','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','g','g','t','t','g','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','g','g','t','t','g','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','g','g','t','t','g','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','g','g','t','t','g','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','t'},
								{'t','k','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','t'},
								{'t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t'}};
			for (int tyy=0;tyy<20;tyy++){
				for (int txx=0;txx<20;txx++){
				
					if (tmparea[txx][tyy]=='k'){
						kosilkax=tyy;
						kosilkay=txx;
					}

					gamearea[txx][tyy]=tmparea[tyy][txx];
				}
			}
	}

			if (levnum==5){
		static char tmparea[20][20]={{'t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t'},
								{'t','s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','t'},
								{'t','t','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','t'},
								{'t','s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','t'},
								{'t','s','g','g','g','g','g','t','g','g','t','g','g','g','g','g','g','g','g','t'},
								{'t','t','g','g','g','g','t','n','t','t','t','g','g','g','g','g','g','g','g','t'},
								{'t','s','g','g','g','g','t','n','n','n','t','g','g','g','g','g','g','g','g','t'},
								{'t','k','g','g','g','g','t','n','n','n','t','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','t','n','n','n','t','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','t','n','n','t','g','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','t','n','n','t','g','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','t','n','n','n','t','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','t','n','n','n','t','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','g','t','n','t','t','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','g','t','t','g','g','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','t'},
								{'t','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','t'},
								{'t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t','t'}};
			for (int tyy=0;tyy<20;tyy++){
				for (int txx=0;txx<20;txx++){
				
					if (tmparea[txx][tyy]=='k'){
						kosilkax=tyy;
						kosilkay=txx;
					}

					gamearea[txx][tyy]=tmparea[tyy][txx];
				}
			}
			}

		if (levnum==6){
		static char tmparea[20][20]={{'s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s'},
								{'s','k','t','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s'},
								{'s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s'},
								{'s','g','g','s','s','s','s','s','s','s','s','s','s','s','s','s','s','g','g','s'},
								{'s','g','g','g','g','g','g','g','g','g','g','g','g','g','t','t','s','g','g','s'},
								{'s','g','g','s','g','g','g','g','g','g','g','g','g','g','t','t','s','g','g','s'},
								{'s','g','g','s','g','g','g','g','g','g','g','g','g','g','t','t','s','g','g','s'},
								{'s','g','g','s','g','g','g','g','g','t','t','g','g','g','g','g','s','g','g','s'},
								{'s','g','g','s','g','g','g','g','g','t','t','t','g','g','g','g','s','g','g','s'},
								{'s','g','g','s','g','g','g','t','t','t','t','t','t','g','g','g','s','g','g','s'},
								{'s','g','g','s','g','g','g','t','t','t','t','t','t','g','g','g','s','g','g','s'},
								{'s','g','g','s','g','g','g','g','g','t','t','g','g','g','g','g','s','g','g','s'},
								{'s','g','g','s','g','g','g','g','g','t','t','g','g','g','g','g','s','g','g','s'},
								{'s','g','g','s','g','g','g','g','g','g','g','g','g','g','g','g','s','g','g','s'},
								{'s','g','g','s','g','g','g','g','g','g','g','g','g','g','g','g','s','g','g','s'},
								{'s','g','g','s','g','g','g','g','g','g','g','g','g','g','g','g','s','g','g','s'},
								{'s','g','g','s','s','s','s','s','s','s','s','s','s','s','s','s','s','g','g','s'},
								{'s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s'},
								{'s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s'},
								{'s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s'}};
			for (int tyy=0;tyy<20;tyy++){
				for (int txx=0;txx<20;txx++){
				
					if (tmparea[txx][tyy]=='k'){
						kosilkax=tyy;
						kosilkay=txx;
					}

					gamearea[txx][tyy]=tmparea[tyy][txx];
				}
			}
		}		


			if (levnum==7){
		static char tmparea[20][20]={{'s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s'},
								{'s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s','t','t','s'},
								{'s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s','t','t','s'},
								{'s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s','t','t','s'},
								{'s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s','t','t','s'},
								{'s','g','g','g','g','g','t','t','t','t','t','t','g','g','g','g','s','t','t','s'},
								{'s','g','g','g','g','g','t','k','g','g','g','t','g','g','g','g','s','t','t','s'},
								{'s','g','g','g','g','g','t','g','g','g','g','t','g','g','g','g','s','t','t','s'},
								{'s','g','g','g','g','g','t','g','g','g','g','t','g','g','g','g','s','t','t','s'},
								{'s','g','g','g','g','g','t','g','g','g','g','t','g','g','g','g','s','s','s','s'},
								{'s','g','g','g','g','g','t','g','g','g','g','t','g','g','g','g','g','g','g','s'},
								{'s','g','g','g','g','g','t','t','t','t','g','t','g','g','g','g','g','g','g','s'},
								{'s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s'},
								{'s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s'},
								{'s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s'},
								{'s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s'},
								{'s','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','g','s'},
								{'s','g','g','g','g','g','g','g','g','t','g','g','t','g','g','g','g','g','g','s'},
								{'s','g','g','g','g','g','g','g','g','t','g','g','t','g','g','g','g','g','g','s'},
								{'s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s','s'}};
			for (int tyy=0;tyy<20;tyy++){
				for (int txx=0;txx<20;txx++){
				
					if (tmparea[txx][tyy]=='k'){
						kosilkax=tyy;
						kosilkay=txx;
					}

					gamearea[txx][tyy]=tmparea[tyy][txx];
				}
			}
		}		
		}
		else
		{
			//External levels
			kosilkadirection=1;

			int currentrow=0;
			int currentcol=0;

			for (int tmpcntr=0;tmpcntr<400;tmpcntr++){
				
				currentcol=(int)(tmpcntr/20);
				currentrow=tmpcntr-(((int)(tmpcntr/20))*20);
				switch(externallevels[tmpcntr+(400*(levnum-1))]){

				case 0:
					gamearea[currentrow][currentcol]='n';
					break;
				case 1:
					gamearea[currentrow][currentcol]='g';
					break;
				case 2:
					gamearea[currentrow][currentcol]='k';
					kosilkax=currentrow;
					kosilkay=currentcol;
					break;
				case 3:
					gamearea[currentrow][currentcol]='s';
					break;
				case 4:
					gamearea[currentrow][currentcol]='t';
					break;

				}
			}
			

		}

	draw_window();

}

int grassLeft(){
	int leftgrass=0;
	for (int chky=0;chky<20;chky++){
		for (int chkx=0;chkx<20;chkx++){
			if (gamearea[chkx][chky]=='g') {
				leftgrass++;
			}
		}
	}
	return leftgrass;
}

//Change level or show win message 
void updateStatus(){

	if (grassLeft()==0) {
		if (level==levelcount){
			gamestarted=false;
			status=2;
			draw_window();
		} else {
			gamestarted=false;
			interlevelpause();
			level++;
			initializeLevel(level);
			gamestarted=true;
		}
	}
	
}


void load_external_levels(){

	CKosFile lev(getLevelsPathName());

	level_read_result=lev.Read (external_levels_count,1);

	if (level_read_result == 1)
			external_levels_available=true;
	else
			external_levels_available=false;

	if (external_levels_count[0]==0)
			external_levels_available=false;
	else
	{
		externallevels = new Byte[400*external_levels_count[0]];
		lev.Read (externallevels,400*external_levels_count[0]);
	}
}

void app_halt(){

	delete apppath;
	delete levpath;
	if (external_levels_available==true) {delete externallevels;}
	kos_ExitApp();
}



void kos_Main(){

	load_external_levels();
	
	decode_graphics(576*0 + kosilka_gfx,kosilka_d);
	decode_graphics(576*1 + kosilka_gfx,kosilka_u);
	decode_graphics(576*2 + kosilka_gfx,kosilka_l);
	decode_graphics(576*3 + kosilka_gfx,kosilka_r);
	decode_graphics(576*4 + kosilka_gfx,skos);
	decode_graphics(576*5 + kosilka_gfx,stone);
	decode_graphics(576*6 + kosilka_gfx,tree);
	decode_graphics(576*7 + kosilka_gfx,grass);

	for (;;){

		switch (kos_WaitForEvent()){
		case 1:
			w_redraw=true;
			draw_window();
			break;
		case 2:
			Byte keyCode;
			kos_GetKey(keyCode);
			if (status==1){
			if (gamestarted==true){	
				switch (keyCode){
				case 177:
					if (kosilkay<19){
						if (gamearea[kosilkax][kosilkay+1]=='g'){
							gamearea[kosilkax][kosilkay]='n';
							gamearea[kosilkax][kosilkay+1]='k';
							
							if (drawanimation==true) {animate(kosilkax,kosilkay,1);}


							lastkosilkadirection=kosilkadirection;
							laststep=1;

							kosilkay++;
							kosilkadirection=1;
							draw_element(kosilkax,kosilkay);
							draw_element(kosilkax,kosilkay-1);
							display_grass_left();
							updateStatus();
						}
					}
					break;
				case 178:
					if (kosilkay>0){
						if (gamearea[kosilkax][kosilkay-1]=='g'){
							gamearea[kosilkax][kosilkay]='n';
							gamearea[kosilkax][kosilkay-1]='k';

							if (drawanimation==true) {animate(kosilkax,kosilkay,2);}

							lastkosilkadirection=kosilkadirection;
							laststep=2;

							kosilkay--;
							kosilkadirection=2;
					
							draw_element(kosilkax,kosilkay);
							draw_element(kosilkax,kosilkay+1);
							display_grass_left();
							updateStatus();
						}
					}
					break;	
	
				case 179:
					if (kosilkax<19){
						if (gamearea[kosilkax+1][kosilkay]=='g'){
							gamearea[kosilkax][kosilkay]='n';
							gamearea[kosilkax+1][kosilkay]='k';

							if (drawanimation==true) {animate(kosilkax,kosilkay,3);}


							lastkosilkadirection=kosilkadirection;
							laststep=3;

							kosilkax++;
							kosilkadirection=4;

							draw_element(kosilkax,kosilkay);
							draw_element(kosilkax-1,kosilkay);
							display_grass_left();
							updateStatus();
						}
					}
					break;
			
				case 176:
					if (kosilkax>0){
						if (gamearea[kosilkax-1][kosilkay]=='g'){
							gamearea[kosilkax][kosilkay]='n';
							gamearea[kosilkax-1][kosilkay]='k';

							if (drawanimation==true) {animate(kosilkax,kosilkay,4);}

							lastkosilkadirection=kosilkadirection;
							laststep=4;

							kosilkax--;
							kosilkadirection=3;

							draw_element(kosilkax,kosilkay);
							draw_element(kosilkax+1,kosilkay);
							display_grass_left();
							updateStatus();
						}
					}
					break;

				case 27:
					if (lives>0){
						lives--;
						initializeLevel(level);

					} else {
						gamestarted=false;
						status=3;
						draw_window();
					}
					break;

				case 8:
					if (laststep!=0){
						kosilkadirection=lastkosilkadirection;
						if (laststep==1){
							gamearea[kosilkax][kosilkay]='g';
							gamearea[kosilkax][kosilkay-1]='k';
							draw_element(kosilkax,kosilkay);
							draw_element(kosilkax,kosilkay-1);
							kosilkay--;
						}
					if (laststep==2){
							gamearea[kosilkax][kosilkay]='g';
							gamearea[kosilkax][kosilkay+1]='k';
							draw_element(kosilkax,kosilkay);
							draw_element(kosilkax,kosilkay+1);
							kosilkay++;
						}

					if (laststep==3){
							gamearea[kosilkax][kosilkay]='g';
							gamearea[kosilkax-1][kosilkay]='k';
							draw_element(kosilkax,kosilkay);
							draw_element(kosilkax-1,kosilkay);
							kosilkax--;
						}

					if (laststep==4){
							gamearea[kosilkax][kosilkay]='g';
							gamearea[kosilkax+1][kosilkay]='k';
							draw_element(kosilkax,kosilkay);
							draw_element(kosilkax+1,kosilkay);
							kosilkax++;
						}


						laststep=0;
					}
					break;
		
				}

			}
			}
			if (status==0){
				if (keyCode==13){ //enter
					if (external_levels_available==true){
						status=4;
						draw_window();
					}
					else
					{
						status=1;
						initializeLevel(1);
						gamestarted=true;
					}
									
				}

				if (keyCode==103){
					if (drawgraphics==true){
						drawgraphics=false;
					} else {
						drawgraphics=true;
					}
					
					if (drawgraphics==true){
						kos_PutImage((RGB*)kosilka_d,24,24,305,150);
					}
					else
					{
					kos_DrawBar(305,150,24,24,0x00AAAA);
					}
				}
				
				if (keyCode==97){
					if (drawanimation==true){
						drawanimation=false;
					} else {
						drawanimation=true;
					}
					
					if (drawanimation==true){
						kos_DrawBar(335,150,24,24,0x00AA00);
					}
					else
					{
						kos_DrawBar(335,150,24,24,0xAA0000);
					}
				}
				if (keyCode==104){
						status=-1;
						draw_window();
				}

				if (keyCode==27){
					app_halt();
				}
				
			}

			if (status==4){
				if (keyCode==49){ //1
						external_levels=false;
						status=1;
						initializeLevel(1);
						gamestarted=true;
									
				}
				
				if (keyCode==50){ //2
						external_levels=true;
						levelcount=external_levels_count[0];
						status=1;
						initializeLevel(1);
						gamestarted=true;
									
				}
			}

			if (status==2){
				if (keyCode==113){
						lives=2;
						status=0;
						level=1;
						draw_window();
						gamestarted=true;
				}
			}
			
			if (status==3){
				if (keyCode==113){
						lives=2;
						status=0;
						level=1;
						draw_window();
						gamestarted=true;	

				}
				if (keyCode==114){
						lives=2;
						status=1;
						level=1;
						initializeLevel(1);
						gamestarted=true;
				}

			}

			if (status==-1){
				if (keyCode==8){
					status=0;
					draw_window();
				}
			}

			break;
		case 3:
			
			app_halt();
			break;
		}
	}
}
