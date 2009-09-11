/*
	menegement of fonts
 */

//---------------------------------------------------------------------------------
//			destroy font
//---------------------------------------------------------------------------------
void DestroyFont(font_t *font)
{
	font_t	*seek_font;
	font_t	*exchange_font;
	font_t	*last_font;
	font_t	*next_font;

	if (font==(font_t*)NULL) return;

	if (FontsMeneger.fnt_bk==FontsMeneger.fnt_fd)
	{
		//parent have got alone font
		if (FontsMeneger.fnt_bk==(DWORD*)font)
		{
#ifdef	DEBUG
			printf("\ndestroyed font %d font name=%s",(int)font,(char*)font->fnt_name);
#endif
			free(font);
			FontsMeneger.fnt_bk=(DWORD*)NULL;
			FontsMeneger.fnt_fd=(DWORD*)NULL;
		}
		return;
	}

	seek_font=(font_t*)FontsMeneger.fnt_bk;
	while(seek_font!=(font_t*)NULL)
	{
		if (seek_font==font)
		{
			//delete font from fonts's cash
			last_font=(font_t*)seek_font->fnt_bk;
			next_font=(font_t*)seek_font->fnt_fd;

			if ((last_font!=(font_t*)NULL) && (next_font!=(font_t*)NULL))
			{//deliting font isn't first font and isn't endest
				next_font->fnt_bk=(DWORD*)last_font;
				last_font->fnt_fd=(DWORD*)next_font;
			}
			else
			{
				if (last_font==(font_t*)NULL)
				{//deliting font is first font of Parend
					FontsMeneger.fnt_bk=(DWORD*)next_font;
					next_font->fnt_bk=(DWORD*)NULL;
					if (next_font->fnt_fd==(DWORD*)NULL)
					{
						FontsMeneger.fnt_fd=(DWORD*)next_font;
						FontsMeneger.fnt_bk=(DWORD*)next_font;
					}
				}

				if (next_font==(font_t*)NULL)
				{
					//there isn't next fonts
					last_font->fnt_fd=(DWORD*)NULL;
					FontsMeneger.fnt_fd=(DWORD*)last_font;
				}
			}
#ifdef	DEBUG
			printf("\ndestroyed font %d font name=%s",(int)font,(char*)font->fnt_name);
#endif
			free(font);
			break;
		}
		exchange_font=(font_t*)seek_font->fnt_fd;
		seek_font=exchange_font;
	}
}

void	*CreateFont(void)
{
	font_t	*font;
	font_t	*backward_font;

	font=malloc(sizeof(struct FONT));

	if (FontsMeneger.fnt_bk==(DWORD*)NULL)
	{//not yet fonts
		FontsMeneger.fnt_bk=(DWORD*)font;
		FontsMeneger.fnt_fd=(DWORD*)font;
		font->fnt_bk=(DWORD*)NULL;
		font->fnt_fd=(DWORD*)NULL;
	}
	else
	{
		backward_font=(font_t*)FontsMeneger.fnt_fd;
		FontsMeneger.fnt_fd=(DWORD*)font;
		backward_font->fnt_fd=(DWORD*)font;
		font->fnt_bk=(DWORD*)backward_font;
		font->fnt_fd=(DWORD*)NULL;
	}
	return(font);
}
////////////////////////////////////////////////////////////////////////////////
//			CHAR.MT and CHAR2.MT fonts unpacker
////////////////////////////////////////////////////////////////////////////////
void	CHAR_MT_FontsUnpacker(unsigned char *loaded_font,font_t *font,char font_type)
{
	int		i,j,k;
	unsigned char	c;
	unsigned char	*p;

	if (font_type==FALSE)
	{//CHAR.MT
		font->font=malloc(6*9*256);
		p=(unsigned char*)font->font;

		for(j=0;j<256;j++)
		{
			for(i=0;i<9;i++)
			{
				c=(unsigned char)(*loaded_font);
				p[5]=(unsigned char)(c & 0x20);
				p[4]=(unsigned char)(c & 0x10);
				p[3]=(unsigned char)(c & 0x8);
				p[2]=(unsigned char)(c & 0x4);
				p[1]=(unsigned char)(c & 0x2);
				p[0]=(unsigned char)(c & 0x1);

				p+=6;
				loaded_font++;
			}
		}
		font->sizex=6;
		font->sizey=9;
	}
	else
	{//CHAR2.MT
		font->font=malloc(8*10*256);
		p=(unsigned char*)font->font;

		for(j=0;j<256;j++)
		{
			for(i=0;i<10;i++)
			{
				c=(unsigned char)(*loaded_font);
				p[7]=(unsigned char)(c & 0x80);
				p[6]=(unsigned char)(c & 0x40);
				p[5]=(unsigned char)(c & 0x20);
				p[4]=(unsigned char)(c & 0x10);
				p[3]=(unsigned char)(c & 0x8);
				p[2]=(unsigned char)(c & 0x4);
				p[1]=(unsigned char)(c & 0x2);
				p[0]=(unsigned char)(c & 0x1);

				p+=8;
				loaded_font++;
			}
		}
		font->sizex=8;
		font->sizey=10;
	}

	font->size=FONT_CONSTANT_SIZE;
	font->type=FONT_TYPE_ASCII;
	font->encoding_type=FONT_ENCODING_CYRILLIC_IBM866;
}

////////////////////////////////////////////////////////////////////////////////
//			CHAR.MT and CHAR2.MT fonts draw
////////////////////////////////////////////////////////////////////////////////
void	MonofontDraw(finition_t *fin,int fx,int fy,
				DWORD color,DWORD background_color,
				font_t *font,unsigned char *s)
{
	unsigned int		i,j,k,step,len;
	int		x,y,size_x,save_size_x,save_size_y;
	unsigned char	*p,*buf,*save_buf;
	unsigned char	c,draw_output;


	step=font->sizex*font->sizey;
	len=strlen(s);

	if (font->flags & FONT_FLAG_ORIENTATION_HORIZONTAL_FROM_LEFT_TO_RIGHT_ON)
	{
		if (font->flags & FONT_FLAG_DRAW_BACKGROUND_ON)
		{//there is a fon and not finition for draw
			//alocate a buffer for draw text
			size_x=font->sizex*len;

			c=screen.bits_per_pixel >> 3;
			i=step*c*len;
			buf=malloc(i);

			//save current screen parameters
			save_buf=screen.buffer;
			save_size_x=screen.size_x;
			save_size_y=screen.size_y;
			draw_output=screen.draw_output;

			//load parameters of local buffer
			screen.buffer=buf;
			screen.size_x=size_x;
			screen.size_y=font->sizey;
			screen.draw_output=DRAW_OUTPUT_BUFFER;

			//fill buffer by background color
			FillArrea(buf,i,screen.bits_per_pixel,background_color);

			//draw text
			x=0;
			for(k=0;k<len;k++)
			{
				c=s[k];
				p=font->font+step*c;
				for(j=0;j<font->sizey;j++)
				{

					for(i=0;i<font->sizex;i++)
					{
						if (*p) DrawPixel(x+i,j,color);
						p++;
					}
				}
				x=x+font->sizex;
			}

			//restore screen parameters
			screen.buffer=save_buf;
			screen.size_x=save_size_x;
			screen.size_y=save_size_y;
			screen.draw_output=draw_output;

			//move text from local buffer to screen
			if (fin->flags & FINITION_ON)
				DrawImageFinit(fin,fx,fy,size_x,font->sizey,screen.bits_per_pixel,buf);
			else
				DrawImage(fx,fy,size_x,font->sizey,screen.bits_per_pixel,buf);

			//free local buffer
			free(buf);
		}
		else
		{
			if (fin->flags & FINITION_ON)
			{//not background and finition for draw
				x=fx;
				y=fy;
				for(k=0;k<len;k++)
				{
					c=s[k];
					p=font->font+step*c;
					for(j=0;j<font->sizey;j++)
					{

						for(i=0;i<font->sizex;i++)
						{
							if (*p) DrawPixelFinit(fin,x+i,y+j,color);
							p++;
						}
					}
					x=x+font->sizex;
				}
			}
			else
			{//not background and not finition for draw
				x=fx;
				y=fy;
				for(k=0;k<len;k++)
				{
					c=s[k];
					p=font->font+step*c;
					for(j=0;j<font->sizey;j++)
					{

						for(i=0;i<font->sizex;i++)
						{
							if (*p) DrawPixel(x+i,y+j,color);
							p++;
						}
					}
					x=x+font->sizex;
				}
			}

		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//				fonts loader
////////////////////////////////////////////////////////////////////////////////
font_t	*LoadFont(char *fullfontname)
{
	char		*path;
	char		*c;
	BYTE		*buf;
	long		filesize;
	int		len1,len2,i;
	font_t		*font;
	static DWORD	buf_for_size[2];
	static DWORD	buf_for_pos[2];

	path=malloc(263);
	if (strchr(fullfontname,'/')==NULL)
	{
		len1=strlen(default_fonts_path);
		len2=strlen(fullfontname);
		memmove(path,default_fonts_path,len1);
		memmove(path+len1,fullfontname,len2);
		*(path+len1+len2)='\0';
	}
	else
		memmove(path,fullfontname,strlen(fullfontname));

	if (gui_get_file_size(path,buf_for_size)==KOLIBRIOS_SYS_FILE_ACCESS_SUCCESSFULLY)
	{
		filesize=buf_for_size[0];
		buf=malloc(filesize);
		//load fonts in buffer
		gui_read_file(path,buf_for_pos,filesize,buf);

		free(path);
		//register font
		font=CreateFont();
		c=strrchr(fullfontname,'/');
		if(c==NULL)	font->fnt_name=fullfontname;
			else
				font->fnt_name=(char*)(c+1);

		//check font type
		c=strstr(font->fnt_name,"CHAR.MT");//check standart type of fonts
		if (c!=NULL)
		{
			font->fnt_unpacker=(DWORD*)&CHAR_MT_FontsUnpacker;
			font->fnt_draw=(DWORD*)&MonofontDraw;
			CHAR_MT_FontsUnpacker(buf,font,FALSE);//CHAR.MT
			free(buf);
			font->flags=0;
			font->flags|=FONT_FLAG_ORIENTATION_HORIZONTAL_FROM_LEFT_TO_RIGHT_ON;
		}

		c=strstr(font->fnt_name,"CHAR2.MT");//check standart type of fonts
		if (c!=NULL)
		{
			font->fnt_unpacker=(DWORD*)&CHAR_MT_FontsUnpacker;
			font->fnt_draw=(DWORD*)&MonofontDraw;
			CHAR_MT_FontsUnpacker(buf,font,TRUE);//CHAR2.MT
			free(buf);
			font->flags=0;
			font->flags|=FONT_FLAG_ORIENTATION_HORIZONTAL_FROM_LEFT_TO_RIGHT_ON;
		}
		//not other fonts yet

	}
	else
	{
#ifdef	DEBUG
		printf("\ncannot load font %s",path);
#endif
		free(path);
		return(NULL);
	}

	FontsMeneger.number_fonts++;
	return(font);
}

void	FreeFont(font_t *font)
{
	if (font==(font_t*)FontsMeneger.default_font)	return;

	free(font->font);
	DestroyFont(font);
}
