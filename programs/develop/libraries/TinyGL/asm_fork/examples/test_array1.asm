use32
	org 0
	db 'MENUET01'
	dd 1,start,i_end,mem,stacktop,0,cur_dir_path

include '../../../../../proc32.inc'
include '../../../../../macros.inc'
include '../../../../../KOSfuncs.inc'
include '../../../../../load_lib.mac'
include '../../../../../dll.inc'
include '../opengl_const.inc'

@use_library

;Constants describing the house.3ds file (obtained using the info_3ds program)
VERTICES_OFFSET =  0x33 ;offset along which the coordinates of the vertices go
FACES_COUNT	= 0x162 ;number of faces
FACES_OFFSET	= 0x96b ;offset along which information about the edges goes

HOUSE_FILE_SIZE = 5297
txt_error_file_size db '"House.3ds file size does not match" -tE',0


align 4
start:
	load_library name_tgl, library_path, system_path, import_tinygl
	cmp eax,SF_TERMINATE_PROCESS
	jz button.exit

	mcall SF_SET_EVENTS_MASK,0x27

	;we fill the array of indices from the house.3ds file (which is embedded inside this program)
	mov esi,house_3ds
	add esi,FACES_OFFSET
	mov edi,Indices
	mov eax,FACES_COUNT
	@@:
		movsd
		movsw
		add esi,2 ;skip face properties
		dec eax
		or eax,eax
	jnz @b

	;tinygl initial context settings
	stdcall [kosglMakeCurrent], 10,10,400,350,ctx1
	stdcall [glEnable], GL_DEPTH_TEST
	stdcall [glClearColor], 0.0,0.0,0.0,0.0
	stdcall [glShadeModel], GL_SMOOTH

	call draw_3d

align 4
red_win:
	call draw_window

align 4
still:
	mcall SF_WAIT_EVENT
	cmp   al,EV_REDRAW
	jz    red_win
	cmp   al,EV_KEY
	jz    key
	cmp   al,EV_BUTTON
	jz    button
	jmp   still

align 4
draw_window:
	pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	mcall SF_CREATE_WINDOW,(50 shl 16)+430,(30 shl 16)+400,0x33ffffff,,caption
	call [kosglSwapBuffers]

	mcall SF_REDRAW,SSF_END_DRAW
	popad
	ret

align 4
key:
	mcall SF_GET_KEY

	cmp ah,27 ;Esc
	je button.exit

	cmp ah,61 ;+
	jne @f
		fld dword[scale]
		fadd dword[delt_sc]
		fstp dword[scale]
		call draw_3d
		call [kosglSwapBuffers]
	@@:
	cmp ah,45 ;-
	jne @f
		fld dword[scale]
		fsub dword[delt_sc]
		fstp dword[scale]
		call draw_3d
		call [kosglSwapBuffers]
	@@:
	cmp ah,178 ;Up
	jne @f
		fld dword[angle_y]
		fadd dword[delt_size]
		fstp dword[angle_y]
		call draw_3d
		call [kosglSwapBuffers]
	@@:
	cmp ah,177 ;Down
	jne @f
		fld dword[angle_y]
		fsub dword[delt_size]
		fstp dword[angle_y]
		call draw_3d
		call [kosglSwapBuffers]
	@@:
	cmp ah,176 ;Left
	jne @f
		fld dword[angle_x]
		fadd dword[delt_size]
		fstp dword[angle_x]
		call draw_3d
		call [kosglSwapBuffers]
	@@:
	cmp ah,179 ;Right
	jne @f
		fld dword[angle_x]
		fsub dword[delt_size]
		fstp dword[angle_x]
		call draw_3d
		call [kosglSwapBuffers]
	@@:

	jmp still

align 4
button:
	mcall SF_GET_BUTTON
	cmp ah,1
	jne still
.exit:
	mcall SF_TERMINATE_PROCESS


align 4
caption db 'Test opengl 1.1 arrays, [Esc] - exit, [<-],[->],[Up],[Down] - rotate',0

align 4
draw_3d:
stdcall [glClear], GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT ;clear the color and depth buffer
mov eax,house_3ds.end-house_3ds
cmp eax,HOUSE_FILE_SIZE
je @f
	notify_window_run txt_error_file_size
	ret
@@:
call [glPushMatrix]

	;scale and rotations
	stdcall [glScalef], [scale], [scale], [scale]
	stdcall [glRotatef], [angle_z],0.0,0.0,1.0
	stdcall [glRotatef], [angle_y],0.0,1.0,0.0
	stdcall [glRotatef], [angle_x],1.0,0.0,0.0

	;drawing via index array
	mov eax,house_3ds ;start of embedded 3ds file
	add eax,VERTICES_OFFSET
	stdcall [glVertexPointer], 3, GL_FLOAT, 0, eax ;we set an array for the vertices, 3 is the number of coordinates for one vertex
	stdcall [glEnableClientState], GL_VERTEX_ARRAY ;turn on the vertex drawing mode
	stdcall [glDrawElements], GL_TRIANGLES, FACES_COUNT*3, GL_UNSIGNED_SHORT, Indices ;mode, count, type, *indices
	stdcall [glDisableClientState], GL_VERTEX_ARRAY ;disable vertex drawing mode

call [glPopMatrix]
ret

align 4
scale dd 0.07 ;initial scale (ideally should be calculated)
delt_sc dd 0.0005
angle_z dd 90.0
angle_y dd 90.0
angle_x dd 0.0
delt_size dd 3.0

align 4
house_3ds: ;we embed the file inside the program (ideally it should open through a dialog box)
file '../../../../../demos/view3ds/3ds_objects/House.3ds'
.end:
align 4
Indices rb FACES_COUNT*6 ;3 points per edge, point index 2 bytes

;--------------------------------------------------
align 4
import_tinygl:

macro E_LIB n
{
	n dd sz_#n
}
include '../export.inc'
	dd 0,0
macro E_LIB n
{
	sz_#n db `n,0
}
include '../export.inc'

;--------------------------------------------------
system_path db '/sys/lib/'
name_tgl db 'tinygl.obj',0
;--------------------------------------------------

align 4
i_end:
	ctx1 rb 28 ;sizeof.TinyGLContext = 28
cur_dir_path rb 4096
library_path rb 4096
	rb 4096
stacktop:
mem:
