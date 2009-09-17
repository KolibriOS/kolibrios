/*
	main meneger loop libGUI library
*/

void LibGUImain(parent_t *WindowParent)
{
	DWORD			event,key,value;
	header_t		*control;
	gui_message_t		*events_message;
	gui_timer_t		*seek_timer,*exchange_timer;
	gui_callbackevent_t	*seek_calev,*calev,*exchange_calev;
	int			i;
	char			quit;


	events_message=(struct MESSAGE*)WindowParent->message;
	
	//tell all child controls of main parent who is main parent
	events_message->type=MESSAGE_SET_MAIN_PARENT;
	events_message->arg1=(DWORD)WindowParent;
	SendMessage((struct HEADER *)WindowParent,events_message);

	//display all created controls in window
	gui_draw_window(WindowParent);
	events_message->type=MESSAGE_FULL_REDRAW_ALL;
	SendMessage((struct HEADER *)WindowParent,events_message);

	events_message->type=0;

	quit=FALSE;
	WindowParent->number_callbacks=0;

	while (quit==FALSE)
	{
		//check for timers
		if ((WindowParent->timer_bk!=(DWORD*)NULL) || 
			(WindowParent->number_timers_for_controls!=0)) {event=gui_ksys_wait_event_with_timeout(1);}
			else	{event=gui_ksys_wait_event();}

		//get and chack system events
		switch(event)
		{
			case KOLIBRIOS_SYS_EVENT_BUTTON_PRESSED:
			{
				if (ControlCheckCallbackEvent(WindowParent,DELETE_EVENT)!=NULL)
				{
						WindowParent->flags |= FLAG_CONNECT_EVENT_ON;
						WindowParent->control_for_callback_function[WindowParent->number_callbacks]=
						(DWORD*)WindowParent;
						WindowParent->callback_for_control_callback[WindowParent->number_callbacks]=
						(DWORD*)ControlCheckCallbackEvent(WindowParent,(DWORD)DELETE_EVENT);
						WindowParent->number_callbacks++;
				}

				quit=TRUE;
				break;
			}

			case KOLIBRIOS_SYS_EVENT_REDRAW:
			{
				GetNewWindowSizePos(WindowParent);
				SetWindowSizeRequest(WindowParent,WindowParent->ctrl_sizex,WindowParent->ctrl_sizey);
				gui_draw_window(WindowParent);
				
				events_message->type=MESSAGE_FULL_REDRAW_ALL;
				SendMessage((struct HEADER *)WindowParent,events_message);
				break;
			}

			case KOLIBRIOS_SYS_EVENT_KEYS:
			{
				key=gui_ksys_get_key();

				
				key=key>>8;

				if (key & 128) 
				{
					events_message->arg1=KEY_UP;
					events_message->arg2=key & 127;
				}
					else
				{
					events_message->arg1=KEY_DOWN;
					events_message->arg2=key;
				}
				
				events_message->type=MESSAGE_KEYS_EVENT;
				SendMessage((struct HEADER *)WindowParent,events_message);
				break;
			}

			case KOLIBRIOS_SYS_EVENT_MOUSE:
			{
				value=gui_ksys_get_window_mouse_coordinates();
				
				events_message->type=MESSAGE_MOUSE_EVENT;
				events_message->arg2=(value & 0xffff)-screen.y;//y
				value=value >>16;
				events_message->arg1=value-screen.x;//x

				value=gui_ksys_get_mouse_buttons_state();
				
				switch(value)
				{
					case KOLIBRIOS_SYS_MOUSE_BUTTON_LEFT_DOWN:
					{
						events_message->arg3=MOUSE_LEFT_BUTTON_DOWN;
						break;
					}
					case KOLIBRIOS_SYS_MOUSE_BUTTON_RIGHT_DOWN:
					{
						events_message->arg3=MOUSE_RIGHT_BUTTON_DOWN;
						break;
					}
					case KOLIBRIOS_SYS_MOUSE_BUTTON_MIDDLE_DOWN:
					{
						events_message->arg3=MOUSE_MIDDLE_BUTTON_DOWN;
						break;
					}
					case KOLIBRIOS_SYS_MOUSE_BUTTON_4_DOWN:
					{
						events_message->arg3=MOUSE_4_BUTTON_DOWN;
						break;
					}
					case KOLIBRIOS_SYS_MOUSE_BUTTON_5_DOWN:
					{
						events_message->arg3=MOUSE_5_BUTTON_DOWN;
						break;
					}
					default:
					{
						if (events_message->arg3==MOUSE_LEFT_BUTTON_DOWN)
						{
							events_message->arg3=MOUSE_LEFT_BUTTON_UP;
							break;
						}
						if (events_message->arg3==MOUSE_RIGHT_BUTTON_DOWN)
						{
							events_message->arg3=MOUSE_RIGHT_BUTTON_UP;
							break;
						}
						if (events_message->arg3==MOUSE_MIDDLE_BUTTON_DOWN)
						{
							events_message->arg3=MOUSE_MIDDLE_BUTTON_UP;
							break;
						}
						if (events_message->arg3==MOUSE_4_BUTTON_DOWN)
						{
							events_message->arg3=MOUSE_4_BUTTON_UP;
							break;
						}
						if (events_message->arg3==MOUSE_5_BUTTON_DOWN)
						{
							events_message->arg3=MOUSE_5_BUTTON_UP;
							break;
						}

						break;
					}
				}				
				SendMessage((struct HEADER *)WindowParent,events_message);
				break;				
			}
		}

		//call functions for events
		seek_calev=(struct CALLBACKEVENT*)WindowParent->calev_bk;
		while(seek_calev!=(struct CALLBACKEVENT*)NULL)
		{
			if (seek_calev->event_type==events_message->type)
			{
				CallbackFunctionForEvent=(void(*)(struct MESSAGE *message,void *data))seek_calev->func;
				CallbackFunctionForEvent(events_message,seek_calev->func_data);
			}
			exchange_calev=(struct CALLBACKEVENT*)seek_calev->calev_fd;
			seek_calev=exchange_calev;
		}
		

		//call timers of controls
		if (WindowParent->number_timers_for_controls!=0)
		{
			events_message->type=(char)MESSAGE_CALL_TIMER_EVENT;
			SendMessage((struct HEADER *)WindowParent,events_message);
		}

		//call callback functions
		for(i=0;i<WindowParent->number_callbacks;i++)
		{
			control=(struct HEADER*)WindowParent->control_for_callback_function[i];
			//check callback control
			if (control!=(struct HEADER*)NULL)
			{
				calev=(struct CALLBACKEVENT*)WindowParent->callback_for_control_callback[i];
				if (control->flags & FLAG_CONNECT_EVENT_ON)
				{
					CallbackFunction=(void (*)( struct HEADER *Control,void *data))calev->func;
					CallbackFunction(control,calev->func_data);

					control->flags=control->flags & FLAG_CONNECT_EVENT_OFF;
				}
			}
		}
		WindowParent->number_callbacks=0;

		//call timers of parent window
		seek_timer=(struct TIMER*)WindowParent->timer_bk;
		while(seek_timer!=(struct TIMER*)NULL)
		{
			if (seek_timer->flags & FLAG_TIMER_ON)	Timer(seek_timer);

			exchange_timer=(struct TIMER*)seek_timer->tmr_fd;
			seek_timer=exchange_timer;
		}

		//check for IDL function and call it if enabled
		if (WindowParent->IDL_func!=(DWORD*)NULL)
		{
			IDL_Function=(void(*)(void *data))WindowParent->IDL_func;
			IDL_Function(WindowParent->IDL_func_data);
		}

	}
}
