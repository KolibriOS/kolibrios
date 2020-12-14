#include <kolibri.h>
#include <kos_heap.h>
#include <load_lib.h>
#include <l_tinygl.h>

using namespace Kolibri;

const char header[] = "Test tinygl library, [<-] and [->] - rotate";
char library_path[2048];

namespace Kolibri{
	char CurrentDirectoryPath[2048];
}

TinyGLContext ctx1;
float angle_z = 0.0, delt_size = 3.0;

void draw_3d()
{
	glClear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT); //очистим буфер цвета и глубины

	glPushMatrix();
	glRotatef(angle_z,0.0,0.0,1.0);

	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_TRIANGLES);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0.0,   0.5,   0.1);
	glVertex3f(0.475, 0.823, 0.1);
	glVertex3f(0.433, 0.25,  0.1);

	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0.5,   0.0,   0.1);
	glVertex3f(0.823,-0.475, 0.1);
	glVertex3f(0.25, -0.433, 0.1);

	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0.0,  -0.5,   0.1);
	glVertex3f(-0.475,-0.823,0.1);
	glVertex3f(-0.433,-0.25, 0.1);

	glVertex3f(-0.5,   0.0,   0.1);
	glColor3f(1.0, 1.0, 0.0);
	glVertex3f(-0.823, 0.475, 0.1);
	glColor3f(1.0, 1.0, 1.0);
	glVertex3f(-0.25,  0.433, 0.1);
	glEnd();

	glPopMatrix();
}

bool KolibriOnStart(TStartData &kos_start, TThreadData /*th*/)
{
	kos_start.Left = 10;
	kos_start.Top = 40;
	kos_start.Width = 330;
	kos_start.Height = 275;
	kos_start.WinData.WindowColor = 0xd0d0d0;
	kos_start.WinData.WindowType = 0x33; // 0x34 - fixed, 0x33 - not fixed
	kos_start.WinData.Title = header;
	if(LoadLibrary("tinygl.obj", library_path, "/sys/lib/tinygl.obj", &import_tinygl))
	{
		kosglMakeCurrent(0,0,kos_start.Width,kos_start.Height,&ctx1);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.2,0.0,0.2,0.0);
		draw_3d();
		return true;
	} 
	else return false;
}

void KolibriOnPaint(void)
{
	kosglSwapBuffers();
}

void KolibriOnKeyPress(TThreadData /*th*/)
{
	long key = GetKey();
	switch(key){
	case 176: //Left
		angle_z+=delt_size;
		draw_3d();
		kosglSwapBuffers();
		break;
	case 179: //Right
		angle_z-=delt_size;
		draw_3d();
		kosglSwapBuffers();
		//break;
	};
}
