/*
iadn
http://www.iadn.narod.ru
iadn@bk.ru
*/
#include<menuet/os.h>
#include <kosgl.h> //TinyGL
#include <string.h>

#include "SysCall.h"
#include "ProcessTab.h"
#include "bmp.h"


int Fps (long x, long y);
extern "C"{
void app_main(void);
}

struct {
	   int x,y;
	   int dx,dy;
	   } win;

#define KEY_ESC       1
#define KEY_F     	 33

char *title1 = "TinyGL in KolibriOS";
char *title2 = "F full screen";
char *title3 = "ESC - exit";
char *fps    = "FPS:";

unsigned char FullScreen = 0;
unsigned char skin = 3;

static GLuint* TexObj;

float angle = 0.0;
process_table_entry_* pri;
KOSGLContext cgl;

struct V3{
	   float v1;
	   float v2;
	   float v3;
	   } ptrv[8] = {{-1.0,1.0,1.0},
	  		        {-1.0,-1.0,1.0},
				  	{1.0,-1.0,1.0},
				  	{1.0,1.0,1.0},
				  	{-1.0,1.0,-1.0},
				  	{-1.0,-1.0,-1.0},
				  	{1.0,-1.0,-1.0},
                  	{1.0,1.0,-1.0}};

struct T2{
	   float t1;
	   float t2;	   
	   } ptrt[4] = {
	   	 		    {0.0, 0.0},
	                {1.0, 0.0},
	                {1.0, 1.0},
					{0.0, 1.0}
					};                

void DrawQUADS(V3* ptr, int iv1, int iv2, int iv3, int iv4, T2* ptrt, int it1, int it2, int it3, int it4)
{ 	   
 	  glBegin(GL_QUADS);
 	  glTexCoord2fv((float*)&ptrt[it1]);
 	  glVertex3fv((float*)&ptr[iv1]);
 	  glTexCoord2fv((float*)&ptrt[it2]);
 	  glVertex3fv((float*)&ptr[iv2]);
 	  glTexCoord2fv((float*)&ptrt[it3]);
	  glVertex3fv((float*)&ptr[iv3]);
	  glTexCoord2fv((float*)&ptrt[it4]);
 	  glVertex3fv((float*)&ptr[iv4]); 
 	  glEnd();
}
void DrawGL() 
{
  glLoadIdentity();                                                                               // устанавливаем еденичную матрицу
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);           

  glTranslatef(0.0, 0.0, -6.0);             
  glRotatef(angle, 1.0, 0.0, 0.0);   
  glRotatef(2.0*angle, 0.0, 1.0, 0.0);   
  glRotatef(3.0*angle, 0.0, 0.0, 1.0);

  DrawQUADS((V3*)&ptrv,0,1,2,3,(T2*)&ptrt,3,0,1,2);
  DrawQUADS((V3*)&ptrv,0,3,7,4,(T2*)&ptrt,1,2,3,0);
  DrawQUADS((V3*)&ptrv,4,7,6,5,(T2*)&ptrt,2,3,0,1);
  DrawQUADS((V3*)&ptrv,5,6,2,1,(T2*)&ptrt,3,0,1,2);
  DrawQUADS((V3*)&ptrv,7,3,2,6,(T2*)&ptrt,3,0,1,2);
  DrawQUADS((V3*)&ptrv,5,1,0,4,(T2*)&ptrt,3,0,1,2);

  kosglSwapBuffers(); 
 } 

void reshape()
{
   __menuet__get_process_table((process_table_entry*)pri,-1);
   glViewport(0, 0, pri->winx_size, pri->winy_size-20);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(50.0, (GLfloat)pri->winx_size/pri->winy_size, 1.0, 300.0);
   glMatrixMode(GL_MODELVIEW);
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );     
} 

void disabletgl()
{
	kosglDestroyContext(cgl); 
	delete pri;
}

void Title()
{
     __menuet__write_text(300,8,0x10ffffff,fps,strlen(fps));
     __menuet__write_text(8,8,0x10ffffff,title1,strlen(title1));
	 __menuet__write_text(180,8,0x00ffffff,title2,strlen(title2));
     __menuet__write_text(600,8,0x00ffffff,title3,strlen(title3));
}

void draw_window(void)
{
	// start redraw
	__menuet__window_redraw(1);
	// define&draw window
	__menuet__define_window(win.x,win.y,win.dx,win.dy,TYPEWIN(0,0,0,1,skin,0,0,0),0,0);
    // end redraw
    __menuet__window_redraw(2);
    // display string
    Title();
}

void app_main(void)          
{ 

  win.x = 100;
  win.y = 100;
  win.dx = 400;
  win.dy = 400;

  draw_window();

  cgl = kosglCreateContext( 0, 0);
  kosglMakeCurrent( 0, 20, win.dx, win.dy-20, cgl);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearDepth(1.0);
  glEnable( GL_CULL_FACE );
  glEnable(GL_DEPTH_TEST);

  
  int width, height;
  unsigned char* texture;
  LoadFileBMP( "./logio.bmp", &texture, &width, &height, false );
  
  /* Setup texturing */
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  
  /* generate texture object IDs */
  glGenTextures(1, TexObj);
  glBindTexture(GL_TEXTURE_2D, *TexObj);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0,  GL_RGB, GL_UNSIGNED_BYTE, texture);
    
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, *TexObj);
  glEnable(GL_TEXTURE_2D);
     
  pri=new process_table_entry_;
  SysCall(66,1,1);
    
  reshape();
  
do{

 angle += 0.001 + 0.1*Fps (330,8); 
 
 DrawGL();
		
		  switch(__menuet__check_for_event())
		      {
		          case 1: draw_window();				  	   	  
				  	   	  reshape();
				  	   	  break;
				  	   	  
		          case 2: 		          
		          	   switch(__menuet__getkey()){

						   case KEY_F:
                                    if(!FullScreen){									 
									 skin=0;
									 SysCall(67,0,0,SysCall(14)>>16,SysCall(14)&0xffff);
									 draw_window();
									 reshape();
									 FullScreen = 1;
									}
									else{
									 skin=3;
									 draw_window();
									 SysCall(67,win.x,win.y,win.dx,win.dy);
									 reshape();
									 FullScreen = 0;
									};
						  			break;
		          
                           case KEY_ESC: disabletgl();
						  				 return;}
						  				 break;
						  			
			  	  case 3: disabletgl();
						  return;
		      }
}while(1);
}

	 
