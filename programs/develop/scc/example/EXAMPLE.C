#asm
use32
org 0x0
   
 db 'MENUET01'
 dd 0x01
 dd _main
 dd I_END
 dd 0x100000
 dd 0x7fff0
 dd 0x0,0x0
   
include 'INTRINS.ASM'
#endasm
 
#include "klib.h" 

// CONTROLS:   
#define CONTROLS 2
int control[CONTROLS];
int cont1[7] = { CheckBox,4,10,40,0x111111,0xFFFFFF,0};
int cont2[7] = { CheckBox,5,25,55,0xBBBBBB,0,0};
   
void main()
{
	int  event;
	int  button_id;
	
	control[0]=&cont1[0];
	control[1]=&cont2[0];	

	draw_window();
	while(1)
	{
		event=get_event();
		switch(event)
		{
			case 1: draw_window(); break;
			case 2: get_button(); break;
			case 3: button_id=get_button();
			    eventControls(control,CONTROLS,button_id);
				if(button_id==1) s_quit();
			break;
		}
	}
}
   
char text1[50]="THIS IS AN EXAMPLE OF C";
char text2[50]="PROGRAM IN KOLIBRIOS";
char text3[50]="";
char text4[50]="SUCCESS";
int  p_text[4];


   
draw_window()
{
	int i; /* for index */
	int y;y=25;	

	p_text[0]=&text1[0];
	p_text[1]=&text2[0];
	p_text[2]=&text3[0];
	p_text[3]=&text4[0];

	begin_draw();

	window(100,100,320,150,0x03ffffff,0x805080d0,0x005080d0);
	label(8,8,0x10ddeeff,"Example application");
	buttonT(50,35,60,12,0x111111,1, "Click Me!", 0xFFFFFF);
	//checkbox(cbTest);
	
	renderControls(control, CONTROLS);

	for(i=0;i<4;i++)
		label(20,40+(y+=10),0x000000,p_text[i]);

	end_draw();
}
   
#asm
I_END:
#endasm
   