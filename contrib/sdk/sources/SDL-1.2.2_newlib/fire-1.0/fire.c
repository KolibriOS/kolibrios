/* Fireworks demo written by Dave Ashley */
/* dash@xdr.com */
/* http://www.xdr.com/dash */
/* Sat Jun 13 02:46:09 PDT 1998 */
/* This is my first attempt at an SDL program */
/* See the SDL home page http://www.devolution.com/~slouken/projects/SDL/ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "SDL.h"

#define XSIZE 640
#define YSIZE 480

SDL_Surface *thescreen;
unsigned char *vmem1, *vmem2;
int mousex,mousey;
SDL_Color themap[256];

int scrlock()
{
	if(SDL_MUSTLOCK(thescreen))
	{
		if ( SDL_LockSurface(thescreen) < 0 )
		{
			SDL_printf("Couldn't lock display surface: %s\n",
								SDL_GetError());
			return -1;
		}
	}
	return 0;
}
void scrunlock(void)
{
	if(SDL_MUSTLOCK(thescreen))
		SDL_UnlockSurface(thescreen);
	SDL_UpdateRect(thescreen, 0, 0, 0, 0);
}

#define MOUSEFRAC 2
#define MAXBLOBS 512
#define BLOBFRAC 6
#define BLOBGRAVITY 5
#define THRESHOLD 20
#define SMALLSIZE 3
#define BIGSIZE 6

#define ABS(x) ((x)<0 ? -(x) : (x))

int explodenum;

char sizes[]={2,3,4,5,8,5,4,3};


struct blob {
	struct blob *blobnext;
	int blobx;
	int bloby;
	int blobdx;
	int blobdy;
	int bloblife;
	int blobsize;
} *blobs,*freeblobs,*activeblobs;


unsigned char **mul640;
int oldmode;

char sqrttab[]={
0,1,1,1,2,2,2,2,2,3,3,3,3,3,3,3,
4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,
5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,
6,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
8,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,
10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,
11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,
12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
12,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,
13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
13,13,13,13,14,14,14,14,14,14,14,14,14,14,14,14,
14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,
14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15
};


void nomem(void)
{
	SDL_printf("Not enough low memory!\n");
	SDL_Quit();
	exit(1);
}



void fire(unsigned char *p1,unsigned char *p2,int pitch,char *map)
{
int x,y;
unsigned char *p3, *p4;

	for(y=2;y<YSIZE;y++)
	{
		for(x=0;x<XSIZE;x++)
		{
			p3 = p1+y*XSIZE+x;
			p4 = p2+y*pitch+x;
			*p4=map[*p3+p3[-XSIZE]+p3[-XSIZE-1]+p3[-XSIZE+1]+p3[-1]+p3[1]+p3[-XSIZE-XSIZE-1]+p3[-XSIZE-XSIZE]+p3[-XSIZE-XSIZE+1]];
		}
	}
}

void disk(x,y,rad)
{
unsigned char *p;
int i,j,k,aj;
int rad2=rad*rad;
int w;


	for(j=-rad;j<=rad;j++)
	{
		w=sqrttab[rad2-j*j];
		aj=ABS(j)<<2;
		if(w)
		{
			p=mul640[y+j]+x-w;
			k=w+w+1;
			i=-w;
			while(k--) {*p++=255-(ABS(i)<<2)-aj;i++;}
		}
	}
}
void trydisk(void)
{
	if(mousex>10 && mousex<XSIZE-10 && mousey>10 && mousey<YSIZE-10)
		disk(mousex,mousey,8);
}

void addblob(void)
{
int dx,dy;
struct blob *ablob;

	if(!freeblobs) return;
	dx=(rand()&255)-128;
	dy=(rand()%200)+340;
	ablob=freeblobs;
	freeblobs=freeblobs->blobnext;
	ablob->bloblife=(rand()&511)+256;
	ablob->blobdx=dx;
	ablob->blobdy=dy;
	ablob->blobx=(256+(rand()&127))<<BLOBFRAC;
	ablob->bloby=2<<BLOBFRAC;
	ablob->blobnext=activeblobs;
	ablob->blobsize=BIGSIZE;
	activeblobs=ablob;
}
void moveblobs(void)
{
struct blob **lastblob,*ablob;
int x,y;

	lastblob=&activeblobs;
	while(ablob=*lastblob)
	{
		x=ablob->blobx>>BLOBFRAC;
		y=ablob->bloby>>BLOBFRAC;
		if(!--ablob->bloblife || y<0 || x<10 || x>XSIZE-10)
		{
			*lastblob=ablob->blobnext;
			ablob->blobnext=freeblobs;
			freeblobs=ablob;
			continue;
		}
		ablob->blobx+=ablob->blobdx;
		ablob->bloby+=ablob->blobdy;
		ablob->blobdy-=BLOBGRAVITY;
		lastblob=&ablob->blobnext;
	}
}
void putblobs(void)
{
struct blob *ablob,*ablob2,*temp;
int x,y,dy;
int i,size;
long x2,y2,vel;

	ablob=activeblobs;
	activeblobs=0;
	while(ablob)
	{
		dy=ablob->blobdy;
		if(ablob->blobsize!=SMALLSIZE && (dy>-THRESHOLD && dy<THRESHOLD && !(rand()&7) || (rand()&127)==63))
		{
			i=explodenum;
			while(i-- && freeblobs)
			{
				ablob2=freeblobs;
				freeblobs=freeblobs->blobnext;
				ablob2->blobx=ablob->blobx;
				ablob2->bloby=ablob->bloby;
				for(;;)
				{
					x2=(rand()&511)-256;
					y2=(rand()&511)-256;
					vel=x2*x2+y2*y2;
					if(vel>0x3000 && vel<0x10000L) break;
				}
				ablob2->blobdx=ablob->blobdx+x2;
				ablob2->blobdy=ablob->blobdy+y2;
				ablob2->bloblife=16+(rand()&31);
				ablob2->blobsize=SMALLSIZE;
				ablob2->blobnext=activeblobs;
				activeblobs=ablob2;
				ablob->bloblife=1;
			}			
		}
		x=ablob->blobx>>BLOBFRAC;
		y=ablob->bloby>>BLOBFRAC;
		size=ablob->blobsize;
		if(size==BIGSIZE && ablob->blobdy>0 && ablob->blobdy<200)
			size=sizes[ablob->bloblife&7];
		if(x>10 && x<XSIZE-10 && y>10 && y<YSIZE-10)
			disk(x,YSIZE-1-y,size);
		temp=ablob;
		ablob=ablob->blobnext;
		temp->blobnext=activeblobs;
		activeblobs=temp;
	}
}



#define RATE 1
void normal(char *map)
{
int i,j;
	for(i=0;i<8192;i++)
	{
		j=i/9;
		map[i]=j<256 ? (j>=RATE ? j-RATE : 0) : 255;
	}
}
void bright(char *map)
{
int i;
	for(i=0;i<8192;i++) map[i]=i>>3<255 ? (i>>3) : 255;
}

void updatemap(void)
{
	SDL_SetColors(thescreen, themap, 0, 256);
}


void loadcolor(int n,int r,int g,int b)
{
	themap[n].r=r<<2;
	themap[n].g=g<<2;
	themap[n].b=b<<2;
}


void loadcolors(unsigned int which)
{
int i,j;
int r,g,b;

	which%=11;
	for(i=0;i<256;i++)
	{
		switch(which)
		{
		case 0:
			if(i<64) loadcolor(i,0,0,0);
			else if(i<128)	loadcolor(i,i-64,0,0);
			else if(i<192) loadcolor(i,63,i-128,0);
			else loadcolor(i,63,63,i-192);
			break;
		case 1:
			if(i<64) loadcolor(i,0,0,0);
			else if(i<128)	loadcolor(i,0,0,i-64);
			else loadcolor(i,(i-128)>>1,(i-128)>>1,63);
			break;
		case 2:
			loadcolor(i,i>>2,i>>2,i>>2);
			break;
		case 3:
			r=rand()&0x3f;
			g=rand()&0x3f;
			b=rand()&0x3f;
			loadcolor(i,r*i>>8,g*i>>8,b*i>>8);
			break;
		case 4:
			loadcolor(i,i>>2,0,0);
			break;
		case 5:
			loadcolor(i,0,i>>2,0);
			break;
		case 6:
			loadcolor(i,0,0,i>>2);
			break;
		case 7:
			j=i&15;
			if(i&16) j=15-j;
			j=(i>>2)*j/16;
			loadcolor(i,j,j,j);
			break;
		case 8:
			j=0;
			if(i>8 && i<128) j=63;
			loadcolor(i,j,j,j);
			break;
		case 9:
			j=31-(i&31)<<1;
			r=i&32 ? j : 0;
			g=i&64 ? j : 0;
			b=i&128 ? j : 0;
			loadcolor(i,r,g,b);
			break;
		case 10:
			j=(i&15)<<2;
			if(i&16) j=63-j;
			r=i&32 ? j : 0;
			g=i&64 ? j : 0;
			b=i&128 ? j : 0;
			loadcolor(i,r,g,b);
			break;
		}
	}
	updatemap();
}

int main(int argc, char *argv[])
{
int i,k;
char *remap,*remap2;
unsigned char *p1, *p2;
long frames;
int flash;
int whichmap;
int key;
int ispaused;
unsigned long videoflags;
int done;
int now;
SDL_Event event;
long starttime;
int buttonstate;

	srand(time(NULL));
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
	{
		SDL_printf("Couldn't initialize SDL: %s\n",SDL_GetError());
		exit(1);
	}
	videoflags = SDL_SWSURFACE;

	thescreen = SDL_SetVideoMode(XSIZE, YSIZE, 8, videoflags);
	if ( thescreen == NULL )
	{
		SDL_printf("Couldn't set display mode: %s\n",
							SDL_GetError());
		SDL_Quit();
		exit(5);
	}

	vmem1=NULL;
	vmem2=malloc(XSIZE*YSIZE);
	if(!vmem2) nomem();
	mul640=malloc(YSIZE*sizeof(char *));
	if(!mul640) nomem();
	remap=malloc(16384);
	if(!remap) nomem();
	remap2=malloc(16384);
	if(!remap2) nomem();
	blobs=malloc(MAXBLOBS*sizeof(struct blob));
	if(!blobs) nomem();

	SDL_printf("Fire demo by David Ashley (dash@xdr.com)");
	SDL_printf("1 = Change color map");
	SDL_printf("2 = Randomly change color map");
	SDL_printf("p = Pause");
	SDL_printf("spc = Fire");
	SDL_printf("esc = Exit");
	SDL_printf("Left mouse button = paint");
	SDL_printf("Right mouse button, CR = ignite atmosphere");

	freeblobs=activeblobs=0;
	for(i=0;i<MAXBLOBS;i++)
	{
		blobs[i].blobnext=freeblobs;
		freeblobs=blobs+i;
	}

	normal(remap);
	bright(remap2);


	flash=0;
	whichmap=0;
	loadcolors(whichmap);
	frames=0;
	ispaused=0;
	addblob();
	done = 0;
	now=0;
	starttime=SDL_GetTicks();
	buttonstate=0;
	mousex=mousey=0;

	while(!done)
	{
		if ( scrlock() < 0 ) continue;
		frames++;
		if ( vmem1 != (unsigned char *)thescreen->pixels )
		{
			p1=vmem1=thescreen->pixels;
			for (i=0;i<YSIZE;i++)
			{
				mul640[i]=i*thescreen->pitch+vmem1;
				memset(p1,0,XSIZE);
				p1+=thescreen->pitch;
			}
		}
		if(!ispaused)
		{
			now++;
			if(!flash)
			{
				if(explodenum>96 && explodenum<160 && !(rand()&511) || (buttonstate&8))
					flash=60;
			} else --flash;
			explodenum=(now>>4)+1;if(explodenum==320) now=0;
			if(explodenum>256) explodenum=256;
			if(!(rand()&31))
				addblob();
			moveblobs();
			putblobs();
			if(buttonstate&2) trydisk();
			p1=vmem1;
			p2=vmem2;
			k=thescreen->pitch;
			for(i=0;i<YSIZE;i++)
			{
				memcpy(p2,p1,XSIZE);
				p2+=XSIZE;
				p1+=k;
			}
			fire(vmem2,vmem1,k,flash ? remap2 :remap);
		}
		scrunlock();

		while(SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if ( event.button.state == SDL_PRESSED )
					buttonstate|=1<<event.button.button;
				else
					buttonstate&=~(1<<event.button.button);
				mousex=event.button.x;
				mousey=event.button.y;
				if(!ispaused && buttonstate&2) trydisk();
				break;
			case SDL_MOUSEMOTION:
				mousex=event.motion.x;
				mousey=event.motion.y;
				if(!ispaused && buttonstate&2) trydisk();
				break;
			case SDL_KEYDOWN:
				key=event.key.keysym.sym;
				if(key==SDLK_RETURN) {flash=60;break;}
				if(key==SDLK_1 || key==SDLK_2)
				{
					if(key==SDLK_1)
						++whichmap;
					else
						whichmap=rand();
					loadcolors(whichmap);
					break;
				}
				if(key==SDLK_ESCAPE) {done=1;break;}
				if(key==SDLK_SPACE && !ispaused) {addblob();break;}
				if(key==SDLK_p) {ispaused=!ispaused;break;}
				break;
			case SDL_QUIT:
				done = 1;
				break;
			default:
				break;
			}
		}
	}

	starttime=SDL_GetTicks()-starttime;
	if(!starttime) starttime=1;
	SDL_Quit();
	SDL_printf("fps = %d\n",1000*frames/starttime);
	exit(0);
}
