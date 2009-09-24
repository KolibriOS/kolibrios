/*
	test libGUI library
*/
#include "stdarg.h"
#include "libGUI.h"
#include "string.h"


void	callback_func_delete_window(header_t *control,void *data)
{
	QuitLibGUI((parent_t*)control);
}

void ProgressBarCallback(void *data)
{
	gui_progress_bar_t	*progress_bar;
	int			progress;
	static char		txt[16];
	
	progress_bar=(gui_progress_bar_t*)data;
	progress_bar->progress+=0.01;//incrase progress
	
	if (progress_bar->progress>1.0) progress_bar->progress=0.0;

	//calculate progress level in %
	progress=progress_bar->progress*100;
	snprintf(txt,16,"progress %d%%",progress);
	//set text for progress bar
	ProgressBarSetText(progress_bar,txt);
}

int main(int argc, char *argv[])
{
	parent_t			*window;
	gui_progress_bar_data_t	progress_bar_data;
	gui_progress_bar_t		*progress_bar;
	gui_timer_t			*timer;

	//load libGUI library
	LoadLibGUI(NULL);//use default system path to library
	//create main window
	window=CreateWindow();
	//change size of main window
	SetWindowSizeRequest(window,320,57);
	//set callback function for button close window
	SetCallbackFunction(window,DELETE_EVENT,&callback_func_delete_window,NULL);
	//create progress bar
	progress_bar_data.x=5;
	progress_bar_data.y=5;
	progress_bar_data.width=300;
	progress_bar_data.height=25;
	progress_bar_data.progress=0.0;
	progress_bar=CreateProgressBar(&progress_bar_data);
	
	//create timer for update progress level each 50 millisecunds 
	timer=SetTimerCallbackForFunction(window,5,&ProgressBarCallback,progress_bar);

	//pack progress bar in window
	PackControls(window,progress_bar);

	//update progress bar automatically each 50 millisecund
	SetProgressBarPulse(progress_bar,5);

	//call main libGUI loop
	LibGUImain(window);
}
