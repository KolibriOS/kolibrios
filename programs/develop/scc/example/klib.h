#ifndef __KLIB_H__
#define __KLIB_H__

/********* C library *********/
   
get_event()
{
#asm
  mov  eax,10
  int  0x40
#endasm
}
   
get_key()
{
#asm
  mov  eax,2
  int  0x40
  and  eax,0x0000ff00
  shr  eax,8
#endasm
}
   
get_button()
{
#asm
  mov  eax,17
  int  0x40
  shr  eax,8
#endasm
}
   
begin_draw()
{
#asm
  mov  ebx,1
  mov  eax,12
  int  0x40
#endasm
}
   
end_draw()
{
#asm
  mov  ebx,2
  mov  eax,12
  int  0x40
#endasm
}
   
window(x1,y1,w,h,c_area,c_grab,c_fram)
int x1,y1,w,h;            /* esp +32 +28 +24 +20 */
int c_area,c_grab,c_fram; /* esp +16 +12 +8 */
{
#asm
  ; color of frames
  mov  edi,[esp+8]
   
  ; color of grab bar bit 8->color gl
  mov  esi,[esp+12]
   
  ; color of work area bit 8-> color gl
  mov  edx,[esp+16]
   
  ;left / width
  mov  ebx,[esp+32]
  shl  ebx,16
  mov  bx,[esp+24]
  ;top / height
  mov  ecx,[esp+28]
  shl  ecx,16
  mov  cx,[esp+20]
   
  ;execute
  mov  eax,0
  int  0x40
#endasm
}
   
label(x,y,color,p_string)
int x,y,color;  /* esp +20 +16 +12 */
char *p_string; /* esp +8 */
{
#asm
  mov  ebx,[esp+20]
  shl  ebx,16
  mov  bx,[esp+16]
  mov  ecx,[esp+12]
  mov  edx,[esp+8]
   
  ;find text lenght
  xor  esi,esi
.next:
  cmp  byte [edx+esi],0
  jz   .good
  inc  esi
  jmp  .next
.good:
   
  mov  eax,4
  int  0x40
#endasm
}



// Button + Text
buttonT(x1,y1,w,h,color,id,p_string, str_color)
int x1,y1,w,h;  /* esp +28 +24 +20 +16 */
int color,id; 
char *p_string;
int str_color;

{
	button(x1,y1,w,h,color,id);
	label(x1+4,y1+2,str_color,p_string);
}
   
// Button
button(x1,y1,w,h,color,id)
int x1,y1,w,h;  /* esp +28 +24 +20 +16 */
int color,id;   /* esp +12 +8 */
{
#asm
  ;left / width
  mov  ebx,[esp+28]
  shl  ebx,16
  mov  bx,[esp+20]
  ;top / height
  mov  ecx,[esp+24]
  shl  ecx,16
  mov  cx,[esp+16]
   
  mov  edx,[esp+8]
  mov  esi,[esp+12]
   
  mov  eax,8
  int  0x40
#endasm
}
// CONTROLS:
#define CheckBox	1

/* CheckBox
array[ ]:
0	int type
1	int id
2	int x,
3	int y,
4	int color,
5	int colorText
6	int checked
*/
char cbt[2] = " ";
checkbox(cb)
int *cb;
{    
	if (cb[6] == 1) // cheked is set
		cbt[0] = 'X';
	else
		cbt[0] = ' ';
	
	buttonT(cb[2], cb[3], 12,10,cb[4],cb[1],cbt,cb[5]);
}

eventControls(control,count,id)
int* control;
int count;
int id;
{
	int i;
	int *cont;
	for (i=0; i<count; i++)
	{
		cont = control[i];
		switch (cont[0])
		{
			case CheckBox:
				if (cont[1]==id)
				{
					cont[6] = 1 - cont[6];
					renderControls(control,count);
					return 1;
				}
			break;
		}
	}
	return 0;
}

renderControls(control, count)
int* control;
int count;
{
	int i;
	int *cont;
	
	for (i=0; i<count; i++)
	{
		cont = control[i];
		switch (cont[0])
		{
			case CheckBox:
				checkbox(cont)
			break;
		}
	}
}

   
s_quit()
{
#asm
  mov  eax,-1
  int  0x40
#endasm
}  

   

#endif