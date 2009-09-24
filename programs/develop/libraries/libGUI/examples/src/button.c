/*
	test libGUI library
*/
#include "stdarg.h"
#include "libGUI.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define	FALSE		0
#define	TRUE		1

void	callback_func_delete_window(header_t *control,void *data)
{
	QuitLibGUI((parent_t*)control);
}

void	callback_func1(header_t *control,void *data)
{
	printf("\nentry in button");
}

void	callback_func2(header_t *control,void *data)
{
	printf("\nbutton pressed");
}

void	callback_func3(header_t *control,void *data)
{
	printf("\nbutton released");
}

void	callback_func4(header_t *control,void *data)
{
	printf("\nleave button");
}

int main(int argc, char *argv[])
{
	parent_t		*window;
	gui_callback_t	*id1,*id2,*id3,*id4;
	gui_button_data_t	button_data;
	gui_button_t		*button;

	//load libGUI library
	LoadLibGUI(NULL);

	//create main window
	window=CreateWindow();
	SetWindowSizeRequest(window,90,60);
	//create button
	button_data.x=5;
	button_data.y=5;
	button_data.width=70;
	button_data.height=20;
	//create button with text
	button=CreateButtonWithText(&button_data,"Click my!");
	//set callback functions for button close window
	SetCallbackFunction(window,DELETE_EVENT,&callback_func_delete_window,NULL);
	
	//set callback functions for button
	id1=SetCallbackFunction(button,BUTTON_ENTER_EVENT,&callback_func1,NULL);
	id2=SetCallbackFunction(button,BUTTON_PRESSED_EVENT,&callback_func2,NULL);
	id3=SetCallbackFunction(button,BUTTON_RELEASED_EVENT,&callback_func3,NULL);
	id4=SetCallbackFunction(button,BUTTON_LEAVE_EVENT,&callback_func4,NULL);
	//pack button in window
	PackControls(window,button);
	//start main libGUI loop
	LibGUImain(window);
}
