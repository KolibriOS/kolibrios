;
; Export functions (115)
;

E_LIB glEnable ;(int)
E_LIB glDisable ;(int)

E_LIB glShadeModel ;(int)
E_LIB glCullFace ;(int)
E_LIB glPolygonMode ;(int, int)

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
E_LIB glVertex4f ;(float ,float ,float, float)
E_LIB glVertex4d ;(double ,double ,double, double)
E_LIB glVertex4fv ;(float *)
E_LIB glVertex4dv ;(double *)

E_LIB glColor3f ;(float ,float ,float)
E_LIB glColor3d ;(double ,double ,double)
E_LIB glColor3fv ;(float *)
E_LIB glColor3dv ;(double *)
E_LIB glColor3ub
E_LIB glColor4f ;(float ,float ,float, float)
E_LIB glColor4d ;(double ,double ,double, double)
E_LIB glColor4fv ;(float *)
E_LIB glColor4dv ;(double *)

E_LIB glNormal3f ;(float, float, float)
E_LIB glNormal3d ;(double, double, double)
E_LIB glNormal3fv ;(float *)
E_LIB glNormal3dv ;(double *)

E_LIB glTexCoord1f ;(float)
E_LIB glTexCoord1d ;(double)
E_LIB glTexCoord1fv ;(float *)
E_LIB glTexCoord1dv ;(double *)
E_LIB glTexCoord2f ;(float, float)
E_LIB glTexCoord2d ;(double, double)
E_LIB glTexCoord2fv ;(float *)
E_LIB glTexCoord2dv ;(double *)
E_LIB glTexCoord3f ;(float, float, float)
E_LIB glTexCoord3d ;(double, double, double)
E_LIB glTexCoord3fv ;(float *)
E_LIB glTexCoord3dv ;(double *)
E_LIB glTexCoord4f ;(float, float, float, float)
E_LIB glTexCoord4d ;(double, double, double, double)
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
E_LIB glRotatef ;(float, float, float, float)
E_LIB glTranslatef ;(float, float, float)
E_LIB glScalef ;(float, float, float)

E_LIB glViewport ;(int, int, int, int)
E_LIB glFrustum ;(double, double, double, double, double, double)

; lists
E_LIB glGenLists ;(int)
E_LIB glIsList ;(unsigned int)
E_LIB glNewList ;(unsigned int, int)
E_LIB glEndList ;(void)
E_LIB glCallList ;(unsigned int)

; clear
E_LIB glClear ;(int)
E_LIB glClearColor ;(float, float, float, float)
E_LIB glClearDepth ;(double)

; selection
E_LIB glRenderMode ;(int)
E_LIB glSelectBuffer ;(int, unsigned int *)

E_LIB glInitNames ;(void)
E_LIB glPushName ;(unsigned int)
E_LIB glPopName ;(void)
E_LIB glLoadName ;(unsigned int)

; textures
E_LIB glGenTextures ;(int, unsigned int *)
E_LIB glDeleteTextures ;(int, const unsigned int *)
E_LIB glBindTexture ;(int, int)
E_LIB glTexImage2D ;(int, int, int, int, int, int, int, int, void *)
E_LIB glTexEnvi ;(int, int, int)
E_LIB glTexParameteri ;(int, int, int)
E_LIB glPixelStorei ;(int, int)

; lighting
E_LIB glMaterialfv ;(int, int, float *)
E_LIB glMaterialf ;(int, int, float)
E_LIB glColorMaterial ;(int, int)

E_LIB glLightfv ;(int, int, float *)
E_LIB glLightf ;(int, int, float)
E_LIB glLightModeli ;(int, int)
E_LIB glLightModelfv ;(int, float *)

; misc
E_LIB glFlush ;(void)
E_LIB glHint ;(int, int)
E_LIB glGetIntegerv ;(int, int *)
E_LIB glGetFloatv ;(int, float *)
E_LIB glFrontFace ;(int)

; opengl 1.2 arrays
E_LIB glEnableClientState ;(GLenum array)
E_LIB glDisableClientState ;(GLenum array)
E_LIB glArrayElement ;(int)
E_LIB glDrawArrays ;(GLenum, int, GLsizei)
E_LIB glDrawElements ;(GLenum, GLsizei, GLenum, const GLvoid *)
E_LIB glVertexPointer ;(int, GLenum, GLsizei, const void *)
E_LIB glColorPointer ;(int, GLenum, GLsizei, const GLvoid *)
E_LIB glNormalPointer ;(GLenum, GLsizei, const GLvoid *)
E_LIB glTexCoordPointer ;(int, GLenum, GLsizei, const GLvoid *)

; opengl 1.2 polygon offset
E_LIB glPolygonOffset ;(float, float)

; not implemented, just added to compile
;E_LIB glPointSize(float)
;E_LIB glLineWidth(float)
;E_LIB glDeleteLists(int, int)
;E_LIB glDepthFunc(int)
;E_LIB glBlendFunc(int, int)
;E_LIB glTexEnvf(int, int, int)
E_LIB glOrtho ;(float, float, float, float, float, float)
;E_LIB glVertex2i(int, int)
;E_LIB glDepthMask(int)
;E_LIB glFogi(int, int)
;E_LIB glFogfv(int, const float*)
;E_LIB glFogf(int, float)
;E_LIB glRasterPos2f(float, float)
;E_LIB glPolygonStipple(void*)
;E_LIB glTexParameterf(int, int, int)
; non compatible functions

E_LIB glDebug ;(int)

E_LIB glInit ;(void *)
E_LIB glClose ;(void)

;
; glu functions
;

E_LIB gluPerspective ;(double, double, double, double)
E_LIB gluNewQuadric
E_LIB gluDeleteQuadric
E_LIB gluQuadricDrawStyle
E_LIB gluQuadricOrientation
E_LIB gluQuadricTexture
E_LIB gluCylinder ;(GLUquadricObj, double, double, double, int, int)
E_LIB gluSphere ;(GLUquadricObj, float, int, int)

;
; Kolibri OS functions
;

E_LIB kosglMakeCurrent
E_LIB kosglSwapBuffers

purge E_LIB
