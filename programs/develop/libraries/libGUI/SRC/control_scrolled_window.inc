/*
		control ScrolledWindow
*/


////////////////////////////////////////////////////////////////////////
//		pack controls in ScrolledWindow
////////////////////////////////////////////////////////////////////////
void ScrolledWindowPackControls(void *parent,void *Control)
{
	struct HEADER 			*control;
	struct ControlScrolledWindow	*ScrolledWindow;
	struct	ControlScrollBar		*HorizontalScrollBar;
	struct	ControlScrollBar		*VerticalScrollBar;
	struct	FINITION			*fin;
	int					x,y;

	ScrolledWindow=(struct ControlScrolledWindow*)parent;
	control=(struct HEADER *)Control;

	if (control->ctrl_x+control->ctrl_sizex>ScrolledWindow->virtual_sizex)
	{
		ScrolledWindow->virtual_sizex=control->ctrl_x+control->ctrl_sizex;
	}
	if (control->ctrl_y+control->ctrl_sizey>ScrolledWindow->virtual_sizey)
	{ 
		ScrolledWindow->virtual_sizey=control->ctrl_y+control->ctrl_sizey;
	}

	PackControls(ScrolledWindow,control);

	//calculate new control coordinates
	x=control->ctrl_x+1;//add border width
	y=control->ctrl_y+1;//add border heght
	SetControlNewPosition(control,x,y);

	//save coordinates of control in arrea
	ScrolledWindow->virtual_controls_x[ScrolledWindow->number_virtual_controls]=x;
	ScrolledWindow->virtual_controls_y[ScrolledWindow->number_virtual_controls]=y;
	ScrolledWindow->number_virtual_controls++;

	x=ScrolledWindow->ctrl_x+1;
	y=ScrolledWindow->ctrl_y+1;

	//check cross control with scroll arrea
	if (CheckCrossRectangles(x,y,ScrolledWindow->scroll_arrea_sizex,ScrolledWindow->scroll_arrea_sizey,
		control->ctrl_x,control->ctrl_y,control->ctrl_sizex,control->ctrl_sizey)==TRUE)
	{
		control->flags=control->flags | FLAG_SHOW_CONTROL;
		control->flags=control->flags & FLAG_MOUSE_BLOCKED_OFF;
	}
	else
	{
		control->flags=control->flags & FLAG_HIDE_CONTROL;
		control->flags=control->flags | FLAG_MOUSE_BLOCKED_ON;
	}

	if (ScrolledWindow->virtual_sizex>ScrolledWindow->scroll_arrea_sizex)
	{
		HorizontalScrollBar=(struct	ControlScrollBar*)ScrolledWindow->horizontal_scroll;

		if ((ScrolledWindow->scw_flags & FLAG_SCROLL_WIN_HORIZONTAL_SCROLL_ON)==FALSE)
		{
			ScrolledWindow->scroll_arrea_sizey=ScrolledWindow->ctrl_sizey-16-2;
			ScrolledWindow->scw_flags=ScrolledWindow->scw_flags | FLAG_SCROLL_WIN_HORIZONTAL_SCROLL_ON;
			HorizontalScrollBar->flags=HorizontalScrollBar->flags | FLAG_SHOW_CONTROL;
			HorizontalScrollBar->flags=HorizontalScrollBar->flags & FLAG_MOUSE_BLOCKED_OFF;
		}
	}

	if (ScrolledWindow->virtual_sizey>ScrolledWindow->scroll_arrea_sizey)
	{
		VerticalScrollBar=(struct	ControlScrollBar*)ScrolledWindow->vertical_scroll;

		if ((ScrolledWindow->scw_flags & FLAG_SCROLL_WIN_VERTICAL_SCROLL_ON)==FALSE)
		{			
			ScrolledWindow->scroll_arrea_sizex=ScrolledWindow->ctrl_sizex-16-2;
			ScrolledWindow->scw_flags=ScrolledWindow->scw_flags | FLAG_SCROLL_WIN_VERTICAL_SCROLL_ON;
			VerticalScrollBar->flags=VerticalScrollBar->flags | FLAG_SHOW_CONTROL;
			VerticalScrollBar->flags=VerticalScrollBar->flags & FLAG_MOUSE_BLOCKED_OFF;
		}
	}

	if (ScrolledWindow->scw_flags & FLAG_SCROLL_WIN_HORIZONTAL_SCROLL_ON)
	{
		HorizontalScrollBar->ruller_size=(float)ScrolledWindow->scroll_arrea_sizex;
		HorizontalScrollBar->ruller_size=HorizontalScrollBar->ruller_size/((float)ScrolledWindow->virtual_sizex);
	}

	if (ScrolledWindow->scw_flags & FLAG_SCROLL_WIN_VERTICAL_SCROLL_ON)
	{
		VerticalScrollBar->ruller_size=(float)ScrolledWindow->scroll_arrea_sizey;
		VerticalScrollBar->ruller_size=VerticalScrollBar->ruller_size/((float)ScrolledWindow->virtual_sizey);
	}

	//finit draw arrea for control
	fin=(struct FINITION*)control->finition;
	fin->x=ScrolledWindow->ctrl_x+1;
	fin->y=ScrolledWindow->ctrl_y+1;
	fin->sizex=ScrolledWindow->scroll_arrea_sizex;
	fin->sizey=ScrolledWindow->scroll_arrea_sizey;
	fin->flags=fin->flags | FINITION_ON;

}
////////////////////////////////////////////////////////////////////////////////
//				Draw full Scrolled Window
////////////////////////////////////////////////////////////////////////////////
void	ScrollWin_FuncCallback_HVScroll(struct HEADER* control,void *data)
{
	struct	ControlScrollBar		*Hscrollbar,*Vscrollbar;
	struct	ControlScrolledWindow 		*ScrolledWindow;
	struct	HEADER				*seek_control,*exchange_control;
	struct	MESSAGE				local_message;
	struct	FINITION			*fin;
	int					i,new_x,new_y,x,y,sizex,sizey;
	char					c;
	char					*save_buf,*buf;
	int					save_size_x,save_size_y;
	DWORD					draw_output;

	ScrolledWindow=(gui_scrolled_window_t*)data;
	Hscrollbar=(gui_scroll_bar_t*)ScrolledWindow->horizontal_scroll;
	Vscrollbar=(gui_scroll_bar_t*)ScrolledWindow->vertical_scroll;
	ScrolledWindow->virtual_x=(ScrolledWindow->virtual_sizex-ScrolledWindow->scroll_arrea_sizex)*Hscrollbar->ruller_pos;
	ScrolledWindow->virtual_y=(ScrolledWindow->virtual_sizey-ScrolledWindow->scroll_arrea_sizey)*Vscrollbar->ruller_pos;

	x=ScrolledWindow->ctrl_x+1;
	y=ScrolledWindow->ctrl_y+1;
	sizex=ScrolledWindow->scroll_arrea_sizex;
	sizey=ScrolledWindow->scroll_arrea_sizey;

	//alocate a buffer for draw text
	c=screen.bits_per_pixel >> 3;
	i=sizex*sizey*c;
	buf=malloc(i);

	//save current screen parameters
	save_buf=screen.buffer;
	save_size_x=screen.size_x;
	save_size_y=screen.size_y;
	draw_output=screen.draw_output;

	//load parameters of local buffer
	screen.buffer=buf;
	screen.size_x=sizex;
	screen.size_y=sizey;
	screen.draw_output=DRAW_OUTPUT_BUFFER;

	//fill buffer by background color
	FillArrea(buf,i,screen.bits_per_pixel,COLOR_LIGHT);

	local_message.type=MESSAGE_FULL_REDRAW_ALL_WITH_FINITION;
	local_message.arg1=0;
	local_message.arg2=0;
	local_message.arg3=sizex;
	local_message.arg4=sizey;

	seek_control=(struct HEADER*)Vscrollbar->ctrl_fd;
	//move controls in new position
	for(i=0;i<ScrolledWindow->number_virtual_controls;i++)
	{
		new_x=ScrolledWindow->virtual_controls_x[i]-ScrolledWindow->virtual_x;
		new_y=ScrolledWindow->virtual_controls_y[i]-ScrolledWindow->virtual_y;

		SetControlNewPosition(seek_control,new_x,new_y);

		if (CheckCrossRectangles(x,y,sizex,sizey,new_x,new_y,
					seek_control->ctrl_sizex,
					seek_control->ctrl_sizey)==TRUE)
		{
			seek_control->flags=seek_control->flags | FLAG_SHOW_CONTROL;
			seek_control->flags=seek_control->flags & FLAG_MOUSE_BLOCKED_OFF;

			//move control 
			SetControlNewPosition(seek_control,new_x-x,new_y-y);
			//call draw control in buffer
			ControlProc=(void (*)(void *Control,struct MESSAGE *message))seek_control->ctrl_proc;
			ControlProc(seek_control,&local_message);
			//restore last position of control
			SetControlNewPosition(seek_control,new_x,new_y);
			//restore coordinates of last finition of control
			fin=(finition_t*)seek_control->finition;
			fin->x=x;
			fin->y=y;
		}
		else
		{
			seek_control->flags=seek_control->flags & FLAG_HIDE_CONTROL;
			seek_control->flags=seek_control->flags | FLAG_MOUSE_BLOCKED_ON;
		}

		exchange_control=(struct HEADER*)seek_control->ctrl_fd;
		seek_control=exchange_control;
	}
	//restore screen parameters
	screen.buffer=save_buf;
	screen.size_x=save_size_x;
	screen.size_y=save_size_y;
	screen.draw_output=draw_output;

	//move rendered objects from local buffer to screen
	fin=(finition_t*)ScrolledWindow->finition;
	if (fin->flags & FINITION_ON)
		DrawImageFinit(fin,x,y,sizex,sizey,screen.bits_per_pixel,buf);
	else
		DrawImage(x,y,sizex,sizey,screen.bits_per_pixel,buf);

	//free local buffer
	free(buf);
}

void DrawScrolledWindow(struct ControlScrolledWindow *ScrolledWindow)
{
	int			x,y,sizex,sizey;
	struct FINITION 	*fin;

	x=ScrolledWindow->ctrl_x;
	y=ScrolledWindow->ctrl_y;
	sizex=ScrolledWindow->ctrl_sizex;
	sizey=ScrolledWindow->ctrl_sizey;
	fin=(struct FINITION*)ScrolledWindow->finition;

	if ((ScrolledWindow->scw_flags & FLAG_SCROLL_WIN_HORIZONTAL_SCROLL_ON) ||
		(ScrolledWindow->scw_flags & FLAG_SCROLL_WIN_VERTICAL_SCROLL_ON))
	{
		Draw(fin,TOOL_RECTANGLE,x,y,ScrolledWindow->scroll_arrea_sizex+2,
		ScrolledWindow->scroll_arrea_sizey+2,COLOR_ABSOLUTE_DARK);
	}
	else
		Draw(fin,TOOL_RECTANGLE,x,y,sizex,sizey,COLOR_ABSOLUTE_DARK);

	ScrollWin_FuncCallback_HVScroll(NULL,ScrolledWindow);
}

void	ScrlWinCheckActivatedForKeysControl(struct ControlScrolledWindow *ScrolledWindow)
{
	struct	HEADER				*control,*seek_control,*exchange_control;
	struct	MESSAGE			local_message;
	struct	FINITION			*fin;
	struct ControlScrollBar		*Vscrollbar,*Hscrollbar;
	int					i,x,y,sizex,sizey;
	int					sx,sy;

	control=(struct HEADER*)ScrolledWindow->active_control_for_keys;

	x=ScrolledWindow->ctrl_x+1;
	y=ScrolledWindow->ctrl_y+1;
	sizex=ScrolledWindow->scroll_arrea_sizex;
	sizey=ScrolledWindow->scroll_arrea_sizey;

	if (CheckFullCrossRectangles(x,y,sizex,sizey,
		control->ctrl_x,control->ctrl_y,control->ctrl_sizex,control->ctrl_sizey)==TRUE)	return;

	//calculate new x and y coordinates
	if (ScrolledWindow->scw_flags & FLAG_SCROLL_WIN_HORIZONTAL_SCROLL_ON)
	{
		sx=(control->ctrl_x-x);

		if (sx<0)	sx=x;
		else
		{
			if (control->ctrl_sizex<sizex)	sx=x+sizex-control->ctrl_sizex;
					else			sx=x;
		}
	}

	if (ScrolledWindow->scw_flags & FLAG_SCROLL_WIN_VERTICAL_SCROLL_ON)
	{
		sy=(control->ctrl_y-y);

		if (sy<0)	sy=y;
		else
		{
			if (control->ctrl_sizey<sizey)	sy=y+sizey-control->ctrl_sizey;
					else			sy=y;
		}
	}
	
	Vscrollbar=(struct ControlScrollBar*)ScrolledWindow->vertical_scroll;
	Hscrollbar=(struct ControlScrollBar*)ScrolledWindow->horizontal_scroll;
	//find active control and virtual control coordinates
	seek_control=(struct HEADER*)Vscrollbar->ctrl_fd;

	for(i=0;i<ScrolledWindow->number_virtual_controls;i++)
	{
		if (seek_control==control)
		{
			if (ScrolledWindow->scw_flags & FLAG_SCROLL_WIN_HORIZONTAL_SCROLL_ON)
					ScrolledWindow->virtual_x=ScrolledWindow->virtual_controls_x[i]-sx;
			if (ScrolledWindow->scw_flags & FLAG_SCROLL_WIN_VERTICAL_SCROLL_ON)
					ScrolledWindow->virtual_y=ScrolledWindow->virtual_controls_y[i]-sy;
			break;
		}

		exchange_control=(struct HEADER*)seek_control->ctrl_fd;
		seek_control=exchange_control;
	}

	if (ScrolledWindow->scw_flags & FLAG_SCROLL_WIN_HORIZONTAL_SCROLL_ON)
	{
		Hscrollbar->ruller_pos=(float)ScrolledWindow->virtual_x;
		Hscrollbar->ruller_pos=Hscrollbar->ruller_pos/((float)(ScrolledWindow->virtual_sizex-ScrolledWindow->scroll_arrea_sizex));
		SpecialRedrawControl(Hscrollbar);
	}
	if (ScrolledWindow->scw_flags & FLAG_SCROLL_WIN_VERTICAL_SCROLL_ON)
	{
		Vscrollbar->ruller_pos=(float)ScrolledWindow->virtual_y;
		Vscrollbar->ruller_pos=Vscrollbar->ruller_pos/((float)(ScrolledWindow->virtual_sizey-ScrolledWindow->scroll_arrea_sizey));
		SpecialRedrawControl(Vscrollbar);
	}
	ScrollWin_FuncCallback_HVScroll(NULL,ScrolledWindow);
}

//---------------------------------------------------------------------------------
//			 control ScrolledWindowScrolledWindow->virtual_sizex
//---------------------------------------------------------------------------------
void ScrolledWindowProc(struct ControlScrolledWindow *ScrolledWindow,struct MESSAGE *message)
{
	int				i,x,y,sizex,sizey;
	struct	HEADER			*seek_control,*exchange_control;
	struct ControlScrollBar		*Hscrollbar,*Vscrollbar;
	struct	MESSAGE			local_message;
	struct	FINITION		*fin;
	struct TIMER			*timer;

	x=ScrolledWindow->ctrl_x;
	y=ScrolledWindow->ctrl_y;
	sizex=ScrolledWindow->ctrl_sizex;
	sizey=ScrolledWindow->ctrl_sizey;

	switch(message->type)
	{
		case MESSAGE_FULL_REDRAW_ALL:
		{
			//draw ScrolledWindow
			if (ScrolledWindow->flags & FLAG_SHOW_CONTROL)
			{
				DrawScrolledWindow(ScrolledWindow);
				Hscrollbar=(gui_scroll_bar_t*)ScrolledWindow->horizontal_scroll;
				Vscrollbar=(gui_scroll_bar_t*)ScrolledWindow->vertical_scroll;
				//draw scroll bars
				ControlProc=(void (*)(void *Control,gui_message_t *message))Hscrollbar->ctrl_proc;
				ControlProc(Hscrollbar,message);
				ControlProc=(void (*)(void *Control,gui_message_t *message))Vscrollbar->ctrl_proc;
				ControlProc(Vscrollbar,message);
			}
			break;
		}
		case MESSAGE_FULL_REDRAW_ALL_WITH_FINITION:
		{
			fin=(struct FINITION*)ScrolledWindow->finition;
			fin->flags=fin->flags | FINITION_ON;
			fin->x=message->arg1;
			fin->y=message->arg2;
			fin->sizex=message->arg3;
			fin->sizey=message->arg4;
			DrawScrolledWindow(ScrolledWindow);
			SendMessage((struct HEADER*)ScrolledWindow,message);//<<<<<<----------------------------------
			break;
		}
		case MESSAGE_SPECIALIZED:
		{
			if (ScrolledWindow->flags & FLAG_SHOW_CONTROL)	SendMessage((struct HEADER*)ScrolledWindow,message);
			ScrolledWindow->flags=ScrolledWindow->flags & FLAG_GET_SPECIALIZED_MESSAGE_OFF;
			break;
		}

		case MESSAGE_KEYS_EVENT:
		{
			if (ScrolledWindow->active_control_for_keys!=NULL) ScrlWinCheckActivatedForKeysControl(ScrolledWindow);
			SendMessage((struct HEADER*)ScrolledWindow,message);
			break;			
		}
		case MESSAGE_MOUSE_EVENT:
		{	
			SendMessage((struct HEADER*)ScrolledWindow,message);
			break;
		}
		case MESSAGE_CHANGE_POSITION_EVENT:
		{
			ScrolledWindow->ctrl_x=ScrolledWindow->ctrl_x+message->arg1;
			ScrolledWindow->ctrl_y=ScrolledWindow->ctrl_y+message->arg2;

			//change virtual coordinates of controls
			Vscrollbar=(struct ControlScrollBar*)ScrolledWindow->vertical_scroll;
			seek_control=(struct HEADER *)Vscrollbar->ctrl_fd;
			for(i=0;i<ScrolledWindow->number_virtual_controls;i++)
			{
				ScrolledWindow->virtual_controls_x[i]+=message->arg1;
				ScrolledWindow->virtual_controls_y[i]+=message->arg2;

				fin=(struct FINITION*)seek_control->finition;
				fin->x=ScrolledWindow->ctrl_x+1;
				fin->y=ScrolledWindow->ctrl_y+1;
				fin->sizex=ScrolledWindow->scroll_arrea_sizex;
				fin->sizey=ScrolledWindow->scroll_arrea_sizey;

				exchange_control=(struct HEADER*)seek_control->ctrl_fd;
				seek_control=exchange_control;
			}

			SendMessage((struct HEADER*)ScrolledWindow,message);
			break;
		}
		case MESSAGE_CALL_TIMER_EVENT:
		{
			if (ScrolledWindow->timer!=(DWORD*)NULL)
			{
				timer=(struct TIMER*)ScrolledWindow->timer;
				if (timer->flags & FLAG_TIMER_ON)	Timer(timer);
			}
			SendMessage((struct HEADER*)ScrolledWindow,message);
			break;
		}
		case MESSAGE_SET_FOCUSE:
		{
			//SendMessage((struct HEADER*)ScrolledWindow,message);
			break;
		}
		case MESSAGE_CHANGE_FOCUSE:
		{
			//SendMessage((struct HEADER*)ScrolledWindow,message);
			break;
		}
		case MESSAGE_DESTROY_CONTROL:
		{
			if (ScrolledWindow->timer!=(DWORD*)NULL)	free(ScrolledWindow->timer);
			free(ScrolledWindow->finition);
			SendMessage((struct HEADER*)ScrolledWindow,message);
			break;
		}
		case MESSAGE_SET_MAIN_PARENT:
		{
			SendMessage((struct HEADER*)ScrolledWindow,message);
			ScrolledWindow->main_parent=(DWORD*)message->arg1;
			break;
		}

		default: break;
	}
}

//---------------------------------------------------------------------------------
//				create control ScrolledWindow
//---------------------------------------------------------------------------------
void* CreateScrolledWindow(struct ScrolledWindowData *info_for_control)
{
	struct ControlScrolledWindow	*ScrolledWindow;
	struct FINITION			*fin;
	struct	ControlScrollBar		*HorizontalScrollBar;
	struct	ControlScrollBar		*VerticalScrollBar;
	struct	ScrollBarData			HorizontalScrollData;
	struct	ScrollBarData			VerticalScrollData;


	ScrolledWindow=malloc(sizeof(struct ControlScrolledWindow));
	ScrolledWindow->finition=malloc(sizeof(struct FINITION));
	fin=(struct FINITION*)ScrolledWindow->finition;
	fin->flags=0;
	ScrolledWindow->scw_flags=0;

	ID++;
#ifdef	DEBUG
	printf("\ncreated scrollet window with ID=%d",(int)ID);
#endif
	ScrolledWindow->child_bk=(DWORD*)NULL;
	ScrolledWindow->child_fd=(DWORD*)NULL;
	ScrolledWindow->active_control_for_keys=(DWORD*)NULL;
	ScrolledWindow->active_control_for_mouse=(DWORD*)NULL;
	ScrolledWindow->callback=(DWORD*)NULL;
	ScrolledWindow->timer=(DWORD*)NULL;

	ScrolledWindow->ctrl_proc=(DWORD*)&ScrolledWindowProc;
	ScrolledWindow->ctrl_x=(DWORD)info_for_control->x;
	ScrolledWindow->ctrl_y=(DWORD)info_for_control->y;
	ScrolledWindow->ctrl_sizex=(DWORD)info_for_control->width;
	ScrolledWindow->ctrl_sizey=(DWORD)info_for_control->height;
	ScrolledWindow->ctrl_ID=ID;
	ScrolledWindow->virtual_x=0;
	ScrolledWindow->virtual_y=0;
	ScrolledWindow->virtual_controls_x=malloc(1024*sizeof(DWORD));
	ScrolledWindow->virtual_controls_y=malloc(1024*sizeof(DWORD));
	ScrolledWindow->virtual_sizex=0;
	ScrolledWindow->virtual_sizey=0;
	ScrolledWindow->number_virtual_controls=0;
	ScrolledWindow->flags=0;
	ScrolledWindow->flags=ScrolledWindow->flags | FLAG_SHOW_CONTROL;
	ScrolledWindow->flags=ScrolledWindow->flags | FLAG_FOCUSE_INPUT_SUPPOROTE;

	//calculate default scroll arrea size
	ScrolledWindow->scroll_arrea_sizex=ScrolledWindow->ctrl_sizex-2;
	ScrolledWindow->scroll_arrea_sizey=ScrolledWindow->ctrl_sizey-2;

	//create child scroll bars
	HorizontalScrollData.x=0;
	HorizontalScrollData.y=ScrolledWindow->ctrl_sizey-16;
	HorizontalScrollData.width=ScrolledWindow->ctrl_sizex-16;
	HorizontalScrollData.height=16;
	HorizontalScrollData.ruller_size=1.0;
	HorizontalScrollData.ruller_pos=0.0;
	HorizontalScrollData.ruller_step=0.05;

	VerticalScrollData.x=ScrolledWindow->ctrl_sizex-16;
	VerticalScrollData.y=0;
	VerticalScrollData.width=16;
	VerticalScrollData.height=ScrolledWindow->ctrl_sizey-16;
	VerticalScrollData.ruller_size=1.0;
	VerticalScrollData.ruller_pos=0.0;
	VerticalScrollData.ruller_step=0.05;

	HorizontalScrollBar=CreateHorizontalScrollBar(&HorizontalScrollData);
	VerticalScrollBar=CreateVerticalScrollBar(&VerticalScrollData);

	SetCallbackFunction(HorizontalScrollBar,SCROLLBAR_CHANGED_EVENT,&ScrollWin_FuncCallback_HVScroll,ScrolledWindow);
	SetCallbackFunction(VerticalScrollBar,SCROLLBAR_CHANGED_EVENT,&ScrollWin_FuncCallback_HVScroll,ScrolledWindow);

	PackControls(ScrolledWindow,HorizontalScrollBar);
	PackControls(ScrolledWindow,VerticalScrollBar);

	ScrolledWindow->horizontal_scroll=(DWORD*)HorizontalScrollBar;
	ScrolledWindow->vertical_scroll=(DWORD*)VerticalScrollBar;
	//disable show scrollers and block mouse for them
	HorizontalScrollBar->flags=HorizontalScrollBar->flags & FLAG_HIDE_CONTROL;
	HorizontalScrollBar->flags=HorizontalScrollBar->flags | FLAG_MOUSE_BLOCKED_ON;
	VerticalScrollBar->flags=VerticalScrollBar->flags & FLAG_HIDE_CONTROL;
	VerticalScrollBar->flags=VerticalScrollBar->flags | FLAG_MOUSE_BLOCKED_ON;

	return(ScrolledWindow);
}

