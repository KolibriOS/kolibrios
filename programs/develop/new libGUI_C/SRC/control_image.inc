/*
		control Image
*/

void DisplayImage(gui_image_t *Image)
{
	int	x;
	int	y;
	int	sizex;
	int	sizey;
	struct FINITION *fin;

	x=Image->ctrl_x;
	y=Image->ctrl_y;
	sizex=Image->ctrl_sizex;
	sizey=Image->ctrl_sizey;
	fin=(struct FINITION*)Image->finition;

	Draw(fin,TOOL_IMAGE,x,y,sizex,sizey,Image->bits_per_pixel,Image->img);
}

//---------------------------------------------------------------------------------
//			 control Image
//---------------------------------------------------------------------------------
void ImageProc(gui_image_t *Image,gui_message_t *message)
{
	finition_t	*fin;

	switch(message->type)
	{
		case MESSAGE_FULL_REDRAW_ALL:
		{
			//draw Image
			if (Image->flags & FLAG_SHOW_CONTROL)	DisplayImage(Image);
			break;
		}
		case MESSAGE_FULL_REDRAW_ALL_WITH_FINITION:
		{
			fin=(struct FINITION*)Image->finition;
			fin->flags=fin->flags | FINITION_ON;
			fin->x=message->arg1;
			fin->y=message->arg2;
			fin->sizex=message->arg3;
			fin->sizey=message->arg4;
			DisplayImage(Image);
			break;
		}
		case MESSAGE_SPECIALIZED:
		{
			if (Image->flags & FLAG_GET_SPECIALIZED_MESSAGE_ON)
			{
				if (Image->flags & FLAG_SHOW_CONTROL)	DisplayImage(Image);
				Image->flags=Image->flags & FLAG_GET_SPECIALIZED_MESSAGE_OFF;
			}			
			break;
		}
		case MESSAGE_CHANGE_POSITION_EVENT:
		{
			Image->ctrl_x=Image->ctrl_x+message->arg1;
			Image->ctrl_y=Image->ctrl_y+message->arg2;
			break;
		}
		case MESSAGE_DESTROY_CONTROL:
		{
			free(Image->finition);
			free(Image->img);
			break;
		}
		case MESSAGE_SET_MAIN_PARENT:
		{
			SendMessage((struct HEADER*)Image,message);
			Image->main_parent=(DWORD*)message->arg1;
			break;
		}

		default: break;
	}
	//send message to child controls(if there is)
	SendMessage((struct HEADER*)Image,message);
}

//---------------------------------------------------------------------------------
//				create control Image
//---------------------------------------------------------------------------------
void* CreateImage(struct ImageData *info_for_control)
{
	gui_image_t	*Image;
	finition_t	*fin;
	DWORD		sizemem;

	Image=malloc(sizeof(struct ControlImage));
	Image->finition=malloc(sizeof(struct FINITION));
	fin=(struct FINITION*)Image->finition;
	fin->flags=0;

	ID++;
#ifdef	DEBUG
	printf("\ncreated image with ID=%d",(int)ID);
#endif
	Image->child_bk=(DWORD*)NULL;
	Image->child_fd=(DWORD*)NULL;
	Image->active_control_for_keys=(DWORD*)NULL;
	Image->active_control_for_mouse=(DWORD*)NULL;
	Image->callback=(DWORD*)NULL;
	Image->timer=(DWORD*)NULL;

	Image->ctrl_proc=(DWORD*)&ImageProc;
	Image->ctrl_x=(DWORD)info_for_control->x;
	Image->ctrl_y=(DWORD)info_for_control->y;
	Image->ctrl_sizex=(DWORD)info_for_control->width;
	Image->ctrl_sizey=(DWORD)info_for_control->height;
	Image->bits_per_pixel=info_for_control->bits_per_pixel;
	Image->ctrl_ID=ID;
	Image->flags=0;
	Image->flags=Image->flags | FLAG_SHOW_CONTROL;

	switch(Image->bits_per_pixel)
	{
		case 32: 
		{
			sizemem=(Image->ctrl_sizex*Image->ctrl_sizey)*4;
			Image->bytes_per_pixel=4;
			break;
		}
		case 24: 
		{
			sizemem=(Image->ctrl_sizex*Image->ctrl_sizey)*3;
			Image->bytes_per_pixel=3;
			break;
		}
		case 16: 
		{
			sizemem=(Image->ctrl_sizex*Image->ctrl_sizey)*2;
			Image->bytes_per_pixel=2;
			break;
		}
		case 15: 
		{
			sizemem=(Image->ctrl_sizex*Image->ctrl_sizey)*2;
			Image->bytes_per_pixel=2;
			break;
		}
		case 8: 
		{
			sizemem=Image->ctrl_sizex*Image->ctrl_sizey;
			Image->bytes_per_pixel=1;
			break;
		}
		case 4: 
		{
			sizemem=((Image->ctrl_sizex*Image->ctrl_sizey)>>1)+1;
			Image->bytes_per_pixel=0;
			break;
		}
		default: return(NULL);
	}

	Image->img=malloc(sizemem);

	return(Image);
}

