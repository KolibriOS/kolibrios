/*
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * This is a port of the infamous "glxgears" demo to straight EGL
 * Port by Dane Rushton 10 July 2005
 *
 * No command line options.
 * Program runs for 5 seconds then exits, outputing framerate to console
 */

#include <math.h>
#include <sys/time.h>
#include <GL/gl.h>
#include <GL/glu.h>

#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif

#include <EGL/egl.h>
#include <kos32sys.h>
#include "eglut.h"

void init(void);
void reshape(int width, int height);
void draw(void);
void keyboard(unsigned char key);

extern GLfloat angle;

void idle(void)
{
    static uint32_t t0;
    uint32_t t, dt;

    t = get_tick_count();

    dt = t - t0;
    t0 = t;

    angle += 70.0 * dt / 100;  /* 70 degrees per second */
    if (angle > 3600.0)
        angle -= 3600.0;

    eglutPostRedisplay();
}

int main(int argc, char *argv[])
{
   eglutInitWindowSize(384, 384);
   eglutInitAPIMask(EGLUT_OPENGL_BIT);
   eglutInit(argc, argv);

   eglutCreateWindow("eglgears");

   eglutIdleFunc(idle);
   eglutReshapeFunc(reshape);
   eglutDisplayFunc(draw);
   eglutKeyboardFunc(keyboard);

   glClearColor( 0, 0, 0, 1);

   init();
   glDrawBuffer(GL_BACK);

   eglutMainLoop();

   return 0;
}

