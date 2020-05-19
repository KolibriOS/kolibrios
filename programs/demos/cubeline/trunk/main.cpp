/*
ѕример вз€т из набора примеров к компил€тору XS Compiler

iadn
http://www.iadn.narod.ru
iadn@bk.ru
*/

#include <string.h>
#include<menuet/os.h>
#include <kosgl.h> //TinyGL

#include "SysCall.h"
#include "ProcessTab.h"


int Fps (long x, long y);
extern "C"{
int main(void);
}

static struct {
	   int x,y;
	   int dx,dy;
	   } win;

#define CUBE_STEP 0.1

#define KEY_ESC       1
#define KEY_F     	 33

static char title1[] = "TinyGL in KolibriOS";
static char title2[] = "F full screen";
static char title3[] = "ESC - exit";
static char fps[]    = "FPS:";

static unsigned char FullScreen = 0;
static unsigned char skin = 3;

static float angle;
static process_table_entry_* pri;
static KOSGLContext cgl;

static void draw_cube()
{
  float x,y,z;
  glBegin(GL_LINES);

    for(y=-0.5;y<=0.5;y+=CUBE_STEP)
    {
      // the front
      glColor3f(0,y+0.5,0);
      glVertex3f(-0.5,y,-0.5);
      glColor3f(1,y+0.5,0);
      glVertex3f(0.5,y,-0.5);

      // the back
      glColor3f(0,y+0.5,1);
      glVertex3f(-0.5,y,0.5);
      glColor3f(1,y+0.5,1);
      glVertex3f(0.5,y,0.5);

      //right side
      glColor3f(1,y+0.5,0);
      glVertex3f(0.5,y,-0.5);
      glColor3f(1,y+0.5,1);
      glVertex3f(0.5,y,0.5);

      //left side
      glColor3f(0,y+0.5,0);
      glVertex3f(-0.5,y,-0.5);
      glColor3f(0,y+0.5,1);
      glVertex3f(-0.5,y,0.5);
   }

   for(x=-0.5;x<=0.5;x+=CUBE_STEP)
   {
      // the front
      glColor3f(x+0.5,1,0);
      glVertex3f(x,0.5,-0.5);
      glColor3f(x+0.5,0,0);
      glVertex3f(x,-0.5,-0.5);
      
      // the back
      glColor3f(x+0.5,1,1);
      glVertex3f(x,0.5,0.5);
      glColor3f(x+0.5,0,1);
      glVertex3f(x,-0.5,0.5);

      // the top
      glColor3f(x+0.5,1,0);
      glVertex3f(x,0.5,-0.5);
      glColor3f(x+0.5,1,1);
      glVertex3f(x,0.5,0.5);

      // the bottom
      glColor3f(x+0.5,0,0);
      glVertex3f(x,-0.5,-0.5);
      glColor3f(x+0.5,0,1);
      glVertex3f(x,-0.5,0.5);
   }

   for(z=-0.5;z<=0.5;z+=CUBE_STEP)
   {
      // the top
      glColor3f(0,1,z+0.5);
      glVertex3f(-0.5,0.5,z);
      glColor3f(1,1,z+0.5);
      glVertex3f(0.5,0.5,z);

      // the bottom
      glColor3f(0,0,z+0.5);
      glVertex3f(-0.5,-0.5,z);
      glColor3f(1,0,z+0.5);
      glVertex3f(0.5,-0.5,z);

      // right side
      glColor3f(1,1,z+0.5);
      glVertex3f(0.5,0.5,z);
      glColor3f(1,0,z+0.5);
      glVertex3f(0.5,-0.5,z);

      // left side
      glColor3f(0,1,z+0.5);
      glVertex3f(-0.5,0.5,z);
      glColor3f(0,0,z+0.5);
      glVertex3f(-0.5,-0.5,z); 
   }

  glEnd();
}

static void DrawGL() 
{
  glLoadIdentity();                                                                               // устанавливаем еденичную матрицу
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);           

  glTranslatef(0.0, 0.0, -3.0);             
  glRotatef(angle, 1.0, 0.0, 0.0);   
  glRotatef(2.0*angle, 0.0, 1.0, 0.0);   
  glRotatef(3.0*angle, 0.0, 0.0, 1.0);

  draw_cube();

  kosglSwapBuffers(); 
 } 

static void reshape()
{
   __menuet__get_process_table((process_table_entry*)pri,-1);
   glViewport(0, 0, pri->winx_size, pri->winy_size-20);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(45.0, (GLfloat)pri->winx_size/pri->winy_size, 1.0, 300.0);
   glMatrixMode(GL_MODELVIEW);
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );     
} 

static void disabletgl()
{
	kosglDestroyContext(cgl); 
	delete pri;
}

static void Title()
{
     __menuet__write_text(300,8,0x10ffffff,fps,strlen(fps));
     __menuet__write_text(8,8,0x10ffffff,title1,strlen(title1));
	 __menuet__write_text(180,8,0x00ffffff,title2,strlen(title2));
     __menuet__write_text(600,8,0x00ffffff,title3,strlen(title3));
}

static void draw_window(void)
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

int main(void)
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
     
  pri=new process_table_entry_;
  SysCall(66,1,1);
  
  reshape();
  
do{

 if (angle < 360.0) angle += 0.001 + 0.1*Fps (330,8);
               else angle = 0.0;  
 
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
						  				 return 0;}
						  				 break;
						  			
			  	  case 3: disabletgl();
						  return 0;
		      }
}while(1);
}
