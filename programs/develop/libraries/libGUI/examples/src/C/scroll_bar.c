/*
	test libGUI library
*/
#include "stdarg.h"
#include "libGUI.h"
#include "stdio.h"

#define	FALSE		0
#define	TRUE		1

void	callback_func_delete_window(header_t *control,void *data)
{
	printf("\nlibGUI quit...");
	QuitLibGUI((parent_t*)control);
}

void ScrollStateH(header_t *control,void *data)
{
	gui_scroll_bar_t	*hsc;

	hsc=(gui_scroll_bar_t*)control;
	printf("\nhorizontal ruler position %d%%",(int)(hsc->ruller_pos*100));
}

void ScrollStateV(header_t *control,void *data)
{
	gui_scroll_bar_t	*vsc;

	vsc=(gui_scroll_bar_t*)control;
	printf("\nvertical ruler position %d%%",(int)(vsc->ruller_pos*100));
}

int main(int argc, char *argv[])
{
	parent_t			*window;
	gui_callback_t		*id1,*id2;
	gui_scroll_bar_data_t	horizontal_sbar_data;
	gui_scroll_bar_data_t	vertical_sbar_data;
	gui_scroll_bar_t		*ScrollBarH;
	gui_scroll_bar_t		*ScrollBarV;

	//load libGUI library
	LoadLibGUI(NULL);//use default system path to library
	//create main window
	window=CreateWindow();
	//change size of window
	SetWindowSizeRequest(window,270,207);
	//create horizontal scroll bar
	horizontal_sbar_data.x=5;
	horizontal_sbar_data.y=5;
	horizontal_sbar_data.width=250;
	horizontal_sbar_data.height=16;
	horizontal_sbar_data.ruller_size=0.2;//size of ruler E [0,1]
	horizontal_sbar_data.ruller_pos=0.5;//ruler position E [0,1]
	horizontal_sbar_data.ruller_step=0.1;//step of change ruler pos after press of button E [0,1]
	//create vertical scroll bar
	vertical_sbar_data.x=5;
	vertical_sbar_data.y=26;
	vertical_sbar_data.width=16;
	vertical_sbar_data.height=150;
	vertical_sbar_data.ruller_size=0.5;//size of ruler E [0,1]
	vertical_sbar_data.ruller_pos=0.05;//ruler position E [0,1]
	vertical_sbar_data.ruller_step=0.1;//step of change ruler pos after press of button E [0,1]
	
	//create horizontal and vertical scroll bars
	ScrollBarH=CreateHorizontalScrollBar(&horizontal_sbar_data);
	ScrollBarV=CreateVerticalScrollBar(&vertical_sbar_data);
	//set callback functions for scroll bars
	id1=SetCallbackFunction(ScrollBarH,SCROLLBAR_CHANGED_EVENT,&ScrollStateH,NULL);
	id2=SetCallbackFunction(ScrollBarV,SCROLLBAR_CHANGED_EVENT,&ScrollStateV,NULL);
	//pack scroll bars in window
	PackControls(window,ScrollBarH);
	PackControls(window,ScrollBarV);
	//start minl libGUI loop
	LibGUImain(window);
}
