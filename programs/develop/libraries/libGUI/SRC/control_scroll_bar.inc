/*
		control ScrollBar
*/

////////////////////////////////////////////////////////////////////////////////
//				Draw Ruller
////////////////////////////////////////////////////////////////////////////////
void DrawRuller(struct ControlScrollBar *ScrollBar)
{
	int			x,y,sizex,sizey;
	int			scrollbar_size,ruller_size,ruller_pos;
	struct FINITION 	*fin;

	x=ScrollBar->ctrl_x;
	y=ScrollBar->ctrl_y;
	sizex=ScrollBar->ctrl_sizex;
	sizey=ScrollBar->ctrl_sizey;
	fin=(struct FINITION*)ScrollBar->finition;

	if (ScrollBar->scb_flags & FLAG_SCROLL_BAR_ORIENTATION_HORIZONTAL_ON)
	{
		scrollbar_size=sizex-sizey*2;
		ruller_size=(int)(scrollbar_size*ScrollBar->ruller_size);
		if (ruller_size<5) ruller_size=5;
		if (ruller_size>sizex) ruller_size=sizex-2*sizey;

		ruller_pos=x+sizey+(int)((scrollbar_size-ruller_size)*ScrollBar->ruller_pos);
		//left bar
		Draw(fin,TOOL_FILLED_RECTANGLE,x+sizey,y+1,ruller_pos-x-sizey,sizey-2,COLOR_FON);
		//right bar
		Draw(fin,TOOL_FILLED_RECTANGLE,ruller_pos+ruller_size,y+1,x+sizex-sizey-ruller_pos-ruller_size,sizey-2,COLOR_FON);
		//roller
		Draw(fin,TOOL_RECTANGLE,ruller_pos,y,ruller_size,sizey,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_FILLED_RECTANGLE,ruller_pos+1,y+1,ruller_size-2,sizey-2,COLOR_MIDDLE_LIGHT);
		Draw(fin,TOOL_VERTICAL_LINE,ruller_pos+(ruller_size/2)-2,y+sizey/4,y+(3*sizey)/4,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_VERTICAL_LINE,ruller_pos+(ruller_size/2),y+sizey/4,y+(3*sizey)/4,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_VERTICAL_LINE,ruller_pos+(ruller_size/2)+2,y+sizey/4,y+(3*sizey)/4,COLOR_ABSOLUTE_DARK);
	}

	if (ScrollBar->scb_flags & FLAG_SCROLL_BAR_ORIENTATION_VERTICAL_ON)
	{
		scrollbar_size=sizey-sizex*2;
		ruller_size=(int)(scrollbar_size*ScrollBar->ruller_size);
		if (ruller_size<5) ruller_size=5;
		if (ruller_size>sizey) ruller_size=sizey-2*sizex;

		ruller_pos=y+sizex+(int)((scrollbar_size-ruller_size)*ScrollBar->ruller_pos);
		//up bar
		Draw(fin,TOOL_FILLED_RECTANGLE,x+1,y+sizex,sizex-2,ruller_pos-y-sizex,COLOR_FON);
		//down bar
		Draw(fin,TOOL_FILLED_RECTANGLE,x+1,ruller_pos+ruller_size,sizex-2,y+sizey-sizex-ruller_pos-ruller_size,COLOR_FON);
		//roller
		Draw(fin,TOOL_RECTANGLE,x,ruller_pos,sizex,ruller_size,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_FILLED_RECTANGLE,x+1,ruller_pos+1,sizex-2,ruller_size-2,COLOR_MIDDLE_LIGHT);
		Draw(fin,TOOL_HORIZONTAL_LINE,x+sizex/4,x+(3*sizex)/4,ruller_pos+(ruller_size/2)-2,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_HORIZONTAL_LINE,x+sizex/4,x+(3*sizex)/4,ruller_pos+(ruller_size/2),COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_HORIZONTAL_LINE,x+sizex/4,x+(3*sizex)/4,ruller_pos+(ruller_size/2)+2,COLOR_ABSOLUTE_DARK);
	}

}

////////////////////////////////////////////////////////////////////////////////
//				Draw full Scroll Bar
////////////////////////////////////////////////////////////////////////////////
void DrawScrollBar(struct ControlScrollBar *ScrollBar)
{
	int			x,y,sizex,sizey;
	struct FINITION 	*fin;

	x=ScrollBar->ctrl_x;
	y=ScrollBar->ctrl_y;
	sizex=ScrollBar->ctrl_sizex;
	sizey=ScrollBar->ctrl_sizey;
	fin=(struct FINITION*)ScrollBar->finition;

	if (ScrollBar->scb_flags & FLAG_SCROLL_BAR_ORIENTATION_HORIZONTAL_ON)
	{

		//draw child buttons and roller
		Draw(fin,TOOL_RECTANGLE,x,y,sizex,sizey,COLOR_ABSOLUTE_DARK);
		//left button
		Draw(fin,TOOL_RECTANGLE,x,y,sizey,sizey,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_FILLED_RECTANGLE,x+1,y+1,sizey-2,sizey-2,COLOR_MIDDLE_LIGHT);
		//left arrow of left button
		Draw(fin,TOOL_LINE,x+sizey/3,y+sizey/2,x+(2*sizey)/3,y+sizey/4,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_LINE,x+sizey/3,y+sizey/2,x+(2*sizey)/3,y+(3*sizey)/4,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_LINE,x+1+(sizey/3),y+sizey/2,x+1+(2*sizey)/3,y+sizey/4,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_LINE,x+1+(sizey/3),y+sizey/2,x+1+(2*sizey)/3,y+(3*sizey)/4,COLOR_ABSOLUTE_DARK);
		//right button
		Draw(fin,TOOL_RECTANGLE,x+sizex-sizey,y,sizey,sizey,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_FILLED_RECTANGLE,x+sizex-sizey+1,y+1,sizey-2,sizey-2,COLOR_MIDDLE_LIGHT);
		//right arrow of right button
		Draw(fin,TOOL_LINE,x+sizex-sizey/3,y+sizey/2,x+sizex-(2*sizey)/3,y+sizey/4,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_LINE,x+sizex-sizey/3,y+sizey/2,x+sizex-(2*sizey)/3,y+(3*sizey)/4,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_LINE,x+sizex-1-(sizey/3),y+sizey/2,x+sizex-1-(2*sizey)/3,y+sizey/4,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_LINE,x+sizex-1-(sizey/3),y+sizey/2,x+sizex-1-(2*sizey)/3,y+(3*sizey)/4,COLOR_ABSOLUTE_DARK);
		//roller
		DrawRuller(ScrollBar);
	}

	if (ScrollBar->scb_flags & FLAG_SCROLL_BAR_ORIENTATION_VERTICAL_ON)
	{
		//draw child buttons and roller
		Draw(fin,TOOL_RECTANGLE,x,y,sizex,sizey,COLOR_ABSOLUTE_DARK);
		//up button
		Draw(fin,TOOL_RECTANGLE,x,y,sizex,sizex,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_FILLED_RECTANGLE,x+1,y+1,sizex-2,sizex-2,COLOR_MIDDLE_LIGHT);
		//up arrow of up button
		Draw(fin,TOOL_LINE,x+sizex/2,y+sizex/3,x+sizex/4,y+(2*sizex)/3,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_LINE,x+sizex/2,y+sizex/3,x+(3*sizex)/4,y+(2*sizex)/3,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_LINE,x+sizex/2,y+1+(sizex/3),x+sizex/4,y+1+(2*sizex)/3,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_LINE,x+sizex/2,y+1+(sizex/3),x+(3*sizex)/4,y+1+(2*sizex)/3,COLOR_ABSOLUTE_DARK);
		//down button
		Draw(fin,TOOL_RECTANGLE,x,y+sizey-sizex,sizex,sizex,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_FILLED_RECTANGLE,x+1,y+sizey-sizex+1,sizex-2,sizex-2,COLOR_MIDDLE_LIGHT);
		//down arrow of down button
		Draw(fin,TOOL_LINE,x+sizex/2,y+sizey-sizex/3,x+sizex/4,y+sizey-(2*sizex)/3,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_LINE,x+sizex/2,y+sizey-sizex/3,x+(3*sizex)/4,y+sizey-(2*sizex)/3,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_LINE,x+sizex/2,y+sizey-1-(sizex/3),x+sizex/4,y+sizey-1-(2*sizex)/3,COLOR_ABSOLUTE_DARK);
		Draw(fin,TOOL_LINE,x+sizex/2,y+sizey-1-(sizex/3),x+(3*sizex)/4,y+sizey-1-(2*sizex)/3,COLOR_ABSOLUTE_DARK);
		//roller
		DrawRuller(ScrollBar);
	}
}

////////////////////////////////////////////////////////////////////////////////
//				Vertical Scroll Bar
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//		animation of up button of Scroll Bar
////////////////////////////////////////////////////////////////////////////////
void ScrollBar_DrawPressedUpButton(struct ControlScrollBar *ScrollBar)
{
	int			x,y,sizex,sizey;
	struct FINITION 	*fin;

	x=ScrollBar->ctrl_x;
	y=ScrollBar->ctrl_y;
	sizex=ScrollBar->ctrl_sizex;
	sizey=ScrollBar->ctrl_sizey;
	fin=(struct FINITION*)ScrollBar->finition;

	//up button
	Draw(fin,TOOL_RECTANGLE,x,y,sizex,sizex,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_FILLED_RECTANGLE,x+1,y+1,sizex-2,sizex-2,COLOR_FON);
	//up arrow of up button
	Draw(fin,TOOL_LINE,x+sizex/2,y+sizex/3,x+sizex/4,y+(2*sizex)/3,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex/2,y+sizex/3,x+(3*sizex)/4,y+(2*sizex)/3,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex/2,y+1+(sizex/3),x+sizex/4,y+1+(2*sizex)/3,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex/2,y+1+(sizex/3),x+(3*sizex)/4,y+1+(2*sizex)/3,COLOR_ABSOLUTE_DARK);
}

void ScrollBar_DrawReleasedUpButton(struct ControlScrollBar *ScrollBar)
{
	int			x,y,sizex,sizey;
	struct FINITION 	*fin;

	x=ScrollBar->ctrl_x;
	y=ScrollBar->ctrl_y;
	sizex=ScrollBar->ctrl_sizex;
	sizey=ScrollBar->ctrl_sizey;
	fin=(struct FINITION*)ScrollBar->finition;

	//up button
	Draw(fin,TOOL_RECTANGLE,x,y,sizex,sizex,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_FILLED_RECTANGLE,x+1,y+1,sizex-2,sizex-2,COLOR_MIDDLE_LIGHT);
	//up arrow of up button
	Draw(fin,TOOL_LINE,x+sizex/2,y+sizex/3,x+sizex/4,y+(2*sizex)/3,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex/2,y+sizex/3,x+(3*sizex)/4,y+(2*sizex)/3,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex/2,y+1+(sizex/3),x+sizex/4,y+1+(2*sizex)/3,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex/2,y+1+(sizex/3),x+(3*sizex)/4,y+1+(2*sizex)/3,COLOR_ABSOLUTE_DARK);
}

////////////////////////////////////////////////////////////////////////////////
//		animation of down button of Scroll Bar
////////////////////////////////////////////////////////////////////////////////
void ScrollBar_DrawPressedDownButton(struct ControlScrollBar *ScrollBar)
{
	int			x,y,sizex,sizey;
	struct FINITION 	*fin;

	x=ScrollBar->ctrl_x;
	y=ScrollBar->ctrl_y;
	sizex=ScrollBar->ctrl_sizex;
	sizey=ScrollBar->ctrl_sizey;
	fin=(struct FINITION*)ScrollBar->finition;

	//down button
	Draw(fin,TOOL_RECTANGLE,x,y+sizey-sizex,sizex,sizex,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_FILLED_RECTANGLE,x+1,y+sizey-sizex+1,sizex-2,sizex-2,COLOR_FON);
	//down arrow of down button
	Draw(fin,TOOL_LINE,x+sizex/2,y+sizey-sizex/3,x+sizex/4,y+sizey-(2*sizex)/3,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex/2,y+sizey-sizex/3,x+(3*sizex)/4,y+sizey-(2*sizex)/3,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex/2,y+sizey-1-(sizex/3),x+sizex/4,y+sizey-1-(2*sizex)/3,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex/2,y+sizey-1-(sizex/3),x+(3*sizex)/4,y+sizey-1-(2*sizex)/3,COLOR_ABSOLUTE_DARK);
}

void ScrollBar_DrawReleasedDownButton(struct ControlScrollBar *ScrollBar)
{
	int			x,y,sizex,sizey;
	struct FINITION 	*fin;

	x=ScrollBar->ctrl_x;
	y=ScrollBar->ctrl_y;
	sizex=ScrollBar->ctrl_sizex;
	sizey=ScrollBar->ctrl_sizey;
	fin=(struct FINITION*)ScrollBar->finition;

	//down button
	Draw(fin,TOOL_RECTANGLE,x,y+sizey-sizex,sizex,sizex,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_FILLED_RECTANGLE,x+1,y+sizey-sizex+1,sizex-2,sizex-2,COLOR_MIDDLE_LIGHT);
	//down arrow of down button
	Draw(fin,TOOL_LINE,x+sizex/2,y+sizey-sizex/3,x+sizex/4,y+sizey-(2*sizex)/3,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex/2,y+sizey-sizex/3,x+(3*sizex)/4,y+sizey-(2*sizex)/3,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex/2,y+sizey-1-(sizex/3),x+sizex/4,y+sizey-1-(2*sizex)/3,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex/2,y+sizey-1-(sizex/3),x+(3*sizex)/4,y+sizey-1-(2*sizex)/3,COLOR_ABSOLUTE_DARK);
}

////////////////////////////////////////////////////////////////////////////////
//		callback function for up button of Scroll Bar
////////////////////////////////////////////////////////////////////////////////
void ScrollBar_FuncCallbackForUpButton_Pressed(struct HEADER *control,void *data)
{
	struct ControlScrollBar *ScrollBar;
	struct	HEADERPARENT	*main_parent;

	ScrollBar=(struct ControlScrollBar*)data;
	if (ScrollBar->ruller_size!=1.0)
	{
		ScrollBar->ruller_pos=ScrollBar->ruller_pos-ScrollBar->ruller_step;
	}
	if (ScrollBar->ruller_pos<0.0) ScrollBar->ruller_pos=0.0;
	ScrollBar_DrawPressedUpButton(ScrollBar);

	if (ControlCheckCallbackEvent(ScrollBar,SCROLLBAR_CHANGED_EVENT)!=NULL)
	{
		main_parent=(struct	HEADERPARENT*)ScrollBar->main_parent;
		ScrollBar->flags=ScrollBar->flags | FLAG_CONNECT_EVENT_ON;
		main_parent->control_for_callback_function[main_parent->number_callbacks]=
		(DWORD*)ScrollBar;
		main_parent->callback_for_control_callback[main_parent->number_callbacks]=
		(DWORD*)ControlCheckCallbackEvent(ScrollBar,SCROLLBAR_CHANGED_EVENT);
		main_parent->number_callbacks++;
	}
	DrawRuller(ScrollBar);
}

void ScrollBar_FuncCallbackForUpButton_Released(struct HEADER *control,void *data)
{
	struct ControlScrollBar *ScrollBar;

	ScrollBar=(struct ControlScrollBar*)data;
	ScrollBar_DrawReleasedUpButton(ScrollBar);
}

////////////////////////////////////////////////////////////////////////////////
//		callback function for down button of Scroll Bar
////////////////////////////////////////////////////////////////////////////////
void ScrollBar_FuncCallbackForDownButton_Pressed(struct HEADER *control,void *data)
{
	struct ControlScrollBar *ScrollBar;
	struct	HEADERPARENT	*main_parent;

	ScrollBar=(struct ControlScrollBar*)data;
	if (ScrollBar->ruller_size!=1.0)
	{
		ScrollBar->ruller_pos=ScrollBar->ruller_pos+ScrollBar->ruller_step;
	}
	if (ScrollBar->ruller_pos>1.0) ScrollBar->ruller_pos=1.0;
	ScrollBar_DrawPressedDownButton(ScrollBar);

	if (ControlCheckCallbackEvent(ScrollBar,SCROLLBAR_CHANGED_EVENT)!=NULL)
	{
		main_parent=(struct	HEADERPARENT*)ScrollBar->main_parent;
		ScrollBar->flags=ScrollBar->flags | FLAG_CONNECT_EVENT_ON;
		main_parent->control_for_callback_function[main_parent->number_callbacks]=
		(DWORD*)ScrollBar;
		main_parent->callback_for_control_callback[main_parent->number_callbacks]=
		(DWORD*)ControlCheckCallbackEvent(ScrollBar,SCROLLBAR_CHANGED_EVENT);
		main_parent->number_callbacks++;
	}
	DrawRuller(ScrollBar);
}

void ScrollBar_FuncCallbackForDownButton_Released(struct HEADER *control,void *data)
{
	struct ControlScrollBar *ScrollBar;

	ScrollBar=(struct ControlScrollBar*)data;
	ScrollBar_DrawReleasedDownButton(ScrollBar);
	DrawRuller(ScrollBar);
}

////////////////////////////////////////////////////////////////////////////////
//		Horizontal Scroll Bar
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//		animation of left button of Scroll Bar
////////////////////////////////////////////////////////////////////////////////
void ScrollBar_DrawPressedLeftButton(struct ControlScrollBar *ScrollBar)
{
	int			x,y,sizex,sizey;
	struct FINITION 	*fin;

	x=ScrollBar->ctrl_x;
	y=ScrollBar->ctrl_y;
	sizex=ScrollBar->ctrl_sizex;
	sizey=ScrollBar->ctrl_sizey;
	fin=(struct FINITION*)ScrollBar->finition;

	//left button
	Draw(fin,TOOL_FILLED_RECTANGLE,x+1,y+1,sizey-2,sizey-2,COLOR_FON);
	//left arrow of left button
	Draw(fin,TOOL_LINE,x+sizey/3,y+sizey/2,x+(2*sizey)/3,y+sizey/4,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizey/3,y+sizey/2,x+(2*sizey)/3,y+(3*sizey)/4,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+1+(sizey/3),y+sizey/2,x+1+(2*sizey)/3,y+sizey/4,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+1+(sizey/3),y+sizey/2,x+1+(2*sizey)/3,y+(3*sizey)/4,COLOR_ABSOLUTE_DARK);
}

void ScrollBar_DrawReleasedLeftButton(struct ControlScrollBar *ScrollBar)
{
	int			x,y,sizex,sizey;
	struct FINITION 	*fin;

	x=ScrollBar->ctrl_x;
	y=ScrollBar->ctrl_y;
	sizex=ScrollBar->ctrl_sizex;
	sizey=ScrollBar->ctrl_sizey;
	fin=(struct FINITION*)ScrollBar->finition;

	//left button
	Draw(fin,TOOL_FILLED_RECTANGLE,x+1,y+1,sizey-2,sizey-2,COLOR_MIDDLE_LIGHT);
	//left arrow of left button
	Draw(fin,TOOL_LINE,x+sizey/3,y+sizey/2,x+(2*sizey)/3,y+sizey/4,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizey/3,y+sizey/2,x+(2*sizey)/3,y+(3*sizey)/4,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+1+(sizey/3),y+sizey/2,x+1+(2*sizey)/3,y+sizey/4,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+1+(sizey/3),y+sizey/2,x+1+(2*sizey)/3,y+(3*sizey)/4,COLOR_ABSOLUTE_DARK);
}

////////////////////////////////////////////////////////////////////////////////
//		animation of right button of Scroll Bar
////////////////////////////////////////////////////////////////////////////////
void ScrollBar_DrawPressedRightButton(struct ControlScrollBar *ScrollBar)
{
	int			x,y,sizex,sizey;
	struct FINITION 	*fin;

	x=ScrollBar->ctrl_x;
	y=ScrollBar->ctrl_y;
	sizex=ScrollBar->ctrl_sizex;
	sizey=ScrollBar->ctrl_sizey;
	fin=(struct FINITION*)ScrollBar->finition;

	//right button
	Draw(fin,TOOL_FILLED_RECTANGLE,x+sizex-sizey+1,y+1,sizey-2,sizey-2,COLOR_FON);
	//right arrow of right button
	Draw(fin,TOOL_LINE,x+sizex-sizey/3,y+sizey/2,x+sizex-(2*sizey)/3,y+sizey/4,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex-sizey/3,y+sizey/2,x+sizex-(2*sizey)/3,y+(3*sizey)/4,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex-1-(sizey/3),y+sizey/2,x+sizex-1-(2*sizey)/3,y+sizey/4,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex-1-(sizey/3),y+sizey/2,x+sizex-1-(2*sizey)/3,y+(3*sizey)/4,COLOR_ABSOLUTE_DARK);
}

void ScrollBar_DrawReleasedRightButton(struct ControlScrollBar *ScrollBar)
{
	int			x,y,sizex,sizey;
	struct FINITION 	*fin;

	x=ScrollBar->ctrl_x;
	y=ScrollBar->ctrl_y;
	sizex=ScrollBar->ctrl_sizex;
	sizey=ScrollBar->ctrl_sizey;
	fin=(struct FINITION*)ScrollBar->finition;

	//right button
	Draw(fin,TOOL_FILLED_RECTANGLE,x+sizex-sizey+1,y+1,sizey-2,sizey-2,COLOR_MIDDLE_LIGHT);
	//right arrow of right button
	Draw(fin,TOOL_LINE,x+sizex-sizey/3,y+sizey/2,x+sizex-(2*sizey)/3,y+sizey/4,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex-sizey/3,y+sizey/2,x+sizex-(2*sizey)/3,y+(3*sizey)/4,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex-1-(sizey/3),y+sizey/2,x+sizex-1-(2*sizey)/3,y+sizey/4,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_LINE,x+sizex-1-(sizey/3),y+sizey/2,x+sizex-1-(2*sizey)/3,y+(3*sizey)/4,COLOR_ABSOLUTE_DARK);
}
////////////////////////////////////////////////////////////////////////////////
//		callback function for left button of Scroll Bar
////////////////////////////////////////////////////////////////////////////////
void ScrollBar_FuncCallbackForLeftButton_Pressed(struct HEADER *control,void *data)
{
	struct ControlScrollBar *ScrollBar;
	struct	HEADERPARENT	*main_parent;

	ScrollBar=(struct ControlScrollBar*)data;
	if (ScrollBar->ruller_size!=1.0)
	{
		ScrollBar->ruller_pos=ScrollBar->ruller_pos-ScrollBar->ruller_step;
	}
	if (ScrollBar->ruller_pos<0.0) ScrollBar->ruller_pos=0.0;
	ScrollBar_DrawPressedLeftButton(ScrollBar);

	if (ControlCheckCallbackEvent(ScrollBar,SCROLLBAR_CHANGED_EVENT)!=NULL)
	{
		main_parent=(struct	HEADERPARENT*)ScrollBar->main_parent;
		ScrollBar->flags=ScrollBar->flags | FLAG_CONNECT_EVENT_ON;
		main_parent->control_for_callback_function[main_parent->number_callbacks]=
		(DWORD*)ScrollBar;
		main_parent->callback_for_control_callback[main_parent->number_callbacks]=
		(DWORD*)ControlCheckCallbackEvent(ScrollBar,SCROLLBAR_CHANGED_EVENT);
		main_parent->number_callbacks++;
	}
	DrawRuller(ScrollBar);
}

void ScrollBar_FuncCallbackForLeftButton_Released(struct HEADER *control,void *data)
{
	struct ControlScrollBar *ScrollBar;

	ScrollBar=(struct ControlScrollBar*)data;
	ScrollBar_DrawReleasedLeftButton(ScrollBar);
}

////////////////////////////////////////////////////////////////////////////////
//		callback function for right button of Scroll Bar
////////////////////////////////////////////////////////////////////////////////
void ScrollBar_FuncCallbackForRightButton_Pressed(struct HEADER *control,void *data)
{
	struct ControlScrollBar *ScrollBar;
	struct	HEADERPARENT	*main_parent;

	ScrollBar=(struct ControlScrollBar*)data;
	if (ScrollBar->ruller_size!=1.0)
	{
		ScrollBar->ruller_pos=ScrollBar->ruller_pos+ScrollBar->ruller_step;
	}
	if (ScrollBar->ruller_pos>1.0) ScrollBar->ruller_pos=1.0;
	ScrollBar_DrawPressedRightButton(ScrollBar);

	if (ControlCheckCallbackEvent(ScrollBar,SCROLLBAR_CHANGED_EVENT)!=NULL)
	{
		main_parent=(struct	HEADERPARENT*)ScrollBar->main_parent;
		ScrollBar->flags=ScrollBar->flags | FLAG_CONNECT_EVENT_ON;
		main_parent->control_for_callback_function[main_parent->number_callbacks]=
		(DWORD*)ScrollBar;
		main_parent->callback_for_control_callback[main_parent->number_callbacks]=
		(DWORD*)ControlCheckCallbackEvent(ScrollBar,SCROLLBAR_CHANGED_EVENT);
		main_parent->number_callbacks++;
	}
	DrawRuller(ScrollBar);
}

void ScrollBar_FuncCallbackForRightButton_Released(struct HEADER *control,void *data)
{
	struct ControlScrollBar *ScrollBar;

	ScrollBar=(struct ControlScrollBar*)data;
	ScrollBar_DrawReleasedRightButton(ScrollBar);
	DrawRuller(ScrollBar);
}

////////////////////////////////////////////////////////////////////////////////
//		callback function for ruller bar button of Scroll Bar
////////////////////////////////////////////////////////////////////////////////
void ScrollBar_FuncCallbackForScrollRullerPressed(struct HEADER *control,void *data)
{
	struct ControlScrollBar *ScrollBar;

	ScrollBar=(struct ControlScrollBar*)data;
	ScrollBar->scb_flags=ScrollBar->scb_flags | FLAG_SCROLL_RULLER_PRESSED;
}

void ScrollBar_FuncCallbackForScrollRullerReleased(struct HEADER *control,void *data)
{
	struct ControlScrollBar *ScrollBar;

	ScrollBar=(struct ControlScrollBar*)data;
	ScrollBar->scb_flags=ScrollBar->scb_flags & FLAG_SCROLL_RULLER_RELEASED;
}

//---------------------------------------------------------------------------------
//			 control ScrollBar
//---------------------------------------------------------------------------------
void ScrollBarProc(struct ControlScrollBar *ScrollBar,struct MESSAGE *message)
{
	int			mx,my;
	char			mouse_buttons_state;
	int			x,y,sizex,sizey;
	int			scrollbar_size,ruller_size,ruller_pos;
	int			ruller_min,ruller_max;
	float			max_pos,current_pos,last_ruller_pos;
	struct	FINITION	*fin;
	struct TIMER		*timer;
	struct	HEADERPARENT	*main_parent;

	x=ScrollBar->ctrl_x;
	y=ScrollBar->ctrl_y;
	sizex=ScrollBar->ctrl_sizex;
	sizey=ScrollBar->ctrl_sizey;

	switch(message->type)
	{
		case MESSAGE_FULL_REDRAW_ALL:
		{
			//draw ScrollBar
			if (ScrollBar->flags & FLAG_SHOW_CONTROL)	DrawScrollBar(ScrollBar);
			break;
		}
		case MESSAGE_FULL_REDRAW_ALL_WITH_FINITION:
		{
			fin=(struct FINITION*)ScrollBar->finition;
			fin->flags=fin->flags | FINITION_ON;
			fin->x=message->arg1;
			fin->y=message->arg2;
			fin->sizex=message->arg3;
			fin->sizey=message->arg4;
			DrawScrollBar(ScrollBar);
			break;
		}
		case MESSAGE_SPECIALIZED:
		{
			if (ScrollBar->flags & FLAG_GET_SPECIALIZED_MESSAGE_ON)
			{
				if (ScrollBar->flags & FLAG_SHOW_CONTROL)	DrawScrollBar(ScrollBar);
				ScrollBar->flags=ScrollBar->flags & FLAG_GET_SPECIALIZED_MESSAGE_OFF;
			}
			break;
		}
		case MESSAGE_MOUSE_EVENT:
		{
			mx=message->arg1;
			my=message->arg2;
			mouse_buttons_state=message->arg3;
			main_parent=(struct	HEADERPARENT*)ScrollBar->main_parent;

			//check ruller state
			if (ScrollBar->scb_flags & FLAG_SCROLL_RULLER_PRESSED)
			{	//		horizontal ScrollBar
				if (ScrollBar->scb_flags & FLAG_SCROLL_BAR_ORIENTATION_HORIZONTAL_ON)
				{
					scrollbar_size=sizex-sizey*2;
					ruller_size=scrollbar_size*ScrollBar->ruller_size;
					if (ruller_size<5) ruller_size=5;

					ruller_min=x+sizey;     //minimum x
					ruller_max=ruller_min+scrollbar_size-ruller_size;
					ruller_pos=mx-(ruller_size/2);//ruller centred under mouse pointer

					if (ruller_pos<ruller_min) ruller_pos=ruller_min;
					if (ruller_pos>ruller_max) ruller_pos=ruller_max;

					if (ruller_max!=ruller_min)
					{
						max_pos=(float)(ruller_max-ruller_min);
						current_pos=(float)(ruller_pos-ruller_min);
						last_ruller_pos=ScrollBar->ruller_pos;
						ScrollBar->ruller_pos=current_pos/max_pos;
					}
					else {ScrollBar->ruller_pos=0.0;}

					if (ControlCheckCallbackEvent(ScrollBar,SCROLLBAR_CHANGED_EVENT)!=NULL)
					{
						if (ScrollBar->ruller_pos!=last_ruller_pos)
						{
							ScrollBar->flags=ScrollBar->flags | FLAG_CONNECT_EVENT_ON;
							main_parent->control_for_callback_function[main_parent->number_callbacks]=
							(DWORD*)ScrollBar;
							main_parent->callback_for_control_callback[main_parent->number_callbacks]=
							(DWORD*)ControlCheckCallbackEvent(ScrollBar,SCROLLBAR_CHANGED_EVENT);
							main_parent->number_callbacks++;
						}
					}
					if (ScrollBar->ruller_pos!=last_ruller_pos)	DrawRuller(ScrollBar);
					break;
				}
				//		vertical ScrollBar
				if (ScrollBar->scb_flags & FLAG_SCROLL_BAR_ORIENTATION_VERTICAL_ON)
				{
					scrollbar_size=sizey-sizex*2;
					ruller_size=scrollbar_size*ScrollBar->ruller_size;
					if (ruller_size<5) ruller_size=5;

					ruller_min=y+sizex;     //minimum x
					ruller_max=ruller_min+scrollbar_size-ruller_size;
					ruller_pos=my-(ruller_size/2);//ruller centred under mouse pointer

					if (ruller_pos<ruller_min) ruller_pos=ruller_min;
					if (ruller_pos>ruller_max) ruller_pos=ruller_max;

					if (ruller_max!=ruller_min)
					{
						max_pos=(float)(ruller_max-ruller_min);
						current_pos=(float)(ruller_pos-ruller_min);
						last_ruller_pos=ScrollBar->ruller_pos;
						ScrollBar->ruller_pos=current_pos/max_pos;
					}
					else {ScrollBar->ruller_pos=0.0;}

					if (ControlCheckCallbackEvent(ScrollBar,SCROLLBAR_CHANGED_EVENT)!=NULL)
					{
						if (ScrollBar->ruller_pos!=last_ruller_pos)
						{
							ScrollBar->flags=ScrollBar->flags | FLAG_CONNECT_EVENT_ON;
							main_parent->control_for_callback_function[main_parent->number_callbacks]=
							(DWORD*)ScrollBar;
							main_parent->callback_for_control_callback[main_parent->number_callbacks]=
							(DWORD*)ControlCheckCallbackEvent(ScrollBar,SCROLLBAR_CHANGED_EVENT);
							main_parent->number_callbacks++;
						}
					}
					if (ScrollBar->ruller_pos!=last_ruller_pos)	DrawRuller(ScrollBar);
				}
			}
			break;
		}
		case MESSAGE_CHANGE_POSITION_EVENT:
		{
			ScrollBar->ctrl_x=ScrollBar->ctrl_x+message->arg1;
			ScrollBar->ctrl_y=ScrollBar->ctrl_y+message->arg2;
			break;
		}
		case MESSAGE_CALL_TIMER_EVENT:
		{
			if (ScrollBar->timer!=(DWORD*)NULL)
			{
				timer=(struct TIMER*)ScrollBar->timer;
				if (timer->flags & FLAG_TIMER_ON)	Timer(timer);
			}
			break;
		}
		case MESSAGE_DESTROY_CONTROL:
		{
			if (ScrollBar->timer!=(DWORD*)NULL)	free(ScrollBar->timer);
			free(ScrollBar->finition);
			break;
		}
		case MESSAGE_SET_MAIN_PARENT:
		{
			SendMessage((struct HEADER*)ScrollBar,message);
			ScrollBar->main_parent=(DWORD*)message->arg1;
			break;
		}

		default: break;
	}
	//send message to child controls(if there is)
	SendMessage((struct HEADER*)ScrollBar,message);
}

//---------------------------------------------------------------------------------
//				create control ScrollBar
//---------------------------------------------------------------------------------
void* CreateScrollBar(struct ScrollBarData *info_for_control)
{
	struct ControlScrollBar *ScrollBar;
	struct FINITION 	*fin;

	ScrollBar=malloc(sizeof(struct ControlScrollBar));
	ScrollBar->finition=malloc(sizeof(struct FINITION));
	fin=(struct FINITION*)ScrollBar->finition;
	fin->flags=0;

	ID++;
#ifdef	DEBUG
	printf("\ncreated scroll bar with ID=%d",(int)ID);
#endif
	ScrollBar->child_bk=(DWORD*)NULL;
	ScrollBar->child_fd=(DWORD*)NULL;
	ScrollBar->active_control_for_keys=(DWORD*)NULL;
	ScrollBar->active_control_for_mouse=(DWORD*)NULL;
	ScrollBar->callback=(DWORD*)NULL;
	ScrollBar->timer=(DWORD*)NULL;

	ScrollBar->ctrl_proc=(DWORD*)&ScrollBarProc;
	ScrollBar->ctrl_x=(DWORD)info_for_control->x;
	ScrollBar->ctrl_y=(DWORD)info_for_control->y;
	ScrollBar->ctrl_sizex=(DWORD)info_for_control->width;
	ScrollBar->ctrl_sizey=(DWORD)info_for_control->height;
	ScrollBar->ctrl_ID=ID;
	ScrollBar->ruller_size=info_for_control->ruller_size;
	ScrollBar->ruller_pos=info_for_control->ruller_pos;
	ScrollBar->ruller_step=info_for_control->ruller_step;
	ScrollBar->flags=0;
	ScrollBar->flags=ScrollBar->flags | FLAG_SHOW_CONTROL;

	return(ScrollBar);
}

void* CreateHorizontalScrollBar(struct ScrollBarData *info_for_control)
{
	struct ControlScrollBar *ScrollBar;
	struct ControlButton	*ChildButton1;
	struct ControlButton	*ChildButton2;
	struct ControlButton	*ScrollRuller;
	struct ButtonData		ChildButton1Data;
	struct ButtonData		ChildButton2Data;
	struct ButtonData		ScrollRullerData;

	ScrollBar=CreateScrollBar(info_for_control);
	//set horizontal orientation
	ScrollBar->scb_flags=0;
	ScrollBar->scb_flags=ScrollBar->scb_flags | FLAG_SCROLL_BAR_ORIENTATION_HORIZONTAL_ON;

	//fill data for first child button of scroller
	ChildButton1Data.x=0;
	ChildButton1Data.y=0;
	ChildButton1Data.width=ScrollBar->ctrl_sizey;
	ChildButton1Data.height=ChildButton1Data.width;

	//fill data for second child button of scroller
	ChildButton2Data.x=ScrollBar->ctrl_sizex-ScrollBar->ctrl_sizey;
	ChildButton2Data.y=0;
	ChildButton2Data.width=ScrollBar->ctrl_sizey;
	ChildButton2Data.height=ChildButton2Data.width;

	//fill data for scroll ruller button of scroller
	ScrollRullerData.x=ScrollBar->ctrl_sizey;
	ScrollRullerData.y=0;
	ScrollRullerData.width=ScrollBar->ctrl_sizex-2*ScrollBar->ctrl_sizey;
	ScrollRullerData.height=ScrollBar->ctrl_sizey;

	ChildButton1=CreateButton(&ChildButton1Data);
	ChildButton2=CreateButton(&ChildButton2Data);
	ScrollRuller=CreateButton(&ScrollRullerData);

	ChildButton1->flags=ChildButton1->flags &  FLAG_HIDE_CONTROL;
	ChildButton2->flags=ChildButton2->flags &  FLAG_HIDE_CONTROL;
	ScrollRuller->flags=ScrollRuller->flags &  FLAG_HIDE_CONTROL;

	SetCallbackFunction(ChildButton1,BUTTON_PRESSED_EVENT,&ScrollBar_FuncCallbackForLeftButton_Pressed,ScrollBar);
	SetCallbackFunction(ChildButton1,BUTTON_RELEASED_EVENT,&ScrollBar_FuncCallbackForLeftButton_Released,ScrollBar);

	SetCallbackFunction(ChildButton2,BUTTON_PRESSED_EVENT,&ScrollBar_FuncCallbackForRightButton_Pressed,ScrollBar);
	SetCallbackFunction(ChildButton2,BUTTON_RELEASED_EVENT,&ScrollBar_FuncCallbackForRightButton_Released,ScrollBar);

	SetCallbackFunction(ScrollRuller,BUTTON_PRESSED_EVENT,&ScrollBar_FuncCallbackForScrollRullerPressed,ScrollBar);
	SetCallbackFunction(ScrollRuller,BUTTON_RELEASED_EVENT,&ScrollBar_FuncCallbackForScrollRullerReleased,ScrollBar);

	//pack button in control ScrollBar
	PackControls(ScrollBar,ChildButton1);
	PackControls(ScrollBar,ChildButton2);
	PackControls(ScrollBar,ScrollRuller);

	return(ScrollBar);
}

void* CreateVerticalScrollBar(struct ScrollBarData *info_for_control)
{
	struct ControlScrollBar *ScrollBar;
	struct ControlButton	*ChildButton1;
	struct ControlButton	*ChildButton2;
	struct ControlButton	*ScrollRuller;
	struct ButtonData		ChildButton1Data;
	struct ButtonData		ChildButton2Data;
	struct ButtonData		ScrollRullerData;

	ScrollBar=CreateScrollBar(info_for_control);
	//set vertical orientation
	ScrollBar->scb_flags=0;
	ScrollBar->scb_flags=ScrollBar->scb_flags | FLAG_SCROLL_BAR_ORIENTATION_VERTICAL_ON;

	//fill data for first child button of scroller
	ChildButton1Data.x=0;
	ChildButton1Data.y=0;
	ChildButton1Data.width=ScrollBar->ctrl_sizex;
	ChildButton1Data.height=ChildButton1Data.width;

	//fill data for second child button of scroller
	ChildButton2Data.x=0;
	ChildButton2Data.y=ScrollBar->ctrl_sizey-ScrollBar->ctrl_sizex;
	ChildButton2Data.width=ScrollBar->ctrl_sizex;
	ChildButton2Data.height=ChildButton2Data.width;

	//fill data for scroll ruller button of scroller
	ScrollRullerData.x=0;
	ScrollRullerData.y=ScrollBar->ctrl_sizex;
	ScrollRullerData.width=ScrollBar->ctrl_sizex;
	ScrollRullerData.height=ScrollBar->ctrl_sizey-2*ScrollBar->ctrl_sizex;

	ChildButton1=CreateButton(&ChildButton1Data);
	ChildButton2=CreateButton(&ChildButton2Data);
	ScrollRuller=CreateButton(&ScrollRullerData);

	ChildButton1->flags=ChildButton1->flags &  FLAG_HIDE_CONTROL;
	ChildButton2->flags=ChildButton2->flags &  FLAG_HIDE_CONTROL;
	ScrollRuller->flags=ScrollRuller->flags &  FLAG_HIDE_CONTROL;

	SetCallbackFunction(ChildButton1,BUTTON_PRESSED_EVENT,&ScrollBar_FuncCallbackForUpButton_Pressed,ScrollBar);
	SetCallbackFunction(ChildButton1,BUTTON_RELEASED_EVENT,&ScrollBar_FuncCallbackForUpButton_Released,ScrollBar);

	SetCallbackFunction(ChildButton2,BUTTON_PRESSED_EVENT,&ScrollBar_FuncCallbackForDownButton_Pressed,ScrollBar);
	SetCallbackFunction(ChildButton2,BUTTON_RELEASED_EVENT,&ScrollBar_FuncCallbackForDownButton_Released,ScrollBar);

	SetCallbackFunction(ScrollRuller,BUTTON_PRESSED_EVENT,&ScrollBar_FuncCallbackForScrollRullerPressed,ScrollBar);
	SetCallbackFunction(ScrollRuller,BUTTON_RELEASED_EVENT,&ScrollBar_FuncCallbackForScrollRullerReleased,ScrollBar);

	//pack button in control ScrollBar
	PackControls(ScrollBar,ChildButton1);
	PackControls(ScrollBar,ChildButton2);
	PackControls(ScrollBar,ScrollRuller);

	return(ScrollBar);
}

