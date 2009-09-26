/*
	test libGUI library
*/
#include "stdarg.h"
#include "libGUI.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

void	callback_func_delete_window(header_t *control,void *data)
{
	QuitLibGUI((parent_t*)control);
}

void	callback_func(header_t *control,void *data)
{
	printf("\npressed button with ID=%d control=%d",(int)control->ctrl_ID,(int)control);
}

int main(int argc, char *argv[])
{
	parent_t			*window;
	gui_button_data_t		button_data;
	gui_button_t			*button;
	gui_scrolled_window_data_t	scroll_win_data;
	gui_scrolled_window_t	*ScrollWin;
	int				i,j;
	static	char			txt[20];
	
	//load libGUI library
	LoadLibGUI(NULL);
	//create main window
	window=CreateWindow();
	//change size of window
	SetWindowSizeRequest(window,270,282);
	
	//create scrolled window
	scroll_win_data.x=5;
	scroll_win_data.y=5;
	scroll_win_data.width=250;
	scroll_win_data.height=250;
	ScrollWin=CreateScrolledWindow(&scroll_win_data);

	//create buttons
	for(j=1;j<=10;j++)
	{
		for(i=1;i<=10;i++)
		{
			button_data.x=10+(i-1)*75;
			button_data.y=10+(j-1)*25;
			button_data.width=70;
			button_data.height=20;

			snprintf(txt,20,"(%d,%d)",j,i);
			button=CreateButtonWithText(&button_data,txt);
		
			SetCallbackFunction(button,BUTTON_PRESSED_EVENT,&callback_func,NULL);
			ScrolledWindowPackControls(ScrollWin,button);
		}
	}
	//set callback function for button close window
	SetCallbackFunction(window,DELETE_EVENT,&callback_func_delete_window,NULL);
	//pack scrolled window in window
	PackControls(window,ScrollWin);
	//start main libGUI loop
	LibGUImain(window);
}
