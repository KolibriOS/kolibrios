/*
		control Text
*/


gui_text_size_t GetStringSize(font_t *font,char *s)
{
	long			len;
	gui_text_size_t size;

	len=strlen(s);
	if (font->size==FONT_CONSTANT_SIZE)
	{
		if (font->flags & FONT_FLAG_ORIENTATION_HORIZONTAL_FROM_LEFT_TO_RIGHT_ON)
		{
			size.sizex=len*font->sizex;
			size.sizey=font->sizey;
		}
	}

	return(size);
}

void TextBackgroundOn(gui_text_t *Text)
{
	Text->txt_flags |=TEXT_BACKGROUND_ON;
}

void TextBackgroundOff(gui_text_t *Text)
{
	Text->txt_flags &=TEXT_BACKGROUND_ON;
}

void DisplayText(gui_text_t *Text)
{
	int			x;
	int			y;
	int			sizex;
	int			sizey;
	char			v;
	font_t			*font;
	gui_text_size_t 	size;
	struct FINITION 	*fin;

	x=Text->ctrl_x;
	y=Text->ctrl_y;
	fin=(struct FINITION*)Text->finition;
	font=(font_t*)Text->font;

	v=Text->txt_flags & TEXT_BACKGROUND_ON;
	if (v)	font->flags|=FONT_FLAG_DRAW_BACKGROUND_ON;
	else	font->flags&=FONT_FLAG_DRAW_BACKGROUND_OFF;

	v=Text->txt_flags & TEXT_ORIENTATION_FROM_LEFT_TO_RIGHT_ON;
	if (v)	font->flags|=FONT_FLAG_ORIENTATION_HORIZONTAL_FROM_LEFT_TO_RIGHT_ON;
	else	font->flags&=FONT_FLAG_ORIENTATION_HORIZONTAL_FROM_LEFT_TO_RIGHT_OFF;

	size=GetStringSize((font_t*)Text->font,Text->text);
	Text->ctrl_sizex=(DWORD)size.sizex;
	Text->ctrl_sizey=(DWORD)size.sizey;

	DrawFont=(void(*)(finition_t *fin,int fx,int fy,DWORD color,
		DWORD background_color,font_t *font,BYTE *s))font->fnt_draw;
	DrawFont(fin,x,y,Text->color,Text->background_color,font,Text->text);
}

//---------------------------------------------------------------------------------
//			 control Text
//---------------------------------------------------------------------------------
void TextProc(gui_text_t *Text,gui_message_t *message)
{
	finition_t	*fin;

	switch(message->type)
	{
		case MESSAGE_FULL_REDRAW_ALL:
		{
			//draw Text
			if (Text->flags & FLAG_SHOW_CONTROL)	DisplayText(Text);
			break;
		}
		case MESSAGE_FULL_REDRAW_ALL_WITH_FINITION:
		{
			fin=(struct FINITION*)Text->finition;
			fin->flags=fin->flags | FINITION_ON;
			fin->x=message->arg1;
			fin->y=message->arg2;
			fin->sizex=message->arg3;
			fin->sizey=message->arg4;
			DisplayText(Text);
			break;
		}
		case MESSAGE_SPECIALIZED:
		{
			if (Text->flags & FLAG_GET_SPECIALIZED_MESSAGE_ON)
			{
				if (Text->flags & FLAG_SHOW_CONTROL)	DisplayText(Text);
				Text->flags=Text->flags & FLAG_GET_SPECIALIZED_MESSAGE_OFF;
			}
			break;
		}
		case MESSAGE_CHANGE_POSITION_EVENT:
		{
			Text->ctrl_x=Text->ctrl_x+message->arg1;
			Text->ctrl_y=Text->ctrl_y+message->arg2;
			break;
		}
		case MESSAGE_DESTROY_CONTROL:
		{
			free(Text->finition);
			break;
		}
		case MESSAGE_SET_MAIN_PARENT:
		{
			SendMessage((struct HEADER*)Text,message);
			Text->main_parent=(DWORD*)message->arg1;
			break;
		}

		default: break;
	}
	//send message to child controls(if there is)
	SendMessage((struct HEADER*)Text,message);
}

//---------------------------------------------------------------------------------
//				create control Text
//---------------------------------------------------------------------------------
void* CreateText(gui_text_data_t *info_for_control)
{
	gui_text_t		*Text;
	finition_t		*fin;
	gui_text_size_t size;

	Text=malloc(sizeof(struct ControlText));
	Text->finition=malloc(sizeof(struct FINITION));
	fin=(struct FINITION*)Text->finition;
	fin->flags=0;

	if (info_for_control->font==(DWORD*)NULL)	Text->font=FontsMeneger.default_font;
						else		Text->font=info_for_control->font;

	size=GetStringSize((font_t*)Text->font,info_for_control->text);

	ID++;
#ifdef	DEBUG
	printf("\ncreated text with ID=%d",(int)ID);
#endif
	Text->child_bk=(DWORD*)NULL;
	Text->child_fd=(DWORD*)NULL;
	Text->active_control_for_keys=(DWORD*)NULL;
	Text->active_control_for_mouse=(DWORD*)NULL;
	Text->callback=(DWORD*)NULL;
	Text->timer=(DWORD*)NULL;

	Text->ctrl_proc=(DWORD*)&TextProc;
	Text->ctrl_x=(DWORD)info_for_control->x;
	Text->ctrl_y=(DWORD)info_for_control->y;
	Text->ctrl_sizex=(DWORD)size.sizex;
	Text->ctrl_sizey=(DWORD)size.sizey;
	Text->ctrl_ID=ID;
	Text->color=info_for_control->color;
	Text->background_color=info_for_control->background_color;
	Text->text=info_for_control->text;
	Text->txt_flags=0;
	Text->txt_flags|=TEXT_ORIENTATION_FROM_LEFT_TO_RIGHT_ON;

	if (info_for_control->background)
	{
		Text->txt_flags|=TEXT_BACKGROUND_ON;
	}
	else
	{
		Text->txt_flags&=TEXT_BACKGROUND_OFF;
	}

	Text->flags=0;
	Text->flags=Text->flags | FLAG_SHOW_CONTROL;

	return(Text);
}
