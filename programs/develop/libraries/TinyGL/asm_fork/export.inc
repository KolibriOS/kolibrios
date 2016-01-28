;
; Export functions (103)
;

E_LIB glEnable ;(int code)
E_LIB glDisable ;(int code)

E_LIB glShadeModel ;(int mode)
E_LIB glCullFace ;(int mode)
E_LIB glPolygonMode ;(int face,int mode)

E_LIB glBegin ;(int type)
E_LIB glEnd ;(void)

E_LIB glVertex2f ;(float ,float)
E_LIB glVertex2d ;(double ,double)
E_LIB glVertex2fv ;(float *)
E_LIB glVertex2dv ;(double *)
E_LIB glVertex3f ;(float ,float ,float)
E_LIB glVertex3d ;(double ,double ,double)
E_LIB glVertex3fv ;(float *)
E_LIB glVertex3dv ;(double *)
E_LIB glVertex4f ;(float ,float ,float, float )
E_LIB glVertex4d ;(double ,double ,double, double )
E_LIB glVertex4fv ;(float *)
E_LIB glVertex4dv ;(double *)

E_LIB glColor3f ;(float ,float ,float)
E_LIB glColor3d ;(double ,double ,double)
E_LIB glColor3fv ;(float *)
E_LIB glColor3dv ;(double *)
E_LIB glColor3ub
E_LIB glColor4f ;(float ,float ,float, float )
E_LIB glColor4d ;(double ,double ,double, double )
E_LIB glColor4fv ;(float *)
E_LIB glColor4dv ;(double *)

E_LIB glNormal3f ;(float ,float ,float)
E_LIB glNormal3d ;(double ,double ,double)
E_LIB glNormal3fv ;(float *)
E_LIB glNormal3dv ;(double *)

E_LIB glTexCoord1f ;(float)
E_LIB glTexCoord1d ;(double)
E_LIB glTexCoord1fv ;(float *)
E_LIB glTexCoord1dv ;(double *)
E_LIB glTexCoord2f ;(float ,float)
E_LIB glTexCoord2d ;(double ,double)
E_LIB glTexCoord2fv ;(float *)
E_LIB glTexCoord2dv ;(double *)
E_LIB glTexCoord3f ;(float ,float ,float)
E_LIB glTexCoord3d ;(double ,double ,double)
E_LIB glTexCoord3fv ;(float *)
E_LIB glTexCoord3dv ;(double *)
E_LIB glTexCoord4f ;(float ,float ,float, float )
E_LIB glTexCoord4d ;(double ,double ,double, double )
E_LIB glTexCoord4fv ;(float *)
E_LIB glTexCoord4dv ;(double *)

E_LIB glEdgeFlag ;(int flag)

; matrix
E_LIB glMatrixMode ;(int mode)
E_LIB glLoadMatrixf ;(const float *m)
E_LIB glLoadIdentity ;(void)
E_LIB glMultMatrixf ;(const float *m)
E_LIB glPushMatrix ;(void)
E_LIB glPopMatrix ;(void)
E_LIB glRotatef ;(float angle,float x,float y,float z)
E_LIB glTranslatef ;(float x,float y,float z)
E_LIB glScalef ;(float x,float y,float z)

E_LIB glViewport ;(int x,int y,int width,int height)
E_LIB glFrustum ;(double left,double right,double bottom,double top, double near_,double far_);

; lists
E_LIB glGenLists ;(int range)
E_LIB glIsList ;(unsigned int list)
E_LIB glNewList ;(unsigned int list,int mode)
E_LIB glEndList ;(void)
E_LIB glCallList ;(unsigned int list)

; clear
E_LIB glClear ;(int mask)
E_LIB glClearColor ;(float r,float g,float b,float a)
E_LIB glClearDepth ;(double depth)

; selection
E_LIB glRenderMode ;(int mode)
E_LIB glSelectBuffer ;(int size,unsigned int *buf)

E_LIB glInitNames ;(void)
E_LIB glPushName ;(unsigned int name)
E_LIB glPopName ;(void)
E_LIB glLoadName ;(unsigned int name)

; textures
E_LIB glGenTextures ;(int n, unsigned int *textures)
E_LIB glDeleteTextures ;(int n, const unsigned int *textures)
E_LIB glBindTexture ;(int target,int texture)
E_LIB glTexImage2D ;( int target, int level, int components, int width, int height, int border, int format, int type, void *pixels)
E_LIB glTexEnvi ;(int target,int pname,int param)
E_LIB glTexParameteri ;(int target,int pname,int param)
E_LIB glPixelStorei ;(int pname,int param)

; lighting
E_LIB glMaterialfv ;(int mode,int type,float *v)
E_LIB glMaterialf ;(int mode,int type,float v)
E_LIB glColorMaterial ;(int mode,int type)

E_LIB glLightfv ;(int light,int type,float *v)
E_LIB glLightf ;(int light,int type,float v)
E_LIB glLightModeli ;(int pname,int param)
E_LIB glLightModelfv ;(int pname,float *param)

; misc
E_LIB glFlush ;(void)
E_LIB glHint ;(int target,int mode)
E_LIB glGetIntegerv ;(int pname,int *params)
E_LIB glGetFloatv ;(int pname, float *v)
E_LIB glFrontFace ;(int mode)

; opengl 1.2 arrays
E_LIB glEnableClientState ;(GLenum array)
E_LIB glDisableClientState ;(GLenum array)
E_LIB glArrayElement ;(GLint i)
E_LIB glDrawArrays ;(mode, first, count)
E_LIB glDrawElements ;(mode, count, type, indices)
E_LIB glVertexPointer ;(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
E_LIB glColorPointer ;(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
E_LIB glNormalPointer ;(GLenum type, GLsizei stride, const GLvoid *pointer)
E_LIB glTexCoordPointer ;(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)

; opengl 1.2 polygon offset
E_LIB glPolygonOffset ;(GLfloat factor, GLfloat units)

; not implemented, just added to compile
;inline void glPointSize(float) {}
;inline void glLineWidth(float) {}
;inline void glDeleteLists(int, int) {}
;inline void glDepthFunc(int) {}
;inline void glBlendFunc(int, int) {}
;inline void glTexEnvf(int, int, int) {}
;inline void glOrtho(float,float,float,float,float,float){}
;inline void glVertex2i(int,int) {}
;inline void glDepthMask(int) {}
;inline void glFogi(int, int) {}
;inline void glFogfv(int, const float*) {}
;inline void glFogf(int, float) {}
;inline void glRasterPos2f(float, float) {}
;inline void glPolygonStipple(void*) {}
;inline void glTexParameterf(int, int, int) {};
; non compatible functions

E_LIB glDebug ;(int mode)

E_LIB glInit ;(void *zbuffer)
E_LIB glClose ;(void)

;
; glu functions
;

E_LIB gluNewQuadric
E_LIB gluDeleteQuadric
E_LIB gluQuadricDrawStyle
E_LIB gluQuadricOrientation
E_LIB gluQuadricTexture
E_LIB gluSphere

;
; Kolibri OS functions
;

E_LIB kosglMakeCurrent
E_LIB kosglSwapBuffers

purge E_LIB
