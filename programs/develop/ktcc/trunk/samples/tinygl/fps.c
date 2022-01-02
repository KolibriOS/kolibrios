#include <kos32sys1.h>
int time1=0;
int time2=0;
int fps1=0;
int timerend=0;

int Fps()
{
	int tr;
	time1=get_tick_count();
	if (timerend==0)
	{
 	   time2=time1; 
	   timerend=time1;	
	}
	
	tr = time1 - timerend;

	if ((time1 - time2) < 100)
	{				          
		fps1++; 
	}
	else 
	{
		draw_bar(330,8, 50,10,0xFFFFFF);
		draw_number_sys(fps1,330,8,10,0x0);
		fps1=0;
		time2=time1;
	}
	timerend=time1;
return tr;
}

