#define MEMSIZE 1024*40

#include "../lib/window.h"
#include "../lib/timers.h"

window win1=0;
dword timeID = 0;
int t = 0;
void intervalTest()
{
	t++;
	if (t==10) clearInterval(timeID);
	DrawBar(100, 70, 100, 100, 0xDED7CE);
	WriteText(100,70,0x90,0,itoa(t));
}
void main()
{
	word id=0;
	timeID = setInterval(#intervalTest, 100); // 100 => 1s
	loop() 
	{
		switch(WaitEventTimeout(1))
		{
			case evButton:
				id=GetButtonID();  
				IF (id==1) ExitProcess();
				break;
		  
			case evKey:
				GetKeys();
				break;
			 
			case evReDraw:
				DefineAndDrawWindow(20,30,500,600,WINDOW_NORMAL,0xDED7CE,"Window",0);
				if (!t) WriteText(100,70,0x90,0,"Start!");
				break;
		}
		Timer.revise();
	}
}

