//screen Width and Height
#define Width  600
#define Height 600
int ScreenX=0;
int ScreenY=0;
int OffsetX=0;
int OffsetY=0;
int BufferDraw [Width][Height];
char BufferCarry [Height][Width][3];
int BufferCarry2 [Height][Width];
int DRAW_TECH=0;
int Analyx [Width][Height];
int CollideVerts[10][2]={};
int DeltaH [8]={11,16,19,20,21,22,23,23};
//-1-quit, 0-menu, 1-game
int GAME_TYPE=0;

int GLOBAL_SPEED=0;
int Max_Speed=5;
//colors
int COLOR_INDEX=0;
char RAINBOW_NAME[7][7]={"red","orange","yellow","green","blue","indigo","violet"};
int RAINBOW_TABLE[7][5]= {{0x00ff0000, 0x00e80c7a, 0x00ff0dff, 0x00e82c0c, 0x00ff530d},
			  {0x00ff4700, 0x00e8690c, 0x00ff970d, 0x00ff0d18, 0x00e8290c},
			  {0x00ffff00, 0x00e8d20c, 0x00ffce0d, 0x009be80c, 0x0056ff0d},
			  {0x0000ff00, 0x000ce84a, 0x0059e80c, 0x000dff96, 0x00b6ff0d},
			  {0x0000ffff, 0x000d7fff, 0x000caeeb, 0x000dff76, 0x000ce8aa},
			  {0x000000ff, 0x000c46eb, 0x00480ce8, 0x00910dff, 0x000d8cff},
			  {0x006700ff, 0x00a00ce8, 0x00f20dff, 0x000d2eff, 0x00280ce8}};
int GLOBAL_BLOCKCOLOR=0x00e80c7a;
int GLOBAL_BATUTCOLOR=0x00ff0dff;
int GLOBAL_PITCOLOR=0x00e82c0c;
int GLOBAL_FLAGCOLOR=0x00ff530d;
int GLOBAL_BACKGROUNDCOLOR=0x00000000;
int GLOBAL_FRONTCOLOR=0x00ffffff;
//menu settings
int MENU_SELECTED=0;
char NUMBER[100]="0";
int CURRENT_LEVEL=3;
int MAX_LEVEL=3;
int TO_NEXT_LEVEL=0;
int DeltaSpeed=0;
int GLOBAL_CHECKPOINT=0;
int SPAWN_Y=0;
int isRestart=1;
int CurrentCheck=0;

//Hero data
int HeroX=100;
int HeroY=100;
int HeroSides=3;
int HeroAngle=0;
int HeroFly=0;
char HeroIsDead=0;
int HeroColor=0x00ff0000;
char Key=0;

//Saveload folder
int SAVE_FOLDER_TYPE=0;

//EDITOR
int Tile_X=0;
int Tile_Y=0;
int Panel=0;  //0-grid,1-tools, 2-props
int Tile_Type=0;
int Q_SELECTED=0;
char TILENAME[6][11]={"empty","block","spike","jump pad","checkpt","finish"};

//OTHER
int THE_END_COUNT=600;
char Arrow='0';
//Libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kolibrisys.h>
#include <GameAlphabet.h>
#include <Shimath.h>
#include <Draw2D.h>
#include <Levels.h>
#include <GameFunctions.h>



//result of getKey is key character. Condition for Escape - if (getKey()==27)
char getKey() {
	int NewKey=_ksys_get_key();
	NewKey/=256;
	Arrow=(NewKey/256)%256;
	return (NewKey%256);
}
void LoadData () {
	FILE *Profile;
	if (SAVE_FOLDER_TYPE==1) {
		Profile=fopen ("/usbhd0/1/Profile.bin","rb");
	} else {
		Profile=fopen ("Profile.bin","rb");
	}
	char Format=0;
	Format=fgetc (Profile);
	if (Format!='N') {
		DrawText (100,300,"error (cannot load data)",GLOBAL_FRONTCOLOR);
		Update();
		_ksys_delay(200);
		MENU_SELECTED=1;
		fclose (Profile);
		return;
	}
	//Loading operations
	fread (&(COLOR_INDEX), sizeof(int),1,Profile);
	HeroColor=COLOR_INDEX;
	fread (&(GLOBAL_BACKGROUNDCOLOR), sizeof(int),1,Profile);
	fread (&(GLOBAL_FRONTCOLOR),sizeof(int),1,Profile);
	Update();
	fread (&(DRAW_TECH),sizeof(int),1,Profile);
	Update();
	MAX_LEVEL=fgetc (Profile);
	CURRENT_LEVEL=MAX_LEVEL;
	fread (&(LevelProps[0][2]), sizeof(int),1, Profile);
	fread (USER_LEVEL0,1,9*LEVEL_MAXLEN,Profile);
	fread (&(LevelProps[1][2]), sizeof(int),1,Profile);
	fread (USER_LEVEL1,1,9*LEVEL_MAXLEN,Profile);
	fread (&(LevelProps[2][2]), sizeof(int),1,Profile);
	fread (USER_LEVEL2,1,9*LEVEL_MAXLEN,Profile);
	DrawText (100,300,"loaded successfully",GLOBAL_FRONTCOLOR);
	Update();
	_ksys_delay(200);
	Update();

	MENU_SELECTED=1;
	fclose (Profile);
}

void SaveData () {
	FILE *Profile;
	if (SAVE_FOLDER_TYPE==1) {
		Profile=fopen ("/usbhd0/1/Profile.bin","wb");
	} else {
		Profile=fopen ("Profile.bin","wb");
	}
	if (Profile==NULL) {
		DrawText (100,300,"error (cannot save data)",GLOBAL_FRONTCOLOR);
		Update();
		_ksys_delay(200);
		MENU_SELECTED=1;
		return;
	}
	fputc ('N',Profile);
	fwrite (&(COLOR_INDEX), sizeof(int),1,Profile);
	fwrite (&(GLOBAL_BACKGROUNDCOLOR), sizeof(int),1,Profile);
	fwrite (&(GLOBAL_FRONTCOLOR),sizeof(int),1,Profile);
	fwrite (&(DRAW_TECH),sizeof(int),1,Profile);
	fputc (MAX_LEVEL,Profile);
	fwrite (&(LevelProps[0][2]), sizeof(int),1, Profile);
	fwrite (USER_LEVEL0,1,9*LEVEL_MAXLEN,Profile);
	fwrite (&(LevelProps[1][2]), sizeof(int),1,Profile);
	fwrite (USER_LEVEL1,1,9*LEVEL_MAXLEN,Profile);
	fwrite (&(LevelProps[2][2]), sizeof(int),1,Profile);
	fwrite (USER_LEVEL2,1,9*LEVEL_MAXLEN,Profile);
	DrawText (100,300,"saved successfully",GLOBAL_FRONTCOLOR);
	Update();
	_ksys_delay(200);
	MENU_SELECTED=1;
	fclose (Profile);
}

void MainMenu () {
	if (Key==27) GAME_TYPE=-1;
	if (MENU_SELECTED==11 && (Key==' ' || Arrow==28)) GAME_TYPE=-1;
	if (Key=='s' || Arrow=='P') MENU_SELECTED++;
	if (Key=='w' || Arrow=='H') MENU_SELECTED--;
	if (MENU_SELECTED<0) MENU_SELECTED=11;
	if (MENU_SELECTED>11) MENU_SELECTED=0;
	if (MENU_SELECTED==0 && (Key==' ' || Arrow==28)) {
		GAME_TYPE=1;
		GLOBAL_CHECKPOINT=0;
		return;
	}
	if (MENU_SELECTED==3 && (Key=='a' || Key=='d' || Arrow=='K' || Arrow=='M')) {
		if (GLOBAL_BACKGROUNDCOLOR==0) {
			GLOBAL_BACKGROUNDCOLOR=0x00ffffff;
			GLOBAL_FRONTCOLOR=0;
		} else {
			GLOBAL_BACKGROUNDCOLOR=0;
			GLOBAL_FRONTCOLOR=0x00ffffff;
		}
	}
	if (MENU_SELECTED==1) {
		if ((Key=='a' || Arrow=='K') && CURRENT_LEVEL>0) CURRENT_LEVEL--;
		if ((Key=='d' || Arrow=='M') && CURRENT_LEVEL<MAX_LEVEL) CURRENT_LEVEL++;
		HeroSides=LevelProps[CURRENT_LEVEL][2];
	}
	if (MENU_SELECTED==2) {
		if ((Key=='a' || Arrow=='K') && COLOR_INDEX>0) COLOR_INDEX--;
		if ((Key=='d' || Arrow=='M') && COLOR_INDEX<6) COLOR_INDEX++;
		HeroColor=RAINBOW_TABLE [COLOR_INDEX][0];
		GLOBAL_BLOCKCOLOR=RAINBOW_TABLE [COLOR_INDEX][1];
		GLOBAL_BATUTCOLOR=RAINBOW_TABLE [COLOR_INDEX][2];
		GLOBAL_PITCOLOR=RAINBOW_TABLE [COLOR_INDEX][3];
		GLOBAL_FLAGCOLOR=RAINBOW_TABLE [COLOR_INDEX][4];
	}
	if (MENU_SELECTED==7 && (Key=='a' || Key=='d' || Arrow=='K' || Arrow=='M')) {
		if (SAVE_FOLDER_TYPE==0) {
			SAVE_FOLDER_TYPE=1;
		} else {
			SAVE_FOLDER_TYPE=0;
		}
	}
	if (MENU_SELECTED==8 && (Key=='a' || Key=='d' || Arrow=='K' || Arrow=='M')) {
		int i=0;
		int j=0;
		for (i=0; i<Width; i++) {
			for (j=0; j<Height; j++) {
				BufferDraw[i][j]=GLOBAL_BACKGROUNDCOLOR;
			}
		}
		Update();
		if (DRAW_TECH==0) {
			DRAW_TECH=1;
		} else {
			DRAW_TECH=0;
		}
		Update();
	}
	if (MENU_SELECTED==9) {
		if ((Key=='a' || Arrow=='K') && Max_Speed>1) Max_Speed--;
		if ((Key=='d' || Arrow=='M') && Max_Speed<14) Max_Speed++;
	}
	if (MENU_SELECTED==4 && (Key==' ' || Arrow==28)) LoadData();
	if (MENU_SELECTED==5 && (Key==' ' || Arrow==28)) SaveData();
	if (MENU_SELECTED==6 && (Key==' ' || Arrow==28)) {
		if (CURRENT_LEVEL>=3) {
			DrawText (300,300,"error",GLOBAL_FRONTCOLOR);
			DrawText (30,320,"(choose level 0/1/2 to edit them)",GLOBAL_FRONTCOLOR);
			Update();
			_ksys_delay(200);
			MENU_SELECTED=1;
		} else {
			GAME_TYPE=3;
			Panel=1;
		}
	}
	if (Key=='f' && CURRENT_LEVEL<3) {
		GAME_TYPE=3;
		Panel=1;
	}
	if (MENU_SELECTED==10 && (Key==' ' || Arrow==28)) GAME_TYPE=2;


	DrawTitle (60,100,GLOBAL_FRONTCOLOR);
	DrawText (100,300,"start game",GLOBAL_FRONTCOLOR);
	DrawText (100,320,"choose level <   >",GLOBAL_FRONTCOLOR);
	IntToStr(CURRENT_LEVEL,NUMBER);
	DrawText (345,320,NUMBER,GLOBAL_FRONTCOLOR);
	DrawText (100,340,"hero color <        >",GLOBAL_FRONTCOLOR);
	DrawText (330, 340, RAINBOW_NAME [COLOR_INDEX], GLOBAL_FRONTCOLOR);
	DrawText (100,360,"background color <        >",GLOBAL_FRONTCOLOR);
	if (GLOBAL_BACKGROUNDCOLOR==0) {
		DrawText (430,360,"black",GLOBAL_FRONTCOLOR);
	} else {
		DrawText (430,360,"white",GLOBAL_FRONTCOLOR);
	}

	DrawText (100,400,"load game",GLOBAL_FRONTCOLOR);
	DrawText (100,420,"save game",GLOBAL_FRONTCOLOR);
	DrawText (100,440,"level editor",GLOBAL_FRONTCOLOR);
	DrawText (100,460,"saveload folder <          >",GLOBAL_FRONTCOLOR);
	if (SAVE_FOLDER_TYPE==0) {
		DrawText (400,460,"local",GLOBAL_FRONTCOLOR);
	} else {
		DrawText (400,460,"usbhd0/1/",GLOBAL_FRONTCOLOR);
	}
	DrawText (100,480,"redraw technology <        >",GLOBAL_FRONTCOLOR);
	if (DRAW_TECH==0) {
		DrawText (450,480,"frame",GLOBAL_FRONTCOLOR);
	} else {
		DrawText (450,480,"lines",GLOBAL_FRONTCOLOR);
	}
	DrawText (100,500,"max speed <   >",GLOBAL_FRONTCOLOR);
	IntToStr(Max_Speed,NUMBER);
	DrawText (300,500,NUMBER,GLOBAL_FRONTCOLOR);
	DrawText (100,520,"help",GLOBAL_FRONTCOLOR);
	DrawText (100,540,"quit",GLOBAL_FRONTCOLOR);
	DrawText (20,580,"developed by e_shi games 2014",GLOBAL_FRONTCOLOR);

	if (MENU_SELECTED==0) DrawText (100,300,"start game",HeroColor);
	if (MENU_SELECTED==1) DrawText (100,320,"choose level <   >",HeroColor);
	if (MENU_SELECTED==2) DrawText (100,340,"hero color <        >",HeroColor);
	if (MENU_SELECTED==3) DrawText (100,360,"background color <        >",HeroColor);
	if (MENU_SELECTED==4) DrawText (100,400,"load game",HeroColor);
	if (MENU_SELECTED==5) DrawText (100,420,"save game",HeroColor);
	if (MENU_SELECTED==6) DrawText (100,440,"level editor",HeroColor);
	if (MENU_SELECTED==7) DrawText (100,460,"saveload folder <          >",HeroColor);
	if (MENU_SELECTED==8) DrawText (100,480,"redraw technology <        >",HeroColor);
	if (MENU_SELECTED==9) DrawText (100,500,"max speed <   >",HeroColor);
	if (MENU_SELECTED==10) DrawText (100,520,"help",HeroColor);
	if (MENU_SELECTED==11) DrawText (100,540,"quit",HeroColor);


}





void GamePlay () {
	DrawLine (0,500,800,500,GLOBAL_FRONTCOLOR);
	if (Objects==0 || DataBase[Objects-1][1]<=Width-40) {
		ReadLevel(CURRENT_LEVEL);
	 }

	 if (HeroIsDead==0) {
		if (isRestart==1) {
			HeroAngle=(HeroAngle+10)%360;
			HeroIsDead=CheckCollision();
			HeroY+=HeroFly;
			DrawHero (125,HeroY, HeroSides, HeroAngle, HeroColor);
		} else {
			DrawHero (125,HeroY,HeroSides,HeroAngle,GLOBAL_FRONTCOLOR);
		}
		if (TO_NEXT_LEVEL>0) {
			DrawText (Width-TO_NEXT_LEVEL,300,"jump to start new level",GLOBAL_FRONTCOLOR);
			if (TO_NEXT_LEVEL<500) {
				TO_NEXT_LEVEL+=2;
			} else {
				if ((Key==' ' || Arrow==28) && TO_NEXT_LEVEL<=504) {
					CURRENT_LEVEL++;
					HeroSides=CURRENT_LEVEL;
					if (CURRENT_LEVEL>MAX_LEVEL) MAX_LEVEL=CURRENT_LEVEL;
					GLOBAL_CHECKPOINT=0;
					TO_NEXT_LEVEL=505;
				}
				if (TO_NEXT_LEVEL>=505) {
					TO_NEXT_LEVEL+=2;
					if (TO_NEXT_LEVEL>1000) TO_NEXT_LEVEL=0;
				}
			}

		}

	  } else {
		GLOBAL_SPEED=0;
		DeltaSpeed=0;
		TO_NEXT_LEVEL=0;
		HeroIsDead++;
		DrawPew (125,HeroY,HeroIsDead,HeroColor);
		if (HeroIsDead>100) {
			 ResetLevel(CURRENT_LEVEL);
			 if (GLOBAL_CHECKPOINT==0) {
				isRestart=1;
				HeroY=100;
			 } else {
				isRestart=0;
				HeroY=SPAWN_Y+41-DeltaH[HeroSides];
				if (HeroSides>7) HeroY-=22;
			 }
			 HeroIsDead=0;
			 HeroFly=0;
			 if (HeroSides%2==0) {
				HeroAngle=360/(HeroSides*2);
			 } else {
				HeroAngle=0;
			 }
		}
	  }
	  if (Key=='f'&& CURRENT_LEVEL<3) {
		int i=0;
		Objects=0;
		for (i=0; i<11; i++) LevelProps[i][0]=0;
		isRestart=1;
		HeroIsDead=0;
		HeroY=100;
		HeroFly=1;
		TO_NEXT_LEVEL=0;
		GAME_TYPE=3;
		Panel=1;
	  }
	  if (Key==27) {
		GLOBAL_CHECKPOINT=0;
		int i=0;
		for (i=0; i<11; i++) ResetLevel (i);
		isRestart=1;
		HeroIsDead=0;
		HeroY=100;
		HeroFly=1;
		TO_NEXT_LEVEL=0;
		if (CURRENT_LEVEL<3) {
			GAME_TYPE=3;
			Panel=1;
			Key=' ';
		} else {
			GAME_TYPE=0;
		}
	  }
}

void ShowHelp() {
	DrawText (5,10,"controls",HeroColor);
	DrawText (5,30,"w/a/s/d or arrow keys to choose",GLOBAL_FRONTCOLOR);
	DrawText (5,50,"space jump",GLOBAL_FRONTCOLOR);
	DrawText (5,70,"space/enter to select",GLOBAL_FRONTCOLOR);
	DrawText (5,90,"escape return to menu/exit",GLOBAL_FRONTCOLOR);
	DrawText (5,130,"level editor notes",HeroColor);
	DrawText (5,150,"press e to switch grid/tools panel",GLOBAL_FRONTCOLOR);
	DrawText (5,170,"press q to switch grid/properties",GLOBAL_FRONTCOLOR);
	DrawText (5,190,"select save game to save all levels",GLOBAL_FRONTCOLOR);
	DrawText (5,210,"select load game to load all levels",GLOBAL_FRONTCOLOR);
	DrawText (5,230,"select level 0/1/2 to play/edit",GLOBAL_FRONTCOLOR);
	DrawText (5,250,"use f to go to level editor quickly",GLOBAL_FRONTCOLOR);
	DrawText (5,270,"use start column to test level from",GLOBAL_FRONTCOLOR);
	DrawText (5,290,"desired place (you can test",GLOBAL_FRONTCOLOR);
	DrawText (5,310,"level from column which contains",GLOBAL_FRONTCOLOR);
	DrawText (5,330,"checkpoint)",GLOBAL_FRONTCOLOR);
	DrawText (5,370,"redraw technology notes",HeroColor);
	DrawText (5,390,"lines tech works faster but",GLOBAL_FRONTCOLOR);
	DrawText (5,410,"sometimes it can be unstable",GLOBAL_FRONTCOLOR);
	DrawText (5,430,"frame tech works slower but stable",GLOBAL_FRONTCOLOR);

	 if (Key=='f' && CURRENT_LEVEL<3) {
		GAME_TYPE=3;
		Panel=1;
	}

	if (Key==27) GAME_TYPE=0;
}

void SaveArray() {
	FILE *ArrTxt;

	if (SAVE_FOLDER_TYPE==1) {
		ArrTxt=fopen ("/usbhd0/1/LEVEL2D.txt","wb");
	} else {
		ArrTxt=fopen ("LEVEL2D.txt","wb");
	}
	if (ArrTxt==NULL) {
		DrawText (100,300,"error (cannot save data)",GLOBAL_FRONTCOLOR);
		Update();
		_ksys_delay(200);
		Panel=1;
		return;
	}
	char StringLevel[3*9*LEVEL_MAXLEN]="unsigned char NEW_LEVEL[9][LEVEL_MAXLEN]=\n{";
	int CurCharIndex=43;
	int i=0;
	int j=0;
	for (i=0;i<9;i++) {
		StringLevel[CurCharIndex]='{';
		CurCharIndex++;
		for (j=0; j<LEVEL_MAXLEN; j++) {
			StringLevel[CurCharIndex]= *(Levels[CURRENT_LEVEL]+i*LEVEL_MAXLEN+j)+48;
			CurCharIndex++;
			if (j<LEVEL_MAXLEN-1) {
				StringLevel[CurCharIndex]=',';
				CurCharIndex++;
			}
		}
		StringLevel[CurCharIndex]='}';
		CurCharIndex++;
		if (i<8) {
			StringLevel[CurCharIndex]=',';
			CurCharIndex++;
		}
		StringLevel[CurCharIndex]='\n';
		CurCharIndex++;
	}
	StringLevel[CurCharIndex]='}';
	CurCharIndex++;
	StringLevel[CurCharIndex]=';';
	CurCharIndex++;
	StringLevel[CurCharIndex]='\0';
	fwrite (StringLevel,1,CurCharIndex,ArrTxt);
	fclose (ArrTxt);
}

void LevelEditor () {
	int i=0;
	int j=0;
	if (Panel==0) {
		if ((Key=='d' || Arrow=='M') && Tile_X<399) Tile_X++;
		if ((Key=='a' || Arrow=='K') && Tile_X>0) Tile_X--;
		if ((Key=='s' || Arrow=='P') && Tile_Y<8) Tile_Y++;
		if ((Key=='w' || Arrow=='H') && Tile_Y>0) Tile_Y--;
		if ((Key==' ' || Arrow==28)) {
			if (Tile_Type==5) {
				for (i=0; i<LEVEL_MAXLEN; i++) if (*(Levels[CURRENT_LEVEL]+0*LEVEL_MAXLEN+i)==5) *(Levels[CURRENT_LEVEL]+0*LEVEL_MAXLEN+i)=0;
				*(Levels[CURRENT_LEVEL]+0*LEVEL_MAXLEN+Tile_X)=5;
			} else {
				*(Levels[CURRENT_LEVEL]+Tile_Y*LEVEL_MAXLEN+Tile_X)=Tile_Type;
			}
		}
	}
	if (Panel==1) {
		if ((Key=='s' || Arrow=='P') && Tile_Type<5) Tile_Type++;
		if ((Key=='w' || Arrow=='H') && Tile_Type>0) Tile_Type--;
	}
	if (Panel==2) {
		if ((Key=='s' || Arrow=='P') && Q_SELECTED<5) Q_SELECTED++;
		if ((Key=='w' || Arrow=='H') && Q_SELECTED>0) Q_SELECTED--;
		if ((Key==' ' || Arrow==28) && Q_SELECTED==0) {
			GAME_TYPE=1;
			if (GLOBAL_CHECKPOINT>0) {
				int isCheck=0;
				for (i=0;i<9; i++) {
					int CurBlock=*(Levels[CURRENT_LEVEL]+i*LEVEL_MAXLEN+GLOBAL_CHECKPOINT);
					if (CurBlock==4) {
						SPAWN_Y=(i+1)*43+41+70-DeltaH[HeroSides];
						isCheck=1;
						break;
					}
				}
				if (isCheck==0) {
					DrawText (100,300,"error (checkpoint not found)",GLOBAL_FRONTCOLOR);
					Update();
					_ksys_delay(200);
					GAME_TYPE=3;
					Panel=1;
					return;
				}
				ResetLevel(CURRENT_LEVEL);
				isRestart=0;
				HeroIsDead=0;
				HeroFly=0;
				HeroY=SPAWN_Y;
				if (HeroSides>7) HeroY-=22;
				if (HeroSides%2==0) {
				       HeroAngle=360/(HeroSides*2);
				} else {
				       HeroAngle=0;
				}
			}
		}
		if (Q_SELECTED==1) {
			if ((Key=='d' || Arrow=='M') && GLOBAL_CHECKPOINT<399) GLOBAL_CHECKPOINT++;
			if ((Key=='a' || Arrow=='K') && GLOBAL_CHECKPOINT>0) GLOBAL_CHECKPOINT--;
		}
		if ((Key==' ' || Arrow==28) && Q_SELECTED==2) {
			for (i=0; i<9; i++) {
				for (j=0; j<LEVEL_MAXLEN; j++) {
					*(Levels[CURRENT_LEVEL]+i*LEVEL_MAXLEN+j)=0;
				}
			}
		}
		if ((Key==' ' || Arrow==28) && Q_SELECTED==3) SaveArray();
		if (Q_SELECTED==4) {
			if ((Key=='a' || Arrow=='K') && LevelProps[CURRENT_LEVEL][2]>3) LevelProps[CURRENT_LEVEL][2]--;
			if ((Key=='d' || Arrow=='M') && LevelProps[CURRENT_LEVEL][2]<10) LevelProps[CURRENT_LEVEL][2]++;
			HeroSides=LevelProps[CURRENT_LEVEL][2];
		}
		if ((Key==' ' || Arrow==28) && Q_SELECTED==5) GAME_TYPE=0;
	}
	if (Key=='e') {
		if (Panel==0) {
			Panel=1;
		} else {
			Panel=0;
		}
	}
	if (Key=='q') {
		if (Panel==0) {
			Panel=2;
		} else {
			Panel=0;
		}
	}
	//Draw grid
	for (i=0; i<=10; i++) {
		DrawLine (43*i,0,43*i,43*9,GLOBAL_FRONTCOLOR);
	}
	for (i=0; i<=9; i++) {
		DrawLine (0,43*i,430,43*i,GLOBAL_FRONTCOLOR);
	}
	//Draw panel
	DrawText (450,10,"e_tools",GLOBAL_FRONTCOLOR);
	IntToStr (Tile_X,NUMBER);
	DrawText (450,30,"(",GLOBAL_FRONTCOLOR);
	DrawText (470,30,NUMBER,GLOBAL_FRONTCOLOR);
	DrawText (530,30,"/",GLOBAL_FRONTCOLOR);
	IntToStr (Tile_Y,NUMBER);
	DrawText (550,30,NUMBER,GLOBAL_FRONTCOLOR);
	DrawText (570,30,")",GLOBAL_FRONTCOLOR);

	DrawLine (480,70,530,70,GLOBAL_FRONTCOLOR);
	DrawLine (480,70,480,70+300,GLOBAL_FRONTCOLOR);
	DrawLine (530,70,530,70+300,GLOBAL_FRONTCOLOR);
	for (i=1;i<=6; i++) DrawLine (480,70+50*i,530,70+50*i,GLOBAL_FRONTCOLOR);
	//Empty block
	DrawLine (480+4,70+4,480+42,70+42,GLOBAL_FRONTCOLOR);
	DrawLine (480+4,70+42,480+42,70+4,GLOBAL_FRONTCOLOR);
	DrawBlock (480+4,70+4+50,GLOBAL_FRONTCOLOR);
	DrawPit (480+7,70+4+100,GLOBAL_FRONTCOLOR);
	DrawBatut (480+4,70+4+150,GLOBAL_FRONTCOLOR);
	DrawFlag (480+8,70+4+200,GLOBAL_FRONTCOLOR);
	DrawText (480+14,70+14+250,"f",GLOBAL_FRONTCOLOR);
	DrawText (450,400,TILENAME [Tile_Type],GLOBAL_FRONTCOLOR);

	//Draw properties
	DrawText (10,410,"q_properties",GLOBAL_FRONTCOLOR);
	DrawText (10,430+20,"test level",GLOBAL_FRONTCOLOR);
	DrawText (10,470,"start column <     >",GLOBAL_FRONTCOLOR);
	IntToStr (GLOBAL_CHECKPOINT,NUMBER);
	DrawText (270,470,NUMBER,GLOBAL_FRONTCOLOR);
	DrawText (10,490,"reset level",GLOBAL_FRONTCOLOR);
	DrawText (10,510,"save as 2d array (for developers)",GLOBAL_FRONTCOLOR);
	DrawText (10,530,"hero sides <   >",GLOBAL_FRONTCOLOR);
	IntToStr (LevelProps[CURRENT_LEVEL][2],NUMBER);
	DrawText (230,530,NUMBER,GLOBAL_FRONTCOLOR);
	DrawText (10,550,"back to menu",GLOBAL_FRONTCOLOR);

	if (Panel==0) {
		DrawLine ((Tile_X%10)*43,0,(Tile_X%10)*43,400,HeroColor);
		DrawLine ((Tile_X%10)*43+43,0,(Tile_X%10)*43+43,400,HeroColor);
		DrawLine (0,(Tile_Y%10)*43,440,(Tile_Y%10)*43,HeroColor);
		DrawLine (0,(Tile_Y%10)*43+43,440,(Tile_Y%10)*43+43,HeroColor);
	}
	if (Panel==1) {
		DrawText (450,10,"e_tools",HeroColor);
		DrawLine (480,70+Tile_Type*50,530,70+Tile_Type*50,HeroColor);
		DrawLine (480,70+Tile_Type*50,480,70+50+Tile_Type*50,HeroColor);
		DrawLine (480,70+Tile_Type*50+50,530,70+50+Tile_Type*50,HeroColor);
		DrawLine (530,70+Tile_Type*50,530,70+Tile_Type*50+50,HeroColor);
		DrawText (450,400,TILENAME [Tile_Type],HeroColor);
	}
	if (Panel==2) {
	       DrawText (10,410,"q_properties",HeroColor);
	       if (Q_SELECTED==0) DrawText (10,450,"test level",HeroColor);
	       if (Q_SELECTED==1) DrawText (10,470,"start column <     >",HeroColor);
	       if (Q_SELECTED==2) DrawText (10,490,"reset level",HeroColor);
	       if (Q_SELECTED==3) DrawText (10,510,"save as 2d array (for developers)",HeroColor);
	       if (Q_SELECTED==4) DrawText (10,530,"hero sides <   >",HeroColor);
	       if (Q_SELECTED==5) DrawText (10,550,"back to menu",HeroColor);
	}

	for (i=0; i<10; i++) {
		for (j=0; j<9; j++) {
			int CurBlock=*(Levels[CURRENT_LEVEL]+j*LEVEL_MAXLEN+Tile_X-Tile_X%10+i);
			if (CurBlock==1) DrawBlock (43*i,43*j,GLOBAL_BLOCKCOLOR);
			if (CurBlock==2) DrawPit (43*i,43*j,GLOBAL_PITCOLOR);
			if (CurBlock==3) DrawBatut (43*i,43*j,GLOBAL_BATUTCOLOR);
			if (CurBlock==4) DrawFlag (43*i,43*j,GLOBAL_FLAGCOLOR);

		}
		if (*(Levels[CURRENT_LEVEL]+0*LEVEL_MAXLEN+Tile_X-Tile_X%10+i)==5) {
			for (j=0; j<9;j++) {
				DrawText (43*i+10,43*j+10,"f",GLOBAL_FRONTCOLOR);
			}
		}
	}
	if (Key==27) {
		GAME_TYPE=0;
	}
}

void Authors () {
	if (THE_END_COUNT>0) THE_END_COUNT--;
	DrawText (240,10+THE_END_COUNT,"the end",HeroColor);
	DrawText (10,40+THE_END_COUNT,"dev team",HeroColor);
	DrawText (10,60+THE_END_COUNT,"game director _ shimanskey eugene",GLOBAL_FRONTCOLOR);
	DrawText (10,80+THE_END_COUNT,"level designer _ chuduk alexander", GLOBAL_FRONTCOLOR);
	DrawText (10,100+THE_END_COUNT,"programmer _ shimanskey eugene", GLOBAL_FRONTCOLOR);
	DrawText (10,120+THE_END_COUNT,"font designer _ chuduk alexander", GLOBAL_FRONTCOLOR);
	DrawText (10,170+THE_END_COUNT,"this game is dedicated to our",GLOBAL_FRONTCOLOR);
	DrawText (10,190+THE_END_COUNT,"relatives and friends",GLOBAL_FRONTCOLOR);
	if (Key==27) {
		GAME_TYPE=0;
		THE_END_COUNT=600;
	}
}


int main(int argc, char **argv) {
	_ksys_get_screen_size (&ScreenX, &ScreenY);
	OffsetX=ScreenX/2-Width/2;
	OffsetY=ScreenY/2-Height/2;
	draw_window();

	while (!0) {
		if (GAME_TYPE==1) {
		_ksys_delay(15-Max_Speed);
		}
		else {
		_ksys_delay(1);
		}
		Key=getKey();
		if (GAME_TYPE==-1) return 0;
		if (GAME_TYPE==0) MainMenu ();
		if (GAME_TYPE==1) GamePlay ();
		if (GAME_TYPE==2) ShowHelp ();
		if (GAME_TYPE==3) LevelEditor();
		if (GAME_TYPE==4) Authors();
		Update();

	}
}