/*
	libGUI main part of code
*/

//---------------------------------------------------------------------------------
//				destroy_control for libGUI
//---------------------------------------------------------------------------------
void DestroyControl(void *control_ptr)
{
	struct HEADER *Parent;
	struct HEADER *control;
	struct HEADER *seek_control;
	struct HEADER *exchange_control;
	struct HEADER *last_control;
	struct HEADER *next_control;

	if (control_ptr==NULL) return;

	control=control_ptr;
	Parent=(struct HEADER *)control->parent;

	if (Parent->child_bk==Parent->child_fd)
	{
		//parent have got alone child control
		if (Parent->child_bk==(DWORD*)control)
		{
			//current control is child control of parent
#ifdef	DEBUG
			printf("\ndestroyed control=%d parent=%d ID=%d",(int)control,(int)Parent,(int)control->ctrl_ID);
#endif
			free(control);
			Parent->child_bk=(DWORD*)NULL;
			Parent->child_fd=(DWORD*)NULL;
		}
		return;
	}

	seek_control=(struct HEADER*)Parent->child_bk;
	while(seek_control!=(struct HEADER*)NULL)
	{
		if (seek_control==control)
		{
			//delete control from control's stack
			last_control=(struct HEADER*)seek_control->ctrl_bk;
			next_control=(struct HEADER*)seek_control->ctrl_fd;

			if ((last_control!=Parent) && (next_control!=NULL))
			{//deliting control isn't first control and isn't endest of parent
				next_control->ctrl_bk=(DWORD*)last_control;
				last_control->ctrl_fd=(DWORD*)next_control;
			}
			else
			{
				if (last_control==Parent)
				{//deliting control is first control of Parend
					Parent->child_bk=(DWORD*)next_control;
					next_control->ctrl_bk=(DWORD*)Parent;
					if (next_control->ctrl_fd==(DWORD*)NULL)
					{
						Parent->child_fd=(DWORD*)next_control;
						Parent->child_bk=(DWORD*)next_control;
					}
				}

				if (next_control==(struct HEADER*)NULL)
				{
					//there isn't next controls
					last_control->ctrl_fd=(DWORD*)NULL;
					Parent->ctrl_fd=(DWORD*)last_control;
				}
			}
#ifdef	DEBUG
			printf("\ndestroyed control=%d parent=%d ID=%d",(int)control,(int)Parent,(int)control->ctrl_ID);
#endif
			free(control);
			break;
		}
		exchange_control=(struct HEADER*)seek_control->ctrl_fd;
		seek_control=exchange_control;
	}
}

//---------------------------------------------------------------------------------
//			create new timer for parent window
//---------------------------------------------------------------------------------
void	*CreateTimerForParentWindow(parent_t *parent)
{
	struct	TIMER		*timer;
	struct	TIMER		*backward_timer;

	timer=malloc(sizeof(struct	TIMER));
	timer->tmr_parent=(DWORD*)parent;

	if (parent->timer_bk==(DWORD*)NULL)
	{//not yet timers
		parent->timer_bk=(DWORD*)timer;
		parent->timer_fd=(DWORD*)timer;
		timer->tmr_bk=(DWORD*)parent;
		timer->tmr_fd=(DWORD*)NULL;
	}
	else
	{
		backward_timer=(struct TIMER*)parent->timer_fd;
		parent->timer_fd=(DWORD*)timer;
		backward_timer->tmr_fd=(DWORD*)timer;
		timer->tmr_bk=(DWORD*)backward_timer;
		timer->tmr_fd=(DWORD*)NULL;
	}
	return(timer);
}

//---------------------------------------------------------------------------------
//				destroy timer for parent window
//---------------------------------------------------------------------------------
void DestroyTimerCallbackForFunction(struct TIMER *timer)
{
	struct	HEADERPARENT	*Parent;
	struct TIMER		*seek_timer;
	struct TIMER		*exchange_timer;
	struct TIMER		*last_timer;
	struct TIMER		*next_timer;

	if (timer==(struct TIMER*)NULL) return;

	Parent=(struct HEADERPARENT *)timer->tmr_parent;

	if (Parent->timer_bk==Parent->timer_fd)
	{
		//parent have got alone timer
		if (Parent->timer_bk==(DWORD*)timer)
		{
			Parent->timer_bk=(DWORD*)NULL;
			Parent->timer_fd=(DWORD*)NULL;
#ifdef	DEBUG
			printf("\ndestroyed timer %d parent window %d",(int)timer,(int)timer->tmr_parent);
#endif
			free(timer);
			return;
		}
		return;
	}

	seek_timer=(struct TIMER*)Parent->timer_bk;
	while(seek_timer!=(struct TIMER*)NULL)
	{
		if (seek_timer==timer)
		{
			//delete timer from timers's stack
			last_timer=(struct TIMER*)seek_timer->tmr_bk;
			next_timer=(struct TIMER*)seek_timer->tmr_fd;

			if ((last_timer!=(struct TIMER*)Parent) && (next_timer!=(struct TIMER*)NULL))
			{//deliting timer isn't first timer and isn't endest
				next_timer->tmr_bk=(DWORD*)last_timer;
				last_timer->tmr_fd=(DWORD*)next_timer;
			}
			else
			{
				if (last_timer==(struct TIMER*)Parent)
				{//deliting timer is first timer of Parend
					Parent->timer_bk=(DWORD*)next_timer;
					next_timer->tmr_bk=(DWORD*)Parent;
					if (next_timer->tmr_fd==(DWORD*)NULL)
					{
						Parent->timer_fd=(DWORD*)next_timer;
						Parent->timer_bk=(DWORD*)next_timer;
					}
				}

				if (next_timer==(struct TIMER*)NULL)
				{
					//there isn't next controls
					last_timer->tmr_fd=(DWORD*)NULL;
					Parent->timer_fd=(DWORD*)last_timer;
				}
			}
#ifdef	DEBUG
			printf("\ndestroyed timer %d parent window %d",(int)timer,(int)timer->tmr_parent);
#endif
			free(timer);
			break;
		}
		exchange_timer=(struct TIMER*)seek_timer->tmr_fd;
		seek_timer=exchange_timer;
	}
}

//---------------------------------------------------------------------------------
//			create timer for function of parent window
//---------------------------------------------------------------------------------
void	*SetTimerCallbackForFunction(parent_t *parent_window,int time_tick,void *func,void *func_data)
{
	struct	TIMER		*timer;

	timer=CreateTimerForParentWindow(parent_window);
	timer->func=(DWORD*)func;
	timer->func_data=(DWORD*)func_data;
	timer->time_tick=(DWORD)time_tick;
	timer->last_time=gui_ksys_get_ticks();
	timer->flags=timer->flags | FLAG_TIMER_ON;

	return(timer);
}

//---------------------------------------------------------------------------------
//				create timer for control
//---------------------------------------------------------------------------------
void	*SetTimerCallbackForControl(int time_tick,void *func,void *func_data)
{
	struct	TIMER		*timer;

	timer=malloc(sizeof(struct TIMER));
	timer->func=(DWORD*)func;
	timer->func_data=(DWORD*)func_data;
	timer->time_tick=(DWORD)time_tick;
	timer->last_time=gui_ksys_get_ticks();
	timer->flags=timer->flags & FLAG_TIMER_OFF;

	return(timer);
}

//---------------------------------------------------------------------------------
//					call timer
//---------------------------------------------------------------------------------
void	Timer(struct TIMER *timer)
{
	DWORD	time,delta_time;

	time=gui_ksys_get_ticks();
	delta_time=time-timer->last_time;
	if (delta_time>=timer->time_tick)
	{
		TimerCallbackFunction=(void(*)(void *data))timer->func;
		TimerCallbackFunction(timer->func_data);
		timer->last_time=gui_ksys_get_ticks();
	}
}

//---------------------------------------------------------------------------------
//			create new callback for event
//---------------------------------------------------------------------------------
void	*CreateCallbackForEvent(parent_t *parent)
{
	gui_callbackevent_t	*calev;
	gui_callbackevent_t	*backward_calev;

	calev=malloc(sizeof(gui_callbackevent_t));
	calev->calev_parent=(DWORD*)parent;

	if (parent->calev_bk==(DWORD*)NULL)
	{//not yet timers
		parent->calev_bk=(DWORD*)calev;
		parent->calev_fd=(DWORD*)calev;
		calev->calev_bk=(DWORD*)parent;
		calev->calev_fd=(DWORD*)NULL;
	}
	else
	{
		backward_calev=(gui_callbackevent_t*)parent->calev_fd;
		parent->calev_fd=(DWORD*)calev;
		backward_calev->calev_fd=(DWORD*)calev;
		calev->calev_bk=(DWORD*)backward_calev;
		calev->calev_fd=(DWORD*)NULL;
	}
	return(calev);
}

//---------------------------------------------------------------------------------
//			destroy callback function for eventcalev
//---------------------------------------------------------------------------------
void DestroyCallbackFunctionForEvent(gui_callbackevent_t *calev)
{
	struct	HEADERPARENT	*Parent;
	struct CALLBACKEVENT	*seek_calev;
	struct CALLBACKEVENT	*exchange_calev;
	struct CALLBACKEVENT	*last_calev;
	struct CALLBACKEVENT	*next_calev;

	if (calev==(gui_callbackevent_t*)NULL)	return;

	Parent=(parent_t*)calev->calev_parent;

	if (Parent->calev_bk==Parent->calev_fd)
	{
		//parent have got alone timer
		if (Parent->calev_bk==(DWORD*)calev)
		{
			free(calev);
			Parent->calev_bk=(DWORD*)NULL;
			Parent->calev_fd=(DWORD*)NULL;
		}
		return;
	}

	seek_calev=(struct CALLBACKEVENT*)Parent->calev_bk;
	while(seek_calev!=(struct CALLBACKEVENT*)NULL)
	{
		if (seek_calev==calev)
		{
			//delete timer from timers's stack
			last_calev=(struct CALLBACKEVENT*)seek_calev->calev_bk;
			next_calev=(struct CALLBACKEVENT*)seek_calev->calev_fd;

			if ((last_calev!=(struct CALLBACKEVENT*)Parent) && (next_calev!=(struct CALLBACKEVENT*)NULL))
			{//deliting timer isn't first timer and isn't endest
				next_calev->calev_bk=(DWORD*)last_calev;
				last_calev->calev_fd=(DWORD*)next_calev;
			}
			else
			{
				if (last_calev==(struct CALLBACKEVENT*)Parent)
				{//deliting timer is first timer of Parend
					Parent->calev_bk=(DWORD*)next_calev;
					next_calev->calev_bk=(DWORD*)Parent;
					if (next_calev->calev_fd==(DWORD*)NULL)
					{
						Parent->calev_fd=(DWORD*)next_calev;
						Parent->calev_bk=(DWORD*)next_calev;
					}
				}

				if (next_calev==(struct CALLBACKEVENT*)NULL)
				{
					//there isn't next controls
					last_calev->calev_fd=(DWORD*)NULL;
					Parent->calev_fd=(DWORD*)last_calev;
				}
			}
			free(calev);
			break;
		}
		exchange_calev=(struct CALLBACKEVENT*)seek_calev->calev_fd;
		seek_calev=exchange_calev;
	}
}

//---------------------------------------------------------------------------------
//		create callback for messenger of events
//---------------------------------------------------------------------------------
void	*SetCallbackFunctionForEvent(parent_t *parent_window,int event_type,void *func,void *func_data)
{
	gui_callbackevent_t	*calev;

	calev=CreateCallbackForEvent(parent_window);
	calev->func=(DWORD*)func;
	calev->func_data=(DWORD*)func_data;
	calev->event_type=(DWORD)event_type;

	return(calev);
}

//---------------------------------------------------------------------------------
//				check cross control and mouse
//---------------------------------------------------------------------------------
char CheckCrossBox(struct HEADER *control,int mouse_x,int mouse_y)
{
	struct FINITION *fin;
	int			x,y,x2,y2;
	int			xmin,xmax,ymin,ymax,sx,sy;

	fin=(struct FINITION*)control->finition;

	if (fin->flags & FINITION_ON)
	{
		xmin=fin->x;
		xmax=fin->x+fin->sizex-1;
		ymin=fin->y;
		ymax=fin->y+fin->sizey-1;

		x=control->ctrl_x;
		y=control->ctrl_y;
		x2=x+control->ctrl_sizex-1;
		y2=y+control->ctrl_sizey-1;

		if (x2<xmin) return(FALSE);
		if (x>xmax) return(FALSE);
		if (y2<ymin) return(FALSE);
		if (y>ymax) return(FALSE);

		//finit x coordinates and sizex
		sx=x-xmin;

		if (sx>=0)
		{
			if (x2>xmax) x2=xmax;
		}
		else
		{
			x=xmin;
			if (x2>xmax) x2=xmax;
		}

		//finit y coordinates and sizey
		sy=y-ymin;

		if (sy>=0)
		{
			if (y2>ymax) y2=ymax;
		}
		else
		{
			y=ymin;
			if (y2>ymax) y2=ymax;
		}
		//check cross finited control with mouse
		if ((mouse_x>=x) && (mouse_x<=x2) && (mouse_y>=y) && (mouse_y<=y2))
		return(TRUE);
		else
		return(FALSE);
	}
	else
	{
		if ((mouse_x>=control->ctrl_x) && (mouse_x<=control->ctrl_x+control->ctrl_sizex) &&
			(mouse_y>=control->ctrl_y) && (mouse_y<=control->ctrl_y+control->ctrl_sizey))
		return(TRUE);
		else
		return(FALSE);
	}
}

//---------------------------------------------------------------------------------
//				check cross two rectaangles
//---------------------------------------------------------------------------------
char	CheckCrossRectangles(int x1,int y1,int sizex1,int sizey1,int x2,int y2,int sizex2,int sizey2)
{
	int	s;
	int	xmax,ymax;

	xmax=x1+sizex1-1;
	ymax=y1+sizey1-1;

	//check x cross
	s=x2-x1;
	if (s>0)
	{//second rectangle have right position
		if (x2>xmax) return(FALSE);
	}
	if (s<0)
	{//second rectangle have left position
		s=-s;
		if (s>=sizex2) return(FALSE);
	}

	//check y cross
	s=y2-y1;
	if (s>0)
	{//second rectangle have down position
		if (y2>ymax) return(FALSE);
	}
	if (s<0)
	{//second rectangle have up position
		s=-s;
		if (s>=sizey2) return(FALSE);
	}
	return(TRUE);
}

//---------------------------------------------------------------------------------
//				check full cross two rectaangles
//---------------------------------------------------------------------------------
char	CheckFullCrossRectangles(int x1,int y1,int sizex1,int sizey1,int x2,int y2,int sizex2,int sizey2)
{
	int	s;
	int	xmax,ymax;

	xmax=x1+sizex1-1;
	ymax=y1+sizey1-1;

	if (x2>=x1 && x2+sizex2-1<=xmax && y2>=y1 && y2+sizey2-1<=ymax) return(TRUE);
		else	return(FALSE);
}

//---------------------------------------------------------------------------------
//				send message to controls
//---------------------------------------------------------------------------------
void	SendMessage(struct HEADER *Parent,struct MESSAGE *message)
{
	struct	HEADER		*seek_control;
	struct	HEADER		*exchange_control;
	struct	HEADER		*parent;
	struct	HEADERPARENT	*main_parent;
	struct	MESSAGE local_service_message;
	struct	HEADER		*active_control_for_keys;
	struct	HEADER		*active_control_for_mouse;
	char			cross;

	//Parent haven't got child controls
	if ((Parent->child_bk==NULL) && (Parent->child_fd==NULL)) return;

	//load main parent
	main_parent=(struct HEADERPARENT*)Parent->main_parent;

	//load active controls from Parent
	active_control_for_keys=(struct HEADER*)Parent->active_control_for_keys;
	active_control_for_mouse=(struct HEADER*)Parent->active_control_for_mouse;

	//Parent have got alon child control
	if (Parent->child_bk==Parent->child_fd)
	{
		seek_control=(struct HEADER *)Parent->child_bk;
		ControlProc=(void (*)(void *Control,struct MESSAGE *message))seek_control->ctrl_proc;

		switch(message->type)
		{
			case MESSAGE_FULL_REDRAW_ALL:
			{
				//send message redraw to all child controls
				if (seek_control->flags & FLAG_SHOW_CONTROL)	ControlProc(seek_control,message);
				break;
			}
			case MESSAGE_FULL_REDRAW_ALL_WITH_FINITION:
			{
				//send message finited redraw to all child controls
				if (seek_control->flags & FLAG_SHOW_CONTROL)	ControlProc(seek_control,message);
				break;
			}
			case MESSAGE_KEYS_EVENT:
			{	//change active control for keyboard by TAB press
				if ((message->arg1==KEY_DOWN) && (message->arg2==SC_TAB))
				{
					if (seek_control->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)
					{
						if (active_control_for_keys==seek_control)
						{//focuse of input set for current control
							ControlProc=(void (*)(void *Control,struct MESSAGE *message))active_control_for_keys->ctrl_proc;
							local_service_message.type=MESSAGE_CHANGE_FOCUSE;
							local_service_message.arg1=FALSE;
							active_control_for_keys->flags=active_control_for_keys->flags & FLAG_FOCUSE_INPUT_OFF;
							ControlProc(active_control_for_keys,&local_service_message);

							parent=(struct HEADER*)active_control_for_keys->parent;
							
							seek_control=parent;
							do
							{
								if (parent==(struct HEADER*)main_parent) break;
								//if next control NULL go to parent
								if (seek_control==(struct HEADER*)NULL) seek_control=parent;

								exchange_control=(struct HEADER *)seek_control->ctrl_fd;
								seek_control=exchange_control;
								if (seek_control==(struct HEADER*)NULL)
													parent=(struct HEADER*)parent->parent;
							}
							while((seek_control==(struct HEADER*)NULL) ||
										((seek_control->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)==FALSE));

							if (parent==(struct HEADER*)main_parent)
							{
								//find first control of main parent with focuse of input supporote
								seek_control=(struct HEADER *)main_parent->child_bk;
								while(seek_control!=(struct HEADER*)NULL)
								{
									if (seek_control->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)	break;
									exchange_control=(struct HEADER *)seek_control->ctrl_fd;
									seek_control=exchange_control;
								}
								main_parent->active_control_for_keys=(DWORD*)seek_control;
								main_parent->global_active_control_for_keys=(DWORD*)seek_control;

								//send message enable focuse of input to control
								ControlProc=(void (*)(void *Control,struct MESSAGE *message))
												seek_control->ctrl_proc;
								local_service_message.type=MESSAGE_SET_FOCUSE;
								local_service_message.arg1=FALSE;
								seek_control->flags=seek_control->flags | FLAG_FOCUSE_INPUT_ON;
								ControlProc(seek_control,&local_service_message);

								active_control_for_keys=(struct HEADER*)NULL;
								break;
							}
							if (seek_control!=(struct HEADER*)NULL)
							{
								parent=(struct HEADER*)seek_control->parent;
								parent->active_control_for_keys=(DWORD*)seek_control;
								main_parent->global_active_control_for_keys=(DWORD*)seek_control;

								//send message enable focuse of input to control
								ControlProc=(void (*)(void *Control,struct MESSAGE *message))
												seek_control->ctrl_proc;
								local_service_message.type=MESSAGE_SET_FOCUSE;
								local_service_message.arg1=FALSE;
								seek_control->flags=seek_control->flags | FLAG_FOCUSE_INPUT_ON;
								ControlProc(seek_control,&local_service_message);

								active_control_for_keys=(struct HEADER*)NULL;
								Parent->active_control_for_keys=(DWORD*)active_control_for_keys;
								break;
							}
						}
						else
						{
							ControlProc=(void (*)(void *Control,struct MESSAGE *message))seek_control->ctrl_proc;
							local_service_message.type=MESSAGE_SET_FOCUSE;
							local_service_message.arg1=FALSE;
							seek_control->flags=seek_control->flags | FLAG_FOCUSE_INPUT_ON;
							ControlProc(seek_control,&local_service_message);

							active_control_for_keys=seek_control;
							Parent->active_control_for_keys=(DWORD*)active_control_for_keys;
							main_parent->global_active_control_for_keys=(DWORD*)seek_control;
						}
					}
				}

				//send message of keys only to active control
				if (active_control_for_keys!=NULL)
				{
					ControlProc=(void (*)(void *Control,struct MESSAGE *message))active_control_for_keys->ctrl_proc;
					ControlProc(active_control_for_keys,message);
				}
				break;
			}
			case MESSAGE_SPECIALIZED:
			{
				//send specialized message to all child controls
				ControlProc(seek_control,message);
				break;
			}
			case MESSAGE_MOUSE_EVENT:
			{
				if (seek_control->flags & FLAG_MOUSE_BLOCKED_ON) break;

				if (message->arg3==MOUSE_LEFT_BUTTON_DOWN)
				{
					if (active_control_for_mouse!=NULL)
					{
						ControlProc=(void (*)(void *Control,struct MESSAGE *message))active_control_for_mouse->ctrl_proc;
						ControlProc(active_control_for_mouse,message);
						break;
					}
				}
				else
				{
					if (active_control_for_mouse!=NULL) {active_control_for_mouse=NULL;}
				}

				cross=FALSE;
				if (CheckCrossBox(seek_control,message->arg1,message->arg2)==TRUE)
				{
					if (message->arg3==MOUSE_LEFT_BUTTON_DOWN)
					{
						cross=TRUE;
						active_control_for_mouse=seek_control;

						if (seek_control->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)
						{
							parent=(struct HEADER*)seek_control->parent;
							if (parent->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)
							{
								active_control_for_keys=(struct HEADER*)main_parent->global_active_control_for_keys;
								if (active_control_for_keys!=(struct HEADER*)NULL)
								{
									ControlProc=(void (*)(void *Control,struct MESSAGE *message))
												active_control_for_keys->ctrl_proc;

									local_service_message.type=MESSAGE_CHANGE_FOCUSE;
									active_control_for_keys->flags=
										active_control_for_keys->flags & FLAG_FOCUSE_INPUT_OFF;
									ControlProc(active_control_for_keys,&local_service_message);
								}

								ControlProc=(void (*)(void *Control,struct MESSAGE *message))
									seek_control->ctrl_proc;
								local_service_message.type=MESSAGE_SET_FOCUSE;
								seek_control->flags=seek_control->flags | FLAG_FOCUSE_INPUT_ON;
								ControlProc(seek_control,&local_service_message);

								main_parent->global_active_control_for_keys=(DWORD*)seek_control;
								Parent->active_control_for_keys=(DWORD*)seek_control;
							}
						}
					}
				}

				if ((cross==FALSE) && (message->arg3==MOUSE_LEFT_BUTTON_DOWN))
				{
					if (active_control_for_keys!=NULL)
					{//disable focuse of input for active control of keys
						ControlProc=(void (*)(void *Control,struct MESSAGE *message))active_control_for_keys->ctrl_proc;

						local_service_message.type=MESSAGE_CHANGE_FOCUSE;
						active_control_for_keys->flags=active_control_for_keys->flags & FLAG_FOCUSE_INPUT_OFF;
						ControlProc(active_control_for_keys,&local_service_message);
					}
				}

				ControlProc=(void (*)(void *Control,struct MESSAGE *message))seek_control->ctrl_proc;
				ControlProc(seek_control,message);
				break;
			}
			case MESSAGE_CHANGE_POSITION_EVENT:
			{
				ControlProc(seek_control,message);
				break;
			}
			case MESSAGE_CALL_TIMER_EVENT:
			{
				ControlProc(seek_control,message);
				break;
			}
			case MESSAGE_DESTROY_CONTROL:
			{
				//send message to control for destroing child controls
				ControlProc(seek_control,message);
				DestroyControl(seek_control);
				break;
			}
			case MESSAGE_SET_MAIN_PARENT:
			{
				ControlProc(seek_control,message);
				break;
			}
			default:break;
		}
	}
	else
	{
		switch(message->type)
		{
			case MESSAGE_FULL_REDRAW_ALL:
			{
				//send message redraw to all child controls of Parent
				seek_control=(struct HEADER *)Parent->child_bk;
				while(seek_control!=(struct HEADER*)NULL)
				{
					if (seek_control->flags & FLAG_SHOW_CONTROL)
					{
						ControlProc=(void (*)(void *Control,struct MESSAGE *message))seek_control->ctrl_proc;
						ControlProc(seek_control,message);
					}

					exchange_control=(struct HEADER *)seek_control->ctrl_fd;
					seek_control=exchange_control;
				}
				break;
			}
			case MESSAGE_FULL_REDRAW_ALL_WITH_FINITION:
			{
				//send message redraw to all child controls of Parent
				seek_control=(struct HEADER *)Parent->child_bk;
				while(seek_control!=(struct HEADER*)NULL)
				{
					if (seek_control->flags & FLAG_SHOW_CONTROL)
					{
						ControlProc=(void (*)(void *Control,struct MESSAGE *message))seek_control->ctrl_proc;
						ControlProc(seek_control,message);
					}

					exchange_control=(struct HEADER *)seek_control->ctrl_fd;
					seek_control=exchange_control;
				}
				break;
			}
			case MESSAGE_KEYS_EVENT:
			{
				if ((message->arg1==KEY_DOWN) && (message->arg2==SC_TAB))
				{	//change active control for keyboard by TAB press
					if (active_control_for_keys==NULL)
					{
						seek_control=(struct HEADER *)Parent->child_bk;

						if ((seek_control->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)==FALSE)
						{	//find first control of parend with supporote of input
							while(seek_control->ctrl_fd!=(DWORD*)NULL)
							{
								exchange_control=(struct HEADER *)seek_control->ctrl_fd;
								seek_control=exchange_control;
								if (seek_control->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)
								{
									active_control_for_keys=seek_control;
									main_parent->global_active_control_for_keys=
											(DWORD*)active_control_for_keys;
									Parent->active_control_for_keys=(DWORD*)active_control_for_keys;
									break;
								}
							}
							if (active_control_for_keys!=NULL)
							{
								ControlProc=(void (*)(void *Control,struct MESSAGE *message))seek_control->ctrl_proc;
								local_service_message.type=MESSAGE_SET_FOCUSE;
								seek_control->flags=seek_control->flags | FLAG_FOCUSE_INPUT_ON;
								ControlProc(seek_control,&local_service_message);
							}
						}
						else
						{
							ControlProc=(void (*)(void *Control,struct MESSAGE *message))seek_control->ctrl_proc;
							local_service_message.type=MESSAGE_SET_FOCUSE;
							seek_control->flags=seek_control->flags | FLAG_FOCUSE_INPUT_ON;
							ControlProc(seek_control,&local_service_message);

							active_control_for_keys=seek_control;
							main_parent->global_active_control_for_keys=(DWORD*)active_control_for_keys;
							Parent->active_control_for_keys=(DWORD*)active_control_for_keys;
						}
					}
					else
					{
						if (active_control_for_keys==(struct HEADER*)main_parent->global_active_control_for_keys)
						{
							seek_control=active_control_for_keys;
							while(seek_control!=(struct HEADER*)NULL)
							{
								exchange_control=(struct HEADER *)seek_control->ctrl_fd;
								seek_control=exchange_control;

								if (seek_control==(struct HEADER*)NULL)
								{	//send message disable focuse of input to current control
									ControlProc=(void (*)(void *Control,struct MESSAGE *message))
											active_control_for_keys->ctrl_proc;
									local_service_message.type=MESSAGE_CHANGE_FOCUSE;
									active_control_for_keys->flags=
										active_control_for_keys->flags & FLAG_FOCUSE_INPUT_OFF;
									ControlProc(active_control_for_keys,&local_service_message);

									//back into main parent of tree and find next not NULL control
									parent=(struct HEADER*)active_control_for_keys->parent;
									do
									{
										if (parent==(struct HEADER*)main_parent) break;

										//if next control NULL go to parent
										if (seek_control==(struct HEADER*)NULL) seek_control=parent;

										exchange_control=(struct HEADER *)seek_control->ctrl_fd;
										seek_control=exchange_control;
										if (seek_control==(struct HEADER*)NULL)
													parent=(struct HEADER*)parent->parent;

									}
									while((seek_control==(struct HEADER*)NULL) ||
										((seek_control->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)==FALSE));

									if (parent==(struct HEADER*)main_parent)
									{
									//find first control of main parent with focuse of input supporote
										seek_control=(struct HEADER *)main_parent->child_bk;
										while(seek_control!=(struct HEADER*)NULL)
										{
											if (seek_control->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)
												break;
											exchange_control=(struct HEADER *)seek_control->ctrl_fd;
						seek_control=exchange_control;
										}

										main_parent->active_control_for_keys=(DWORD*)seek_control;
										main_parent->global_active_control_for_keys=(DWORD*)seek_control;

										//send message enable focuse of input to control
										ControlProc=(void (*)(void *Control,struct MESSAGE *message))
												seek_control->ctrl_proc;
										local_service_message.type=MESSAGE_SET_FOCUSE;
										seek_control->flags=seek_control->flags | FLAG_FOCUSE_INPUT_ON;
										ControlProc(seek_control,&local_service_message);

										active_control_for_keys=(struct HEADER*)NULL;
										break;

									}
									if (seek_control!=(struct HEADER*)NULL)
									{
										parent=(struct HEADER*)seek_control->parent;
										parent->active_control_for_keys=(DWORD*)seek_control;
										main_parent->global_active_control_for_keys=(DWORD*)seek_control;

										//send message enable focuse of input to control
										ControlProc=(void (*)(void *Control,struct MESSAGE *message))
												seek_control->ctrl_proc;
										local_service_message.type=MESSAGE_SET_FOCUSE;
										seek_control->flags=seek_control->flags | FLAG_FOCUSE_INPUT_ON;
										ControlProc(seek_control,&local_service_message);

										active_control_for_keys=(struct HEADER*)NULL;
										Parent->active_control_for_keys=(DWORD*)active_control_for_keys;
										break;
									}
								}
								if (seek_control->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)
								{
									ControlProc=(void (*)(void *Control,struct MESSAGE *message))
											active_control_for_keys->ctrl_proc;
									local_service_message.type=MESSAGE_CHANGE_FOCUSE;
									active_control_for_keys->flags=
										active_control_for_keys->flags & FLAG_FOCUSE_INPUT_OFF;
									ControlProc(active_control_for_keys,&local_service_message);

									ControlProc=(void (*)(void *Control,struct MESSAGE *message))
											seek_control->ctrl_proc;
									local_service_message.type=MESSAGE_SET_FOCUSE;
									seek_control->flags=seek_control->flags | FLAG_FOCUSE_INPUT_ON;
									ControlProc(seek_control,&local_service_message);

									active_control_for_keys=seek_control;
									main_parent->global_active_control_for_keys=
									(DWORD*)active_control_for_keys;
									Parent->active_control_for_keys=(DWORD*)active_control_for_keys;
									break;
								}
							}
						}
					}
				}

				//send message of keys only to active control
				if (active_control_for_keys!=NULL)
				{
					ControlProc=(void (*)(void *Control,struct MESSAGE *message))active_control_for_keys->ctrl_proc;
					ControlProc(active_control_for_keys,message);
				}
				break;
			}
			case MESSAGE_SPECIALIZED:
			{
				//send specialized message to all child controls
				seek_control=(struct HEADER *)Parent->child_bk;
				while(seek_control!=(struct HEADER*)NULL)
				{
					ControlProc=(void (*)(void *Control,struct MESSAGE *message))seek_control->ctrl_proc;
					ControlProc(seek_control,message);
					exchange_control=(struct HEADER *)seek_control->ctrl_fd;
					seek_control=exchange_control;
				}
				break;
			}
			case MESSAGE_MOUSE_EVENT:
			{
				if (message->arg3==MOUSE_LEFT_BUTTON_DOWN)
				{
					if (active_control_for_mouse!=NULL)
					{
						if (active_control_for_mouse->flags & FLAG_MOUSE_BLOCKED_ON) break;

						ControlProc=(void (*)(void *Control,struct MESSAGE *message))active_control_for_mouse->ctrl_proc;
						ControlProc(active_control_for_mouse,message);
						break;
					}
				}
				else
				{
					if (active_control_for_mouse!=NULL) {active_control_for_mouse=NULL;}
				}

				cross=FALSE;
				seek_control=(struct HEADER*)Parent->child_bk;
				while(seek_control!=(struct HEADER*)NULL)
				{
					if (CheckCrossBox(seek_control,message->arg1,message->arg2)==TRUE &&
						(seek_control->flags & FLAG_MOUSE_BLOCKED_ON)==FALSE)
					{
						if (message->arg3==MOUSE_LEFT_BUTTON_DOWN)
						{
							cross=TRUE;
							active_control_for_mouse=seek_control;

							if (active_control_for_mouse->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)
							{
								//set focuse of input for new active control
								active_control_for_mouse->flags=active_control_for_mouse->flags | FLAG_FOCUSE_INPUT_ON;

								if (active_control_for_keys!=NULL)
								{
									if (active_control_for_keys!=seek_control)
									{
										//check seek control for supporote focuse of input
										if (seek_control->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)
										{
											//check parent for supporote focuse of input
											parent=(struct HEADER*)seek_control->parent;
											if (parent->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)
											{

												ControlProc=(void (*)(void *Control,struct MESSAGE *message))
															active_control_for_keys->ctrl_proc;

												local_service_message.type=MESSAGE_CHANGE_FOCUSE;
												local_service_message.arg1=FALSE;
												active_control_for_keys->flags=
												active_control_for_keys->flags &
														FLAG_FOCUSE_INPUT_OFF;
												ControlProc(active_control_for_keys,&local_service_message);

												ControlProc=(void (*)(void *Control,struct MESSAGE *message))
															seek_control->ctrl_proc;
												local_service_message.type=MESSAGE_SET_FOCUSE;
												local_service_message.arg1=FALSE;
												seek_control->flags=seek_control->flags | FLAG_FOCUSE_INPUT_ON;
												ControlProc(seek_control,&local_service_message);

												active_control_for_keys=seek_control;
												main_parent->global_active_control_for_keys=
														(DWORD*)active_control_for_keys;
												Parent->active_control_for_keys=
														(DWORD*)active_control_for_keys;
											}
										}
									}
								}
								else
								{
									//check seek control for supporote focuse of input
									if (seek_control->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)
									{
										//check parent for supporote focuse of input
										parent=(struct HEADER*)seek_control->parent;
										if (parent->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)
										{
											active_control_for_keys=(struct HEADER*)main_parent->global_active_control_for_keys;
											if (active_control_for_keys!=NULL)
											{
												ControlProc=(void (*)(void *Control,struct MESSAGE *message))
														active_control_for_keys->ctrl_proc;

												local_service_message.type=MESSAGE_CHANGE_FOCUSE;
												active_control_for_keys->flags=
													active_control_for_keys->flags & FLAG_FOCUSE_INPUT_OFF;
												ControlProc(active_control_for_keys,&local_service_message);
											}

											ControlProc=(void (*)(void *Control,struct MESSAGE *message))
													seek_control->ctrl_proc;
											local_service_message.type=MESSAGE_SET_FOCUSE;
											seek_control->flags=seek_control->flags | FLAG_FOCUSE_INPUT_ON;
											ControlProc(seek_control,&local_service_message);

											main_parent->global_active_control_for_keys=(DWORD*)seek_control;
											Parent->active_control_for_keys=(DWORD*)seek_control;
										}
									}
								}
							}
						}
					}

					if ((seek_control->flags & FLAG_MOUSE_BLOCKED_ON)==FALSE)
					{
						ControlProc=(void (*)(void *Control,struct MESSAGE *message))seek_control->ctrl_proc;
						ControlProc(seek_control,message);
					}

					exchange_control=(struct HEADER *)seek_control->ctrl_fd;
					seek_control=exchange_control;
				}

				if ((cross==FALSE) && (message->arg3==MOUSE_LEFT_BUTTON_DOWN))
				{
					if (active_control_for_keys!=NULL)
					{//disable focuse of input for active control of keys
						ControlProc=(void (*)(void *Control,struct MESSAGE *message))active_control_for_keys->ctrl_proc;

						local_service_message.type=MESSAGE_CHANGE_FOCUSE;
						active_control_for_keys->flags=active_control_for_keys->flags & FLAG_FOCUSE_INPUT_OFF;
						ControlProc(active_control_for_keys,&local_service_message);

						main_parent->global_active_control_for_keys=(DWORD*)NULL;
						Parent->active_control_for_keys=(DWORD*)NULL;
					}
				}

				break;
			}
			case MESSAGE_CHANGE_POSITION_EVENT:
			{
				seek_control=(struct HEADER *)Parent->child_bk;
				while(seek_control!=(struct HEADER*)NULL)
				{
					ControlProc=(void (*)(void *Control,struct MESSAGE *message))seek_control->ctrl_proc;
					ControlProc(seek_control,message);

					exchange_control=(struct HEADER *)seek_control->ctrl_fd;
					seek_control=exchange_control;
				}
				break;
			}
			case MESSAGE_CALL_TIMER_EVENT:
			{
				seek_control=(struct HEADER *)Parent->child_bk;
				while(seek_control!=(struct HEADER*)NULL)
				{
					ControlProc=(void (*)(void *Control,struct MESSAGE *message))seek_control->ctrl_proc;
					ControlProc(seek_control,message);

					exchange_control=(struct HEADER *)seek_control->ctrl_fd;
					seek_control=exchange_control;
				}
				break;
			}
			case MESSAGE_SET_FOCUSE:
			{
				seek_control=(struct HEADER *)Parent->child_bk;
				while(seek_control!=(struct HEADER*)NULL)
				{
					if (seek_control->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)
					{
						ControlProc=(void (*)(void *Control,struct MESSAGE *message))seek_control->ctrl_proc;
						ControlProc(seek_control,message);
						break;
					}

					exchange_control=(struct HEADER *)seek_control->ctrl_fd;
					seek_control=exchange_control;
				}
				break;
			}
			case MESSAGE_CHANGE_FOCUSE:
			{
				seek_control=(struct HEADER *)Parent->child_bk;
				while(seek_control!=(struct HEADER*)NULL)
				{
					if (seek_control->flags & FLAG_FOCUSE_INPUT_SUPPOROTE)
					{
						ControlProc=(void (*)(void *Control,struct MESSAGE *message))seek_control->ctrl_proc;
						ControlProc(seek_control,message);
						break;
					}

					exchange_control=(struct HEADER *)seek_control->ctrl_fd;
					seek_control=exchange_control;
				}
				break;
			}
			case MESSAGE_DESTROY_CONTROL:
			{
				seek_control=(struct HEADER *)Parent->child_bk;
				while(seek_control!=(struct HEADER*)NULL)
				{	//befor delet control get next control of parent
					exchange_control=(struct HEADER *)seek_control->ctrl_fd;

					ControlProc=(void (*)(void *Control,struct MESSAGE *message))seek_control->ctrl_proc;
					//send message to control for destroyng child controls
					ControlProc(seek_control,message);
					DestroyControl(seek_control);

					seek_control=exchange_control;
				}
				break;
			}
			case MESSAGE_SET_MAIN_PARENT:
			{
				seek_control=(struct HEADER *)Parent->child_bk;
				while(seek_control!=(struct HEADER*)NULL)
				{
					ControlProc=(void (*)(void *Control,struct MESSAGE *message))seek_control->ctrl_proc;
					//send message to control for destroyng child controls
					ControlProc(seek_control,message);

					seek_control->main_parent=(DWORD*)message->arg1;

					exchange_control=(struct HEADER *)seek_control->ctrl_fd;
					seek_control=exchange_control;
				}
				break;
			}
			default:break;
		}
	}
	Parent->active_control_for_mouse=(DWORD*)active_control_for_mouse;
}

//---------------------------------------------------------------------------------
//				pack control_child in control_parent for libGUI
//---------------------------------------------------------------------------------
//add new control for Parent
void PackControls(void *parent,void *Control)
{
	struct HEADER	*Parent;
	struct HEADER	*last_control;
	struct HEADER	*control;
	struct	MESSAGE message;

	Parent=(struct HEADER *)parent;
	control=(struct HEADER *)Control;
	//set control's dependeces

	if (Parent->child_bk==NULL)
	{
		//create first child control for parent
		Parent->child_bk=(DWORD*)control;
		Parent->child_fd=(DWORD*)control;

		//init control
		control->parent=(DWORD*)Parent;
		control->ctrl_bk=(DWORD*)Parent;//last control is parent
		control->ctrl_fd=(DWORD*)NULL;//haven't next control
	}
	else
	{
		//set new control as endest child control of parent
		last_control=(struct HEADER*)Parent->child_fd;
		Parent->child_fd=(DWORD*)control;

		last_control->ctrl_fd=(DWORD*)control;
		control->ctrl_bk=(DWORD*)last_control;
		control->ctrl_fd=(DWORD*)NULL;
		control->parent=(DWORD*)Parent;
	}

	//finite control's coordinates and size
	control->ctrl_x=control->ctrl_x+Parent->ctrl_x;
	control->ctrl_y=control->ctrl_y+Parent->ctrl_y;

	if (control->ctrl_sizex<=1) {control->ctrl_sizex=1;}
	if (control->ctrl_sizey<=1) {control->ctrl_sizey=1;}

	message.type=(DWORD)MESSAGE_CHANGE_POSITION_EVENT;
	message.arg1=Parent->ctrl_x;
	message.arg2=Parent->ctrl_y;
	//send message change position to child controls
	SendMessage(control,&message);

	if (Parent->main_parent!=(DWORD*)NULL)
	{
		//tell all child controls of main parent who is main parent
		message.type=MESSAGE_SET_MAIN_PARENT;
		message.arg1=(DWORD)Parent->main_parent;
		SendMessage((struct HEADER *)Parent,&message);
	}
}

//---------------------------------------------------------------------------------
//				show/hide controls
//---------------------------------------------------------------------------------
void	ShowControl(void *Control)
{
	struct	HEADER	*control;
	control=(struct HEADER	*)Control;
	control->flags=control->flags | FLAG_SHOW_CONTROL;
}

void	HideControl(void *Control)
{
	struct	HEADER	*control;
	control=(struct HEADER	*)Control;
	control->flags=control->flags & FLAG_HIDE_CONTROL;
}

//---------------------------------------------------------------------------------
//				redraw control
//---------------------------------------------------------------------------------
void	RedrawControl(void *Control)
{
	struct	HEADER	*control;
	struct MESSAGE message;

	control=(struct HEADER*)Control;
	ControlProc=(void (*)(void *Control,struct MESSAGE *message))control->ctrl_proc;
	message.type=MESSAGE_FULL_REDRAW_ALL;
	ControlProc(control,&message);
}

//---------------------------------------------------------------------------------
//				special redraw of control
//---------------------------------------------------------------------------------
void	SpecialRedrawControl(void *Control)
{
	struct	HEADER		*control;
	struct MESSAGE	message;
	struct	HEADERPARENT	*main_parent;

	control=(struct HEADER*)Control;
	main_parent=(struct	HEADERPARENT*)control->main_parent;
	control->flags=control->flags | FLAG_GET_SPECIALIZED_MESSAGE_ON;

	ControlProc=(void (*)(void *Control,struct MESSAGE *message))control->ctrl_proc;

	message.type=MESSAGE_SPECIALIZED;
	ControlProc((struct HEADER*)main_parent,&message);
}

//---------------------------------------------------------------------------------
//				set new size for control
//---------------------------------------------------------------------------------
void	SetControlSizeRequest(void *Control,int new_size_x,int new_size_y)
{
	struct	HEADER	*control;
	struct	HEADER	*Parent;

	control=(struct HEADER*)Control;
	Parent=(struct	HEADER*)control->parent;

	control->ctrl_sizex=(DWORD)new_size_x;
	control->ctrl_sizey=(DWORD)new_size_y;

	if ((control->ctrl_x+control->ctrl_sizex)>Parent->ctrl_sizex)
	{control->ctrl_sizex=Parent->ctrl_sizex-control->ctrl_x;}

	if ((control->ctrl_y+control->ctrl_sizey)>Parent->ctrl_sizey)
	{control->ctrl_sizey=Parent->ctrl_sizey-control->ctrl_y;}

	if (control->ctrl_sizex<=1) {control->ctrl_sizex=1;}
	if (control->ctrl_sizey<=1) {control->ctrl_sizey=1;}

}

//---------------------------------------------------------------------------------
//				get size x of control
//---------------------------------------------------------------------------------
int	GetControlSizeX(void *Control)
{
	struct	HEADER	*control;

	control=(struct HEADER *)Control;

	return ((int)control->ctrl_sizex);
}

//---------------------------------------------------------------------------------
//				get size y of control
//---------------------------------------------------------------------------------
int	GetControlSizeY(void *Control)
{
	struct	HEADER	*control;

	control=(struct HEADER *)Control;

	return ((int)control->ctrl_sizey);
}

//---------------------------------------------------------------------------------
//				set new size for control
//---------------------------------------------------------------------------------
void	SetControlNewPosition(void *Control,int new_x,int new_y)
{
	struct	HEADER		*control;
	struct	HEADER		*Parent;
	struct MESSAGE	 message;
	parent_t		*main_parent;
	int		old_x;
	int		old_y;

	control=(struct HEADER*)Control;
	/*
	main_parent=(parent_t*)control->main_parent;
	if (control->parent==(DWORD*)main_parent)
	{//check position of child control of main parent
		if (new_x+control->ctrl_sizex-1>screen.size_x) return;
		if (new_y+control->ctrl_sizey-1>screen.size_y) return;
	}*/

	message.type=(DWORD)MESSAGE_CHANGE_POSITION_EVENT;
	message.arg1=(DWORD)(new_x-control->ctrl_x);
	message.arg2=(DWORD)(new_y-control->ctrl_y);

	ControlProc=(void (*)(void *Control,struct MESSAGE *message))control->ctrl_proc;
	ControlProc(control,&message);
}

//---------------------------------------------------------------------------------
//				get x position of control
//---------------------------------------------------------------------------------
int	GetControlPositionX(void *Control)
{
	struct	HEADER	*control;

	control=(struct HEADER *)Control;

	return ((int)control->ctrl_x);
}

//---------------------------------------------------------------------------------
//				get y position of control
//---------------------------------------------------------------------------------
int	GetControlPositionY(void *Control)
{
	struct	HEADER	*control;

	control=(struct HEADER *)Control;

	return ((int)control->ctrl_y);
}

//---------------------------------------------------------------------------------
//			set focuse of input for control
//---------------------------------------------------------------------------------
void	*SetFocuse(void *Control)
{
	struct	HEADER		*control;
	struct	HEADERPARENT	*main_parent;
	struct	HEADER		*old_control;
	struct	MESSAGE *message;

	//new active control with focuse
	control=(struct HEADER*)Control;
	main_parent=(struct HEADERPARENT*)control->main_parent;

	if (main_parent==(struct HEADERPARENT*)NULL) return;

	//old control with focuse
	old_control=(struct HEADER*)main_parent->global_active_control_for_keys;

	if (old_control!=(struct HEADER*)NULL)
	{
		message->type=MESSAGE_CHANGE_FOCUSE;
		message->arg1=FALSE;
		message->arg2=FALSE;
		message->arg3=FALSE;
		ControlProc=(void (*)(void *Control,struct MESSAGE *message))old_control->ctrl_proc;
		ControlProc(old_control,message);
	}

	message->type=MESSAGE_SET_FOCUSE;
	message->arg1=FALSE;
	message->arg2=FALSE;
	message->arg3=FALSE;;
	ControlProc=(void (*)(void *Control,struct MESSAGE *message))control->ctrl_proc;
	ControlProc(control,message);

	return(old_control);
}

//---------------------------------------------------------------------------------
//			set callback function for control
//---------------------------------------------------------------------------------
void *SetCallbackFunction(void *Control,int event_name,void *callback_func,void *callback_func_data)
{
	struct	HEADER		*control;
	struct	CALLBACK	*seek_callback;
	struct	CALLBACK	*exchange_callback;
	struct	CALLBACK	*new_callback;

	control=(struct HEADER *)Control;
	seek_callback=(struct CALLBACK*)control->callback;

	if (seek_callback==(struct CALLBACK *)NULL)
	{	//callback function creating at first
		new_callback=malloc(sizeof(struct CALLBACK));
		control->callback=(DWORD*)new_callback;
		new_callback->clb_bk=(DWORD*)control;
		new_callback->clb_fd=(DWORD*)NULL;
		new_callback->clb_control=(DWORD*)control;
		new_callback->connect_event=(DWORD)event_name;
		new_callback->func=(DWORD*)callback_func;
		new_callback->func_data=(DWORD*)callback_func_data;
		new_callback->flags=0x0;
	}
	else
	{
		while(seek_callback!=(struct CALLBACK*)NULL)
		{
			if (seek_callback->clb_fd==(DWORD*)NULL)
			{//create new callback control for callback function
				new_callback=malloc(sizeof(struct CALLBACK));
				seek_callback->clb_fd=(DWORD*)new_callback;
				new_callback->clb_bk=(DWORD*)seek_callback;
				new_callback->clb_fd=(DWORD*)NULL;
				new_callback->clb_control=(DWORD*)control;
				new_callback->connect_event=(DWORD)event_name;
				new_callback->func=(DWORD*)callback_func;
				new_callback->func_data=(DWORD*)callback_func_data;
				new_callback->flags=0x0;
				break;
			}
			exchange_callback=(struct CALLBACK*)seek_callback->clb_fd;
			seek_callback=exchange_callback;
		}
	}
#ifdef DEBUG
        printf("\ncreated callback %d for function %d data %d",
			(DWORD)new_callback,
			(DWORD)callback_func,
			(DWORD)callback_func_data);
#endif

	return(new_callback);
}

//---------------------------------------------------------------------------------
//			block current  callback function for control
//---------------------------------------------------------------------------------
void	BlockCallbackFunction(void *Control,void *callback_ID)
{
	struct	HEADER		*control;
	struct	CALLBACK	*seek_callback;
	struct	CALLBACK	*exchange_callback;
	struct	CALLBACK	*unknown_callback;

	control=(struct HEADER *)Control;

	seek_callback=(struct CALLBACK *)control->callback;
	unknown_callback=(struct CALLBACK*)callback_ID;

	while(seek_callback!=(struct CALLBACK*)NULL)
	{
		if (seek_callback==unknown_callback)
		{
			unknown_callback->flags=unknown_callback->flags | FLAG_BLOCK_CALLBACK_ON;
			break;
		}
		exchange_callback=(struct CALLBACK*)seek_callback->clb_fd;
		seek_callback=exchange_callback;
	}

}

//---------------------------------------------------------------------------------
//			unblock current  callback function for control
//---------------------------------------------------------------------------------
void	UnblockCallbackFunction(void *Control,void *callback_ID)
{
	struct	HEADER		*control;
	struct	CALLBACK	*seek_callback;
	struct	CALLBACK	*exchange_callback;
	struct	CALLBACK	*unknown_callback;

	control=(struct HEADER *)Control;

	seek_callback=(struct CALLBACK *)control->callback;
	unknown_callback=(struct CALLBACK*)callback_ID;

	while(seek_callback!=(struct CALLBACK*)NULL)
	{
		if (seek_callback==unknown_callback)
		{
			unknown_callback->flags=unknown_callback->flags & FLAG_BLOCK_CALLBACK_OFF;
			break;
		}
		exchange_callback=(struct CALLBACK*)seek_callback->clb_fd;
		seek_callback=exchange_callback;
	}
}

//---------------------------------------------------------------------------------
//			check callback event for control
//---------------------------------------------------------------------------------
void	*ControlCheckCallbackEvent(void *Control,DWORD event)
{
	struct	HEADER		*control;
	struct	CALLBACK	*seek_callback;
	struct	CALLBACK	*exchange_callback;

	control=(struct HEADER *)Control;
	seek_callback=(struct CALLBACK *)control->callback;

	while(seek_callback!=(struct CALLBACK*)NULL)
	{
		if ((seek_callback->flags & FLAG_BLOCK_CALLBACK_ON)==FALSE)
		{
			if (seek_callback->connect_event==event)	return (seek_callback);
		}
		exchange_callback=(struct CALLBACK*)seek_callback->clb_fd;
		seek_callback=exchange_callback;
	}
	return(NULL);
}

//---------------------------------------------------------------------------------
//			set IDL function for libGUI
//---------------------------------------------------------------------------------
void	SetIDL_Function(parent_t *parent,void *function,void *function_data)
{
	parent->IDL_func=(DWORD*)function;
	parent->IDL_func_data=(DWORD*)function_data;
}

//---------------------------------------------------------------------------------
//			destroy IDL function for libGUI
//---------------------------------------------------------------------------------
void	DestroyIDL_Function(parent_t *parent)
{
	parent->IDL_func=(DWORD*)NULL;
}

//---------------------------------------------------------------------------------
//					initialize libGUI
//---------------------------------------------------------------------------------
char	InitLibGUI()
{
	font_t	*font;
//---------------------------------------------------------------------------------
//---------------------------platform depended part of code------------------------
//---------------------------------------------------------------------------------
	//set new events mask
	gui_ksys_set_events_mask(119);
	//set scan codes input mode for keyboard
	gui_ksys_set_keyboard_input_mode(1);
//------------------------------------------------------------------------------------
	FontsMeneger.fnt_fd=(DWORD*)NULL;
	FontsMeneger.fnt_bk=(DWORD*)NULL;
	FontsMeneger.number_fonts=0;
	//load default fonts
	font=LoadFont("CHAR.MT");

	if (font==NULL) return (TRUE);
		else	FontsMeneger.default_font=(DWORD*)font;

	font->flags=font->flags | FONT_FLAG_DEFAULT_FONT_ON;

	return(FALSE);
}

//---------------------------------------------------------------------------------
//			quit from libGUI loop and destroy all GUI objects
//---------------------------------------------------------------------------------
void	QuitLibGUI(parent_t *window)
{
	font_t			*font,*seek_font,*exchange_font;
	gui_timer_t		*seek_timer,*exchange_timer;
	gui_message_t		message;

#ifdef DEBUG
	printf("\nbegin free libGUI...");
#endif

	//destroy controls of parent window
	message.type=MESSAGE_DESTROY_CONTROL;
	SendMessage((header_t*)window,&message);
#ifdef DEBUG
	printf("\ncontrols destroyed");
#endif
	//destroy timers for callback functions
	seek_timer=(gui_timer_t*)window->timer_bk;
	while(seek_timer!=(gui_timer_t*)NULL)
	{
		exchange_timer=(gui_timer_t*)seek_timer->tmr_fd;

		DestroyTimerCallbackForFunction(seek_timer);

		seek_timer=exchange_timer;
	}
#ifdef DEBUG
	printf("\ntimers destroyed");
#endif
	//free arrays of parent window
	free(window->message);
	free(window->control_for_callback_function);
	free(window->callback_for_control_callback);
#ifdef DEBUG
	printf("\narrays destroyed");
#endif
	//destroy parent window
	free(window);
#ifdef DEBUG
	printf("\nparent window destroyed");
#endif
	//destroy fonts cash

	seek_font=(font_t*)FontsMeneger.fnt_bk;
	while(seek_font!=(font_t*)NULL)
	{
		exchange_font=(font_t*)seek_font->fnt_fd;

		free(seek_font->font);
		DestroyFont(seek_font);

		seek_font=exchange_font;
	}
#ifdef DEBUG
	printf("\nfonts destroyed");
	printf("\nexit program...");
#endif
	exit(0);
}

DWORD	LibGUIversion(void)
{	//25.10.09
	return(91025);
}
