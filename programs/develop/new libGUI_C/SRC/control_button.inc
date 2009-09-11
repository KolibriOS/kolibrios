/*
		control Button
*/

void DrawFocuseForButton(struct ControlButton *Button)
{
	int	x;
	int	y;
	int	sizex;
	int	sizey;
	struct FINITION *fin;

	x=Button->ctrl_x;
	y=Button->ctrl_y;
	sizex=Button->ctrl_sizex;
	sizey=Button->ctrl_sizey;
	fin=(struct FINITION*)Button->finition;

	Draw(fin,TOOL_RECTANGLE,x,y,sizex,sizey,0xbfff);
}

void DrawPressedButton(struct ControlButton *Button)
{
	int	x;
	int	y;
	int	sizex;
	int	sizey;
	struct FINITION *fin;
	gui_message_t	message;
	
	x=Button->ctrl_x;
	y=Button->ctrl_y;
	sizex=Button->ctrl_sizex;
	sizey=Button->ctrl_sizey;
	fin=(struct FINITION*)Button->finition;

	Draw(fin,TOOL_GRADIENT_DOWN_FILLED_RECTANGLE,x,y,sizex,sizey/2,COLOR_FON,COLOR_MIDDLE_LIGHT);
	Draw(fin,TOOL_GRADIENT_DOWN_FILLED_RECTANGLE,x,y+sizey/2,sizex,sizey/2,COLOR_MIDDLE_LIGHT,COLOR_FON);

	
	Draw(fin,TOOL_HORIZONTAL_LINE,x,x+sizex-1,y,COLOR_ABSOLUTE_DARK);
	Draw(fin,TOOL_VERTICAL_LINE,x,y,y+sizey-1,COLOR_LIGHT);
	Draw(fin,TOOL_HORIZONTAL_LINE,x,x+sizex-1,y+sizey-1,COLOR_LIGHT);
	Draw(fin,TOOL_VERTICAL_LINE,x,y,y+sizey-1,COLOR_ABSOLUTE_DARK);

	if (fin->flags & FINITION_ON)
	{
			message.type=MESSAGE_FULL_REDRAW_ALL_WITH_FINITION;
			message.arg1=fin->x;
			message.arg2=fin->y;
			message.arg3=fin->sizex;
			message.arg4=fin->sizey;
	}
	else
	{
			message.type=MESSAGE_FULL_REDRAW_ALL;
	}

	SendMessage((struct HEADER*)Button,&message);
	
	if (Button->flags & FLAG_FOCUSE_INPUT_ON) DrawFocuseForButton(Button);
}

void DrawInsertButton(struct ControlButton *Button)
{
	int	x;
	int	y;
	int	sizex;
	int	sizey;
	struct FINITION *fin;

	x=Button->ctrl_x;
	y=Button->ctrl_y;
	sizex=Button->ctrl_sizex;
	sizey=Button->ctrl_sizey;
	fin=(struct FINITION*)Button->finition;
	Draw(fin,TOOL_RECTANGLE,x,y,sizex,sizey,COLOR_INSERT);
}

void DrawButton(struct ControlButton *Button)
{
	int	x;
	int	y;
	int	sizex;
	int	sizey;
	struct FINITION *fin;
	gui_message_t	message;
	
	x=Button->ctrl_x;
	y=Button->ctrl_y;
	sizex=Button->ctrl_sizex;
	sizey=Button->ctrl_sizey;
	fin=(struct FINITION*)Button->finition;

	Draw(fin,TOOL_GRADIENT_UP_FILLED_RECTANGLE,x,y,sizex,sizey,COLOR_FON,COLOR_MIDDLE_LIGHT);
	//Draw(fin,TOOL_GRADIENT_DOWN_FILLED_RECTANGLE,x,y+sizey/2,sizex,sizey/2,COLOR_FON,COLOR_MIDDLE_LIGHT);

	
	Draw(fin,TOOL_RECTANGLE,x,y,sizex,sizey,COLOR_ABSOLUTE_DARK);

	Draw(fin,TOOL_HORIZONTAL_LINE,x+1,x+sizex-2,y+1,COLOR_LIGHT);
	Draw(fin,TOOL_VERTICAL_LINE,x+sizex-2,y+1,y+sizey-2,COLOR_MIDDLE_LIGHT);
	Draw(fin,TOOL_HORIZONTAL_LINE,x+1,x+sizex-2,y+sizey-2,COLOR_MIDDLE_LIGHT);
	Draw(fin,TOOL_VERTICAL_LINE,x+1,y+1,y+sizey-2,COLOR_LIGHT);

	if (Button->child_bk!=NULL)
	{		
		if (fin->flags & FINITION_ON)
		{
			message.type=MESSAGE_FULL_REDRAW_ALL_WITH_FINITION;
			message.arg1=fin->x;
			message.arg2=fin->y;
			message.arg3=fin->sizex;
			message.arg4=fin->sizey;
		}
		else
		{
			message.type=MESSAGE_FULL_REDRAW_ALL;
		}
		SendMessage((struct HEADER*)Button,&message);
	}
	if (Button->flags & FLAG_FOCUSE_INPUT_ON) DrawFocuseForButton(Button);
}

//---------------------------------------------------------------------------------
//			 control Button
//---------------------------------------------------------------------------------
void ButtonProc(struct ControlButton *button,struct MESSAGE *message)
{
	int			x,y,btn_state;
	char			v;
	struct	TIMER		*timer;
	struct	FINITION	*fin;
	parent_t		*main_parent;

	switch(message->type)
	{
		case MESSAGE_FULL_REDRAW_ALL:
		{
			//draw button
			if (button->flags & FLAG_SHOW_CONTROL)	DrawButton(button);
			break;
		}
		case MESSAGE_FULL_REDRAW_ALL_WITH_FINITION:
		{
			fin=(struct FINITION*)button->finition;
			fin->flags=fin->flags | FINITION_ON;
			fin->x=message->arg1;
			fin->y=message->arg2;
			fin->sizex=message->arg3;
			fin->sizey=message->arg4;
			DrawButton(button);
			break;
		}
		case MESSAGE_KEYS_EVENT:
		{
			main_parent=(parent_t*)button->main_parent;
			//not relazed yet
			if (button->flags & FLAG_FOCUSE_INPUT_ON)
			{
				if (message->arg1==KEY_DOWN)
				{
					if (message->arg2==SC_SPACE)
					{
						button->btn_flags=button->btn_flags | FLAG_PRESSED_BUTTON_ON;

						if (ControlCheckCallbackEvent(button,(DWORD)BUTTON_PRESSED_EVENT)!=NULL)
						{
							button->flags=button->flags | FLAG_CONNECT_EVENT_ON;
							main_parent->control_for_callback_function[main_parent->number_callbacks]=
							(DWORD*)button;
							main_parent->callback_for_control_callback[main_parent->number_callbacks]=
							(DWORD*)ControlCheckCallbackEvent(button,(DWORD)BUTTON_PRESSED_EVENT);
							main_parent->number_callbacks++;
						}
						if (button->flags & FLAG_SHOW_CONTROL) DrawPressedButton(button);
					}
				}
				else
				{
					if (message->arg2==SC_SPACE) 
					{
						button->btn_flags=button->btn_flags | FLAG_RELEASED_BUTTON_ON;
						button->btn_flags=button->btn_flags & FLAG_PRESSED_BUTTON_OFF;

						if (ControlCheckCallbackEvent(button,(DWORD)BUTTON_RELEASED_EVENT)!=NULL)
						{
							button->flags=button->flags | FLAG_CONNECT_EVENT_ON;
							main_parent->control_for_callback_function[main_parent->number_callbacks]=
							(DWORD*)button;
							main_parent->callback_for_control_callback[main_parent->number_callbacks]=
							(DWORD*)ControlCheckCallbackEvent(button,(DWORD)BUTTON_RELEASED_EVENT);
							main_parent->number_callbacks++;
						}
						if (button->flags & FLAG_SHOW_CONTROL) DrawButton(button);
					}
				}
			}
			break;
		}
		case MESSAGE_SPECIALIZED:
		{
			if (button->flags & FLAG_GET_SPECIALIZED_MESSAGE_ON)
			{
				if (button->flags & FLAG_SHOW_CONTROL)	DrawButton(button);
				button->flags=button->flags & FLAG_GET_SPECIALIZED_MESSAGE_OFF;
			}			
			break;
		}
		case MESSAGE_MOUSE_EVENT:
		{	//check press of mouse buttons
			x=message->arg1;
			y=message->arg2;
			main_parent=(parent_t*)button->main_parent;

			if (message->arg3==MOUSE_LEFT_BUTTON_UP)
			{
				//insert of button
				if (CheckCrossBox((struct HEADER*)button,x,y)==TRUE)
				{
					v=button->btn_flags & FLAG_INSERT_BUTTON_ON;
					if ((ControlCheckCallbackEvent(button,(DWORD)BUTTON_ENTER_EVENT)!=NULL) && (v==FALSE))
					{
						button->flags=button->flags | FLAG_CONNECT_EVENT_ON;
						main_parent->control_for_callback_function[main_parent->number_callbacks]=
						(DWORD*)button;
						main_parent->callback_for_control_callback[main_parent->number_callbacks]=
						(DWORD*)ControlCheckCallbackEvent(button,(DWORD)BUTTON_ENTER_EVENT);
						main_parent->number_callbacks++;
					}
					button->btn_flags=button->btn_flags | FLAG_INSERT_BUTTON_ON;

					if (button->flags & FLAG_SHOW_CONTROL) DrawInsertButton(button);
				}
				else 
				{
					v=button->btn_flags & FLAG_INSERT_BUTTON_ON;
					if (v==TRUE)
					{
						if (ControlCheckCallbackEvent(button,(DWORD)BUTTON_LEAVE_EVENT)!=NULL)
						{
							button->flags=button->flags | FLAG_CONNECT_EVENT_ON;
							main_parent->control_for_callback_function[main_parent->number_callbacks]=
							(DWORD*)button;
							main_parent->callback_for_control_callback[main_parent->number_callbacks]=
							(DWORD*)ControlCheckCallbackEvent(button,(DWORD)BUTTON_LEAVE_EVENT);
							main_parent->number_callbacks++;
						}
						button->btn_flags=button->btn_flags & FLAG_INSERT_BUTTON_OFF;
						if (button->flags & FLAG_SHOW_CONTROL) DrawButton(button);
					}
				}

				if (button->btn_flags & FLAG_PRESSED_BUTTON_ON)
				{
					button->btn_flags=button->btn_flags & FLAG_PRESSED_BUTTON_OFF;
					button->btn_flags=button->btn_flags | FLAG_RELEASED_BUTTON_ON;

					if (ControlCheckCallbackEvent(button,(DWORD)BUTTON_RELEASED_EVENT)!=NULL)
					{
						button->flags=button->flags | FLAG_CONNECT_EVENT_ON;
						main_parent->control_for_callback_function[main_parent->number_callbacks]=
						(DWORD*)button;
						main_parent->callback_for_control_callback[main_parent->number_callbacks]=
						(DWORD*)ControlCheckCallbackEvent(button,(DWORD)BUTTON_RELEASED_EVENT);
						main_parent->number_callbacks++;
					}
					if (button->flags & FLAG_SHOW_CONTROL) DrawButton(button);
				}
				
				
			}
			else
			{
				if (CheckCrossBox((struct HEADER*)button,x,y)==TRUE)
				{
					if (message->arg3==MOUSE_LEFT_BUTTON_DOWN)
					{
						button->btn_flags=button->btn_flags | FLAG_PRESSED_BUTTON_ON;
					}
				}

				if ((message->arg3==MOUSE_LEFT_BUTTON_DOWN) && (button->btn_flags & FLAG_PRESSED_BUTTON_ON))
				{
					if (ControlCheckCallbackEvent(button,(DWORD)BUTTON_PRESSED_EVENT)!=NULL)
					{
						button->flags=button->flags | FLAG_CONNECT_EVENT_ON;
						main_parent->control_for_callback_function[main_parent->number_callbacks]=
						(DWORD*)button;
						main_parent->callback_for_control_callback[main_parent->number_callbacks]=
						(DWORD*)ControlCheckCallbackEvent(button,(DWORD)BUTTON_PRESSED_EVENT);
						main_parent->number_callbacks++;
					}
					if (button->flags & FLAG_SHOW_CONTROL) DrawPressedButton(button);
				}
			}
			break;
		}
		case MESSAGE_CHANGE_POSITION_EVENT:
		{
			button->ctrl_x=button->ctrl_x+message->arg1;
			button->ctrl_y=button->ctrl_y+message->arg2;
			break;
		}
		case MESSAGE_CALL_TIMER_EVENT:
		{
			if (button->timer!=(DWORD*)NULL)
			{
				timer=(struct TIMER*)button->timer;
				if (timer->flags & FLAG_TIMER_ON)	Timer(timer);
			}
			break;
		}
		case MESSAGE_SET_FOCUSE:
		{
			button->flags=button->flags | FLAG_FOCUSE_INPUT_ON;
			if (button->flags & FLAG_SHOW_CONTROL)	DrawButton(button);
			break;
		}
		case MESSAGE_CHANGE_FOCUSE:
		{
			button->flags=button->flags & FLAG_FOCUSE_INPUT_OFF;
			if (button->flags & FLAG_SHOW_CONTROL)	DrawButton(button);
			break;
		}
		case MESSAGE_DESTROY_CONTROL:
		{
			if (button->timer!=(DWORD*)NULL)	free(button->timer);
			free(button->finition);
			break;
		}
		case MESSAGE_SET_MAIN_PARENT:
		{
			SendMessage((struct HEADER*)button,message);
			button->main_parent=(DWORD*)message->arg1;
			break;
		}
		default: break;
	}
	//send message to child controls(if there is)
	SendMessage((struct HEADER*)button,message);
}

//---------------------------------------------------------------------------------
//				create control Button
//---------------------------------------------------------------------------------
void* CreateButton(struct ButtonData *info_for_control)
{
	struct ControlButton *Button;
	struct FINITION	*fin;

	Button=malloc(sizeof(struct ControlButton));
	Button->finition=malloc(sizeof(struct FINITION));
	fin=(struct FINITION*)Button->finition;
	fin->flags=0;
	
	ID++;
#ifdef	DEBUG
	printf("\ncreated button with ID=%d",(int)ID);
#endif
	Button->child_bk=(DWORD*)NULL;
	Button->child_fd=(DWORD*)NULL;
	Button->active_control_for_keys=(DWORD*)NULL;
	Button->active_control_for_mouse=(DWORD*)NULL;
	Button->callback=(DWORD*)NULL;
	Button->timer=(DWORD*)NULL;

	Button->ctrl_proc=(DWORD*)&ButtonProc;
	Button->ctrl_x=info_for_control->x;
	Button->ctrl_y=info_for_control->y;
	Button->ctrl_sizex=info_for_control->width;
	Button->ctrl_sizey=info_for_control->height;
	Button->ctrl_ID=ID;
	Button->flags=0;
	Button->flags=Button->flags | FLAG_SHOW_CONTROL;
	Button->flags=Button->flags | FLAG_FOCUSE_INPUT_SUPPOROTE;

	Button->btn_flags=0;

	return(Button);
}

void* CreateButtonWithText(gui_button_data_t *info,char *txt)
{
	gui_button_t	*Button;
	gui_text_t		*text;
	gui_text_data_t	txtdata;
	int			len;
	
	Button=CreateButton(info);
	len=strlen(txt);
	
	txtdata.x=0;
	txtdata.y=0;
	txtdata.font=NULL;
	txtdata.background=FALSE;
	txtdata.color=0;
	txtdata.text=malloc(len);
	memmove(txtdata.text,txt,len);
	text=CreateText(&txtdata);

	if (text->ctrl_sizex>Button->ctrl_sizex) Button->ctrl_sizex=text->ctrl_sizex+6;
 	if (text->ctrl_sizey>Button->ctrl_sizey) Button->ctrl_sizey=text->ctrl_sizey+6;
	
	text->ctrl_x=(Button->ctrl_sizex/2)-(text->ctrl_sizex/2);
	text->ctrl_y=(Button->ctrl_sizey/2)-(text->ctrl_sizey/2);
	PackControls(Button,text);
	
	return(Button);
}

