#include "text.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PHL.h"
#include "game.h"

char gameLanguage;
Message* saving;
Message* saveError[3];
Message* itemName[41];
Message* found;
Message* itemDescription[28];
Message* dungeon[8];

void loadMessage(Message* m, FILE* f);
void trimMessage(Message* m);

void textInit()
{
	gameLanguage = JAPANESE;
	
	saving = (Message*)malloc(sizeof(Message));
	
	int i;
	for (i = 0; i < 3; i++) {
		saveError[i] = (Message*)malloc(sizeof(Message));
	}
	
	for (i = 0; i < 8; i++) {
		dungeon[i] = (Message*)malloc(sizeof(Message));
	}
	
	for (i = 0; i < 41; i++) {
		itemName[i] = (Message*)malloc(sizeof(Message));
	}
	
	found = (Message*)malloc(sizeof(Message));
		
	for (i = 0; i < 28; i++) {
		itemDescription[i] = (Message*)malloc(sizeof(Message));
	}
	
	//printf("\nText init");
}

void textFree()
{
	free(saving);
	
	int i;
	for (i = 0; i < 3; i++) {
		free(saveError[i]);
	}
	
	for (i = 0; i < 8; i++) {
		free(dungeon[i]);
	}
	
	for (i = 0; i < 41; i++) {
		free(itemName[i]);
	}
	
	free(found);
		
	for (i = 0; i < 28; i++) {
		free(itemDescription[i]);
	}
}

void loadText()
{
	FILE* f;
	
	char fullPath[30];
	#ifdef _3DS
		strcpy(fullPath, "romfs:/");
	#elif defined(_SDL)
	strcpy(fullPath, "data/");
	#else
		strcpy(fullPath, "romfs/");
	#endif
	
	if (gameLanguage == ENGLISH) {
		strcat(fullPath, "en.dat");
	}else{
		strcat(fullPath, "jp.dat");
	}
	
	//printf("\n");
	//printf(fullPath);
	
	if ( (f = fopen(fullPath, "rb")) )
	{
		//printf("\ntext.dat found");
		
		//Load saving message		
		loadMessage(saving, f);
		
		//printf("\n%d", saving->length);
		
		//Load save error message
		int i;
		for (i = 0; i < 3; i++) {
			loadMessage(saveError[i], f);
		}
		
		//Load dungeon intros
		for (i = 0; i < 8; i++) {
			loadMessage(dungeon[i], f);
		}
		
		//Load item names
		for (i = 0; i < 41; i++) {
			loadMessage(itemName[i], f);
		}
		
		//Found!
		loadMessage(found, f);
		
		//Load item descriptions
		for (i = 0; i < 28; i++) {
			loadMessage(itemDescription[i], f);
		}
		
	}else{
		//printf("\ntext.dat was not found");
	}
	
	fclose(f);
}

//Returns the next character X position
int drawText(Message* m, int x, int y)
{
	int textw = 20;
	int texth = 32;
	
	int dx = x;
	
	int i;
	for (i = 0; i < m->length; i++) {
		PHL_DrawSurfacePart(dx, y, m->x[i] * textw, m->y[i] * texth, textw, texth, images[imgFontKana]);
		dx += textw - 2;
	}
	
	return dx;
}

int drawCharacter(int cx, int cy, int x, int y)
{
	int textw = 20;
	int texth = 32;
	
	PHL_DrawSurfacePart(x, y, cx * textw, cy * texth, textw, texth, images[imgFontKana]);
	
	return x + textw - 2;
}

void drawTextCentered(Message* m, int x, int y)
{
	int textw = 20;
	int texth = 32;
	
	x -= (m->length / 2) * (textw - 2);
	
	int i;
	for (i = 0; i < m->length; i++) {
		PHL_DrawSurfacePart(x + (i * (textw - 2)), y, m->x[i] * textw, m->y[i] * texth, textw, texth, images[imgFontKana]);
	}
}

void setLanguage(char lan)
{
	gameLanguage = lan;
	loadText();
}

char getLanguage()
{
	return gameLanguage;
}

void loadMessage(Message* m, FILE* f)
{
	unsigned char* buffer = (unsigned char*)malloc(sizeof(unsigned char) * 64);
	
	int cnt = fread(buffer, 1, 64, f);
	
	int i;
	for (i = 0; i < cnt; i+=2) {
		m->x[i/2] = buffer[i];
		m->y[i/2] = buffer[i+1];
	}
	trimMessage(m);
	
	free(buffer);
}

void trimMessage(Message* m)
{
	m->length = 32;
	
	int i;	
	for (i = 31; i >= 0; i--)
	{
		if (m->x[i] == 0 && m->y[i] == 0) {
			m->length -= 1;
		}else{
			i = -1;
		}
	}
}