/*
	hello world example
*/

#include "libGUI.h"

#define TRUE		1
#define FALSE		0

void	callback_func_delete_window(header_t *control,void *data)
{
	QuitLibGUI((parent_t*)control);
}

int main(int argc, char *argv[])
{
	parent_t		*window;
	gui_text_data_t	txtdata;
	gui_text_t		*text;

	//load libGUI library
	LoadLibGUI(NULL);//load from default system path to library
	//create main window
	window=CreateWindow();
	//change size of window
	SetWindowSizeRequest(window,92,46);
	//set callback function for button close window
	SetCallbackFunction(window,DELETE_EVENT,&callback_func_delete_window,NULL);
	//create control text
	txtdata.x=5;
	txtdata.y=5;
	txtdata.font=NULL;//use default system libGUI font
	txtdata.background=TRUE;//use background for text
	txtdata.color=0xffffff;//text color
	txtdata.background_color=0xff8000;//background color
	txtdata.text="Hello world!";
	text=CreateText(&txtdata);
	
	//pack control text in window
	PackControls(window,text);

	//start libGUI main loop
	LibGUImain(window);
}

