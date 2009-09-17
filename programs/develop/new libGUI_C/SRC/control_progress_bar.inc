/*
		control ProgressBar
*/

void ProgressBarDrawProgress(struct ControlProgressBar *ProgressBar)
{
	int	x;
	int	y;
	int	pos_progress;
	int	sizex;
	int	sizey;
	char	v;
	struct FINITION *fin;
	gui_message_t	message;
	gui_text_size_t		size;
	gui_text_t		*Text;
	
	if ((ProgressBar->flags & FLAG_SHOW_CONTROL)==FALSE)	return;

	x=ProgressBar->ctrl_x;
	y=ProgressBar->ctrl_y;
	sizex=ProgressBar->ctrl_sizex;
	sizey=ProgressBar->ctrl_sizey;
	fin=(struct FINITION*)ProgressBar->finition;

	if (ProgressBar->progress<0.0)	ProgressBar->progress=0.0;
	if (ProgressBar->progress>1.0)	ProgressBar->progress=1.0;

	v=ProgressBar->prb_flags & FLAG_PB_HORIZONTAL_ORIENTATION_ON;
	if (v!=FALSE)
	{
		pos_progress=(int)(ProgressBar->progress*(sizex-2));

		if (ProgressBar->prb_flags & FLAG_PB_FROM_LEFT_TO_RIGHT_ON)
		{
			Draw(fin,TOOL_GRADIENT_UP_FILLED_RECTANGLE,x+1,y+1,pos_progress,sizey-2,COLOR_FON,COLOR_MIDDLE_LIGHT);
			Draw(fin,TOOL_GRADIENT_UP_FILLED_RECTANGLE,x+1+pos_progress,y+1,sizex-2-pos_progress,
											sizey-2,COLOR_MIDDLE_LIGHT,COLOR_LIGHT);
			if (ProgressBar->prb_flags & FLAG_PB_TEXT_ON)
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
				Text=(gui_text_t*)ProgressBar->child_bk;
				size=GetStringSize((font_t*)Text->font,Text->text);
				Text->ctrl_sizex=(DWORD)size.sizex;
				Text->ctrl_sizey=(DWORD)size.sizey;
				Text->ctrl_x=x+(sizex/2)-(Text->ctrl_sizex/2);
				Text->ctrl_y=y+(sizey/2)-(Text->ctrl_sizey/2);
				
				SendMessage((struct HEADER*)ProgressBar,&message);
			}
		}

		if (ProgressBar->prb_flags & FLAG_PB_FROM_RIGHT_TO_LEFT_ON)
		{
			//Draw(fin,"filled_rectangle",x+sizex-pos_progress-1,y+1,x+sizex-1,sizey-2,0xff0000);
		}
		
	}
}

void DrawProgressBar(struct ControlProgressBar *ProgressBar)
{
	int		x,y,sizex,sizey;
	char		c;
	char		*save_buf,*buf;
	int		save_size_x,save_size_y;
	DWORD		draw_output,flags;
	finition_t	*fin;

	x=ProgressBar->ctrl_x;
	y=ProgressBar->ctrl_y;
	sizex=ProgressBar->ctrl_sizex;
	sizey=ProgressBar->ctrl_sizey;
	fin=(struct FINITION*)ProgressBar->finition;

	//alocate a buffer for draw text
	c=screen.bits_per_pixel >> 3;
	buf=malloc(sizex*sizey*c);

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

	//move control
	SetControlNewPosition(ProgressBar,0,0);
	//save finition parameters
	flags=fin->flags;
	fin->flags &=FINITION_OFF;

	//draw progress bar in buffer
	Draw(fin,TOOL_RECTANGLE,0,0,sizex,sizey,COLOR_ABSOLUTE_DARK);
	ProgressBarDrawProgress(ProgressBar);

	//restore last position of control
	SetControlNewPosition(ProgressBar,x,y);
	//restore finition
	fin->flags=flags;

	//restore screen parameters
	screen.buffer=save_buf;
	screen.size_x=save_size_x;
	screen.size_y=save_size_y;
	screen.draw_output=draw_output;

	//move rendered objects from local buffer to screen
	if (fin->flags & FINITION_ON)
		DrawImageFinit(fin,x,y,sizex,sizey,screen.bits_per_pixel,buf);
	else
		DrawImage(x,y,sizex,sizey,screen.bits_per_pixel,buf);

	//free local buffer
	free(buf);
}

void SetProgressBarPulse(struct ControlProgressBar *ProgressBar,int time_tick)
{
	struct TIMER *timer;
	struct	HEADERPARENT	*main_parent;

	main_parent=(struct	HEADERPARENT*)ProgressBar->main_parent;
	if (main_parent!=(struct HEADERPARENT*)NULL)
	{
		main_parent->number_timers_for_controls++;

		ProgressBar->timer=(DWORD*)SetTimerCallbackForControl(time_tick,&DrawProgressBar,ProgressBar);
		timer=(struct TIMER*)ProgressBar->timer;
		timer->flags=timer->flags | FLAG_TIMER_ON;
	}
}

//---------------------------------------------------------------------------------
//			 control ProgressBar
//---------------------------------------------------------------------------------
void ProgressBarProc(struct ControlProgressBar *ProgressBar,struct MESSAGE *message)
{
	int			btn_state;
	char			v;
	struct TIMER		*timer;
	struct	FINITION	*fin;

	switch(message->type)
	{
		case MESSAGE_FULL_REDRAW_ALL:
		{
			//draw ProgressBar
			if (ProgressBar->flags & FLAG_SHOW_CONTROL)	DrawProgressBar(ProgressBar);
			break;
		}
		case MESSAGE_FULL_REDRAW_ALL_WITH_FINITION:
		{
			fin=(struct FINITION*)ProgressBar->finition;
			fin->flags=fin->flags | FINITION_ON;
			fin->x=message->arg1;
			fin->y=message->arg2;
			fin->sizex=message->arg3;
			fin->sizey=message->arg4;
			DrawProgressBar(ProgressBar);
			break;
		}
		case MESSAGE_SPECIALIZED:
		{	//redraw bar of progress
			if (ProgressBar->flags & FLAG_GET_SPECIALIZED_MESSAGE_ON)
			{
				if (ProgressBar->flags & FLAG_SHOW_CONTROL)	ProgressBarDrawProgress(ProgressBar);
				ProgressBar->flags=ProgressBar->flags & FLAG_GET_SPECIALIZED_MESSAGE_OFF;
			}			
			break;
		}
		case MESSAGE_CHANGE_POSITION_EVENT:
		{
			ProgressBar->ctrl_x=ProgressBar->ctrl_x+message->arg1;
			ProgressBar->ctrl_y=ProgressBar->ctrl_y+message->arg2;
			SendMessage((struct HEADER*)ProgressBar,message);
			break;
		}
		case MESSAGE_CALL_TIMER_EVENT:
		{
			if (ProgressBar->timer!=(DWORD*)NULL)
			{
				timer=(struct TIMER*)ProgressBar->timer;
				if (timer->flags & FLAG_TIMER_ON)	Timer(timer);
			}
			SendMessage((struct HEADER*)ProgressBar,message);
			break;
		}
		case MESSAGE_DESTROY_CONTROL:
		{
			if (ProgressBar->timer!=(DWORD*)NULL)	free(ProgressBar->timer);
			free(ProgressBar->finition);
			SendMessage((struct HEADER*)ProgressBar,message);
			break;
		}
		case MESSAGE_SET_MAIN_PARENT:
		{
			SendMessage((struct HEADER*)ProgressBar,message);
			ProgressBar->main_parent=(DWORD*)message->arg1;
			SendMessage((struct HEADER*)ProgressBar,message);
			break;
		}

		default: break;
	}
}

//---------------------------------------------------------------------------------
//				create control ProgressBar
//---------------------------------------------------------------------------------
void* CreateProgressBarEmpty(struct ProgressBarData *info_for_control)
{
	struct ControlProgressBar *ProgressBar;
	struct FINITION		*fin;

	ProgressBar=malloc(sizeof(struct ControlProgressBar));
	ProgressBar->finition=malloc(sizeof(struct FINITION));
	fin=(struct FINITION*)ProgressBar->finition;
	fin->flags=0;

	ID++;
#ifdef	DEBUG
	printf("\ncreated progress bar with ID=%d",(int)ID);
#endif
	ProgressBar->child_bk=(DWORD*)NULL;
	ProgressBar->child_fd=(DWORD*)NULL;
	ProgressBar->active_control_for_keys=(DWORD*)NULL;
	ProgressBar->active_control_for_mouse=(DWORD*)NULL;
	ProgressBar->callback=(DWORD*)NULL;
	ProgressBar->timer=(DWORD*)NULL;
	ProgressBar->ctrl_proc=(DWORD*)&ProgressBarProc;
	ProgressBar->ctrl_x=(DWORD)info_for_control->x;
	ProgressBar->ctrl_y=(DWORD)info_for_control->y;
	ProgressBar->ctrl_sizex=(DWORD)info_for_control->width;
	ProgressBar->ctrl_sizey=(DWORD)info_for_control->height;
	ProgressBar->ctrl_ID=ID;
	ProgressBar->progress=info_for_control->progress;
	ProgressBar->flags=0;
	ProgressBar->flags=ProgressBar->flags | FLAG_SHOW_CONTROL;

	ProgressBar->prb_flags=0;
	ProgressBar->prb_flags=ProgressBar->prb_flags | FLAG_PB_HORIZONTAL_ORIENTATION_ON;
	ProgressBar->prb_flags=ProgressBar->prb_flags | FLAG_PB_FROM_LEFT_TO_RIGHT_ON;

	return(ProgressBar);
}

void* CreateProgressBar(gui_progress_bar_data_t *info)
{
	gui_text_t			*text;
	gui_text_data_t		txtdata;
	gui_progress_bar_t	*pbar;
	
	pbar=CreateProgressBarEmpty(info);
	
	txtdata.x=0;
	txtdata.y=0;
	txtdata.font=NULL;
	txtdata.background=FALSE;
	txtdata.color=0;
	txtdata.text=malloc(1024);
	txtdata.text[0]=0;
	text=CreateText(&txtdata);
	
	text->ctrl_x=(pbar->ctrl_sizex/2)-(text->ctrl_sizex/2);
	text->ctrl_y=(pbar->ctrl_sizey/2)-(text->ctrl_sizey/2);
	PackControls(pbar,text);
	
	text->flags &=FLAG_HIDE_CONTROL;

	return(pbar);
}

void	ProgressBarSetText(gui_progress_bar_t *pbar,char *txt)
{
	gui_text_t			*text;

	text=(gui_text_t*)pbar->child_bk;
	
	if (*txt!='\0')
	{
		pbar->prb_flags|=FLAG_PB_TEXT_ON;
		memmove(text->text,txt,strlen(txt));
		text->flags |=FLAG_SHOW_CONTROL;
	}
}

char	*ProgressBarGetText(gui_progress_bar_t *pbar)
{
	gui_text_t			*text;

	text=(gui_text_t*)pbar->child_bk;
	
	return(text->text);
}
