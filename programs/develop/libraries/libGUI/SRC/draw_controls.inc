/*
	functions for draw controls
*/

//---------------------------------------------------------------------------------
//				fill buffer by value
//---------------------------------------------------------------------------------
void FillArrea(void *buf,DWORD size,BYTE bits_per_pixel,DWORD value)
{
	DWORD	i,j;
	char	r,g,b;

	switch(bits_per_pixel)
	{
		case 32:
		{
			j=(size >> 2);
			for(i=0;i<j;i++)
			{
				*(DWORD*)buf=(DWORD)value;
				buf=(DWORD*)buf+1;
			}
			break;
		}
		case 24:
		{
			b=value & 0xff;
			value=value >> 8;
			g=value & 0xff;
			value=value >> 8;
			r=value & 0xff;

			j=(size/3);
			for(i=0;i<j;i++)
			{
				*(char*)buf=(char)b;
				*((char*)buf+1)=(char)g;
				*((char*)buf+2)=(char)r;
				buf=(char*)buf+3;
			}
			break;
		}
		case 16:
		{
			break;
		}
		case 8:
		{
			break;
		}
		case 4:
		{
			break;
		}
		default: break;
	}
}

//---------------------------------------------------------------------------------
//				draw pixel on screen
//---------------------------------------------------------------------------------
void DrawPixel(int x,int y,DWORD color)
{
	char		r,g,b;
	char		*ptr;
	DWORD	*ptr2;

	if (x>screen.size_x-1) {x=screen.size_x-1;}
	if (y>screen.size_y-1) {y=screen.size_y-1;}
	if (x<0) {x=0;}
	if (y<0) {y=0;}

	switch(screen.draw_output)
	{
		case DRAW_OUTPUT_SCREEN:
		{
			x+=screen.x;
			y+=screen.y;
			gui_ksys_put_pixel_window(x,y,color);
			break;
		}
		case DRAW_OUTPUT_BUFFER:
		{
			ptr=screen.buffer;
			switch(screen.bits_per_pixel)
			{
				case 24:
				{
					ptr=ptr+(y*screen.size_x+x)*3;
					b=color & 0xff;
					color=color >>8;
					g=color & 0xff;
					color=color >>8;
					r=color & 0xff;

					ptr[0]=b;
					ptr[1]=g;
					ptr[2]=r;
					break;
				}
				case 32:
				{
					ptr2=(DWORD*)ptr+y*screen.size_x+x;
					*ptr2=color;
					break;
				}
				default: break;
			}
			break;
		}
	}
}

//---------------------------------------------------------------------------------
//		libGUI function drawing pixels on screen with finition
//---------------------------------------------------------------------------------
void DrawPixelFinit(struct FINITION *fin,int x,int y,DWORD color)
{
	if (x>=fin->x && x<=fin->x+fin->sizex-1 &&
		y>=fin->y && y<=fin->y+fin->sizey-1) DrawPixel(x,y,color);
}


//---------------------------------------------------------------------------------
//			get color of pixel inc coordinates x,y
//---------------------------------------------------------------------------------
DWORD GetColorPixel(int x,int y)
{
	char		r,g,b;
	char		*ptr;
	DWORD	color,coordinates;
	DWORD	*ptr2;

	if (x>screen.size_x-1) {x=screen.size_x-1;}
	if (y>screen.size_y-1) {y=screen.size_y-1;}
	if (x<0) {x=0;}
	if (y<0) {y=0;}

	switch(screen.draw_output)
	{
		case DRAW_OUTPUT_SCREEN:
		{
			x+=screen.x;
			y+=screen.y;
			coordinates=x;
			coordinates=coordinates << 16;
			coordinates +=y;
			color=gui_ksys_get_color_pixel_window(coordinates);
			break;
		}
		case DRAW_OUTPUT_BUFFER:
		{
			ptr=screen.buffer;
			switch(screen.bits_per_pixel)
			{
				case 24:
				{
					ptr=ptr+(y*screen.size_x+x)*3;
					color=*ptr;
					color=color & 0xffffff;
					break;
				}
				case 32:
				{
					ptr2=(DWORD*)ptr+y*screen.size_x+x;
					color=*ptr2;
					break;
				}
				default: break;
			}
		}
	}
	return(color);
}

//---------------------------------------------------------------------------------
//				draw vertical line on screen
//---------------------------------------------------------------------------------
void DrawVerticalLine(int x,int y1,int y2,DWORD color)
{
	char		r,g,b;
	char		*ptr;
	DWORD	*ptr2;
	int		a,i,count;

	if (x>screen.size_x-1) {x=screen.size_x-1;}
	if (y1>screen.size_y-1) {y1=screen.size_y-1;}
	if (y2>screen.size_y-1) {y2=screen.size_y-1;}
	if (x<0) {x=0;}
	if (y1<0) {y1=0;}
	if (y2<0) {y2=0;}

	switch(screen.draw_output)
	{
		case DRAW_OUTPUT_SCREEN:
		{
			x+=screen.x;
			y1+=screen.y;
			y2+=screen.y;
			gui_ksys_draw_line_window(x,y1,x,y2,color);
			break;
		}
		case DRAW_OUTPUT_BUFFER:
		{
			if (y1<y2)	{a=y1;count=y2-y1;}
				else
					{a=y2;count=y1-y2;}

			ptr=screen.buffer;
			switch(screen.bits_per_pixel)
			{
				case 24:
				{
					ptr=ptr+(a*screen.size_x+x)*3;
					b=color & 0xff;
					color=color >>8;
					g=color & 0xff;
					color=color >>8;
					r=color & 0xff;

					a=screen.size_x*3;
					for(i=0;i<=count;i++)
					{
						ptr[0]=b;
						ptr[1]=g;
						ptr[2]=r;
						ptr+=a;
					}
					break;
				}
				case 32:
				{
					ptr2=(DWORD*)ptr+a*screen.size_x+x;
					a=screen.size_x;
					for(i=0;i<=count;i++)
					{
						*ptr2=color;
						ptr2+=a;
					}
					break;
				}
				default: break;
			}
		}
	}
}

//---------------------------------------------------------------------------------
//			draw finited vertical line on screen
//---------------------------------------------------------------------------------
void DrawVerticalLineFinit(struct FINITION *fin,int x,int y1,int y2,DWORD color)
{
	int	xmin,xmax,ymin,ymax,v,sy;

	xmin=fin->x;
	xmax=fin->x+fin->sizex-1;
	ymin=fin->y;
	ymax=fin->y+fin->sizey-1;

	if (y2<y1)
	{
		v=y1;
		y1=y2;
		y2=v;
	}

	if (x<xmin) return;
	if (x>xmax) return;
	if (y2<ymin) return;
	if (y1>ymax) return;

	//finit x coordinates and sizex
	sy=y1-ymin;

	if (sy>=0)
	{
		if (y2>ymax) y2=ymax;
	}
	else
	{
		y1=ymin;
		if (y2>ymax)	y2=ymax;
	}
	DrawVerticalLine(x,y1,y2,color);
}

//---------------------------------------------------------------------------------
//			draw horizontal line on screen
//---------------------------------------------------------------------------------
void DrawHorizontalLine(int x1,int x2,int y,DWORD color)
{
	char		r,g,b;
	char		*ptr;
	int		a,i,count;
	DWORD	*ptr2;

	if (y>screen.size_y-1) {y=screen.size_y-1;}
	if (x1>screen.size_x-1) {x1=screen.size_x-1;}
	if (x2>screen.size_x-1) {x2=screen.size_x-1;}
	if (y<0) {y=0;}
	if (x1<0) {x1=0;}
	if (x2<0) {x2=0;}

	switch(screen.draw_output)
	{
		case DRAW_OUTPUT_SCREEN:
		{
			x1+=screen.x;
			x2+=screen.x;
			y+=screen.y;
			gui_ksys_draw_line_window(x1,y,x2,y,color);
			break;
		}
		case DRAW_OUTPUT_BUFFER:
		{
			if (x1<x2)	{a=x1;count=x2-x1;}
				else
					{a=x2;count=x1-x2;}

			ptr=screen.buffer;
			switch(screen.bits_per_pixel)
			{
				case 24:
				{
					ptr=ptr+(y*screen.size_x+a)*3;
					b=color & 0xff;
					color=color >>8;
					g=color & 0xff;
					color=color >>8;
					r=color & 0xff;

					for(i=0;i<=count;i++)
					{
						ptr[0]=b;
						ptr[1]=g;
						ptr[2]=r;
						ptr=ptr+3;
					}
					break;
				}
				case 32:
				{
					ptr2=(DWORD*)ptr+y*screen.size_x+a;

					for(i=0;i<=count;i++)
					{
						*ptr2=color;ptr2++;
					}
					break;
				}
				default:break;
			}
		}
	}
}

//---------------------------------------------------------------------------------
//			draw finited vertical line on screen
//---------------------------------------------------------------------------------
void DrawHorizontalLineFinit(struct FINITION *fin,int x1,int x2,int y,DWORD color)
{
	int	xmin,xmax,ymin,ymax,v,sx;

	xmin=fin->x;
	xmax=fin->x+fin->sizex-1;
	ymin=fin->y;
	ymax=fin->y+fin->sizey-1;

	if (x2<x1)
	{
		v=x1;
		x1=x2;
		x2=v;
	}

	if (y<ymin) return;
	if (y>ymax) return;
	if (x2<xmin) return;
	if (x1>xmax) return;

	//finit x coordinates and sizex
	sx=x1-xmin;

	if (sx>=0)
	{
		if (x2>xmax) x2=xmax;
	}
	else
	{
		x1=xmin;
		if (x2>xmax)	x2=xmax;
	}
	DrawHorizontalLine(x1,x2,y,color);
}

//---------------------------------------------------------------------------------
//			libGUI function drawing line on screen
//---------------------------------------------------------------------------------
int sign(int value)
{
	if (value==0) return(0);
	if (value<0) {return(-1);}
	else {return(1);}
}

int abs(int value)
{
	if (value<0) {value=-value;}

	return(value);
}

void DrawLine(int x1, int y1, int x2, int y2,DWORD color)
{
    int x;
    int y;
    int dx;
    int dy;
    int sx;
    int sy;
    int z;
    int e;
    int i;
    char ch;

	switch(screen.draw_output)
	{
		case DRAW_OUTPUT_SCREEN:
		{
			if (y1>screen.size_y-1) {y1=screen.size_y-1;}
			if (y2>screen.size_y-1) {y2=screen.size_y-1;}
			if (x1>screen.size_x-1) {x1=screen.size_x-1;}
			if (x2>screen.size_x-1) {x2=screen.size_x-1;}
			if (y1<0) {y1=0;}
			if (y2<0) {y2=0;}
			if (x1<0) {x1=0;}
			if (x2<0) {x2=0;}

			x1+=screen.x;
			x2+=screen.x;
			y1+=screen.y;
			y2+=screen.y;
			gui_ksys_draw_line_window(x1,y1,x2,y2,color);
			break;
		}
		case DRAW_OUTPUT_BUFFER:
		{

			x = x1;
			y = y1;
			dx = abs(x2-x1);
			dy = abs(y2-y1);
			sx = sign(x2-x1);
			sy = sign(y2-y1);

			if( dx==0 && dy==0 )
			{
				DrawPixel(x1, y1,color);
				return;
			}
			if( dy>dx )
			{
				z = dx;
				dx = dy;
				dy = z;
				ch = 1;
			}
			else
			{
				ch = 0;
			}
			e = 2*dy-dx;
			i = 1;

			do
			{
				DrawPixel(x,y,color);
				while(e>=0)
				{
					if( ch==1 )	x = x+sx;
						else	y = y+sy;

					if( ch==1 )	y = y+sy;
						else	x = x+sx;

					e = e-2*dx;
				}
				if( ch==1 )		y = y+sy;
					else		x = x+sx;

				e = e+2*dy;
				i++;
			}
			while(i<=dx);
			DrawPixel(x, y,color);
			break;
		}
	}
}

//---------------------------------------------------------------------------------
//			libGUI function drawing rectangle on screen
//---------------------------------------------------------------------------------
void DrawRectangle(int x,int y,int size_x,int size_y,DWORD color)
{

	if (size_x==0 || size_y==0) return;

	DrawHorizontalLine(x,x+size_x-1,y,color);
	DrawVerticalLine(x+size_x-1,y,y+size_y-1,color);
	DrawHorizontalLine(x,x+size_x-1,y+size_y-1,color);
	DrawVerticalLine(x,y,y+size_y-1,color);
}

//---------------------------------------------------------------------------------
//			libGUI function drawing finited rectangle on screen
//---------------------------------------------------------------------------------
void DrawRectangleFinit(struct FINITION *fin,int x,int y,int size_x,int size_y,DWORD color)
{

	if (size_x==0 || size_y==0) return;

	DrawHorizontalLineFinit(fin,x,x+size_x-1,y,color);
	DrawVerticalLineFinit(fin,x+size_x-1,y,y+size_y-1,color);
	DrawHorizontalLineFinit(fin,x,x+size_x-1,y+size_y-1,color);
	DrawVerticalLineFinit(fin,x,y,y+size_y-1,color);
}

//---------------------------------------------------------------------------------
//			libGUI function drawing filled rectangle on screen
//---------------------------------------------------------------------------------
void DrawFilledRectangle(int x,int y,int size_x,int size_y,DWORD color)
{
	int	i,j;
	int	x1,y1,x2,y2;

	if (size_x==0 || size_y==0) return;

	switch(screen.draw_output)
	{
		case DRAW_OUTPUT_SCREEN:
		{
			x1=x;
			y1=y;
			x2=x+size_x-1;
			y2=y+size_y-1;
			if (y1>screen.size_y-1) {y1=screen.size_y-1;}
			if (y2>screen.size_y-1) {y2=screen.size_y-1;}
			if (x1>screen.size_x-1) {x1=screen.size_x-1;}
			if (x2>screen.size_x-1) {x2=screen.size_x-1;}
			if (y1<0) {y1=0;}
			if (y2<0) {y2=0;}
			if (x1<0) {x1=0;}
			if (x2<0) {x2=0;}

			size_x=x2-x1+1;
			size_y=y2-y1+1;
			x1+=screen.x;
			y1+=screen.y;

			gui_ksys_draw_filled_rectangle_window(x1,y1,size_x,size_y,color);
			break;
		}
		case DRAW_OUTPUT_BUFFER:
		{
			j=y;
			for(i=0;i<size_y;i++)
			{
				DrawHorizontalLine(x,x+size_x-1,j,color);
				j++;
			}
			break;
		}
	}
}

//---------------------------------------------------------------------------------
//	libGUI function drawing filled gradient up and down rectangles on screen
//---------------------------------------------------------------------------------
void DrawGradientUpDownFilledRectangle(char flag_up,int x,int y,int sizex,int sizey,DWORD color_from,DWORD color_to)
{
	unsigned	char	r_f,g_f,b_f;
	unsigned	char	r_t,g_t,b_t;
	unsigned	char	r,g,b;
	float			f_r,f_g,f_b;
	float			d_r,d_g,d_b;
	float			div;
	DWORD		color;
	int			i,j,dj;

	if (sizex==0 || sizey==0) return;

	color=color_from;
	b_f=color & 0xff;
	color=color >> 8;
	g_f=color & 0xff;
	color=color >> 8;
	r_f=color & 0xff;

	color=color_to;
	b_t=color & 0xff;
	color=color >> 8;
	g_t=color & 0xff;
	color=color >> 8;
	r_t=color & 0xff;

	div=sizey-1;
	d_r=(float)(r_t-r_f);
	d_r=d_r/div;
	d_g=(float)(g_t-g_f);
	d_g=d_g/div;
	d_b=(float)(b_t-b_f);
	d_b=d_b/div;

	f_r=r_f;
	f_g=g_f;
	f_b=b_f;

	r=r_f;
	g=g_f;
	b=b_f;

	if (flag_up==TRUE)
	{
		j=y+sizey-1;
		dj=-1;
	}
	else
	{
		j=y;
		dj=1;
	}


	for(i=0;i<sizey;i++)
	{
		color=0;
		color=color | r;
		color=color << 8;
		color=color | g;
		color=color << 8;
		color=color | b;

		DrawHorizontalLine(x,x+sizex-1,j,color);

		f_r=f_r+d_r;
		f_g=f_g+d_g;
		f_b=f_b+d_b;

		r=(char)f_r;
		g=(char)f_g;
		b=(char)f_b;

		j=j+dj;
	}

}

//---------------------------------------------------------------------------------
//	libGUI function drawing filled gradient left and right rectangles on screen
//---------------------------------------------------------------------------------
void DrawGradientLeftRightFilledRectangle(char flag_left,int x,int y,int sizex,int sizey,DWORD color_from,DWORD color_to)
{
	unsigned	char	r_f,g_f,b_f;
	unsigned	char	r_t,g_t,b_t;
	unsigned	char	r,g,b;
	float			f_r,f_g,f_b;
	float			d_r,d_g,d_b;
	float			div;
	DWORD		color;
	int			i,j,dj;

	if (sizex==0 || sizey==0) return;

	color=color_from;
	b_f=color & 0xff;
	color=color >> 8;
	g_f=color & 0xff;
	color=color >> 8;
	r_f=color & 0xff;

	color=color_to;
	b_t=color & 0xff;
	color=color >> 8;
	g_t=color & 0xff;
	color=color >> 8;
	r_t=color & 0xff;

	div=sizex-1;
	d_r=(float)(r_t-r_f);
	d_r=d_r/div;
	d_g=(float)(g_t-g_f);
	d_g=d_g/div;
	d_b=(float)(b_t-b_f);
	d_b=d_b/div;

	f_r=r_f;
	f_g=g_f;
	f_b=b_f;

	r=r_f;
	g=g_f;
	b=b_f;

	if (flag_left==TRUE)
	{
		j=x;
		dj=1;
	}
	else
	{
		j=x+sizex-1;
		dj=-1;
	}

	for(i=0;i<sizex;i++)
	{
		color=0;
		color=color | r;
		color=color << 8;
		color=color | g;
		color=color << 8;
		color=color | b;

		DrawVerticalLine(j,y,y+sizey-1,color);

		f_r=f_r+d_r;
		f_g=f_g+d_g;
		f_b=f_b+d_b;

		r=(char)f_r;
		g=(char)f_g;
		b=(char)f_b;

		j=j+dj;
	}

}

//---------------------------------------------------------------------------------
//			libGUI function drawing circle on screen
//---------------------------------------------------------------------------------
void DrawCircle(int xc, int yc, int r,DWORD color)
{
	int x;
	int y;
	int d;

	x = 0;
	y = r;
	d = 3-2*r;
	while(y>=x)
	{
		DrawPixel(x+xc,y+yc,color);
		DrawPixel(x+xc,-y+yc,color);
		DrawPixel(-x+xc,y+yc,color);
		DrawPixel(-x+xc,-y+yc,color);
		DrawPixel(y+xc,x+yc,color);
		DrawPixel(y+xc,-x+yc,color);
		DrawPixel(-y+xc,x+yc,color);
		DrawPixel(-y+xc,-x+yc,color);
		if( d<0 )
		{
			d = d+4*x+6;
		}
		else
		{
			d = d+4*(x-y)+10;
			y--;
		}
		x++;
	}
}

//---------------------------------------------------------------------------------
//		libGUI function drawing circle on screen with finition
//---------------------------------------------------------------------------------
void DrawCircleFinit(struct FINITION *fin,int xc, int yc, int r,DWORD color)
{
	int x;
	int y;
	int d;

	x = 0;
	y = r;
	d = 3-2*r;
	while(y>=x)
	{
		DrawPixelFinit(fin,x+xc,y+yc,color);
		DrawPixelFinit(fin,x+xc,-y+yc,color);
		DrawPixelFinit(fin,-x+xc,y+yc,color);
		DrawPixelFinit(fin,-x+xc,-y+yc,color);
		DrawPixelFinit(fin,y+xc,x+yc,color);
		DrawPixelFinit(fin,y+xc,-x+yc,color);
		DrawPixelFinit(fin,-y+xc,x+yc,color);
		DrawPixelFinit(fin,-y+xc,-x+yc,color);
		if( d<0 )
		{
			d = d+4*x+6;
		}
		else
		{
			d = d+4*(x-y)+10;
			y--;
		}
		x++;
	}
}

//---------------------------------------------------------------------------------
//		libGUI function drawing filled circle on screen
//---------------------------------------------------------------------------------
void DrawFilledCircle(int xc, int yc, int r,DWORD color)
{
	int		xl,yu,xr,yd;
	int		i,j;
	DWORD	pixcolor;

	DrawCircle(xc,yc,r,color);

	yu=yc;
	yd=yc;
	for(i=0;i<r;i++)
	{
		xl=xc;
		xr=xc+1;
		j=0;
		//fill left up
		while((pixcolor=GetColorPixel(xl,yu))!=color)
		{
			DrawPixel(xl,yu,color);
			xl--;
			j++;
			if (j>r) break;
		}
		j=0;
		//fill right up
		while((pixcolor=GetColorPixel(xr,yu))!=color)
		{
			DrawPixel(xr,yu,color);
			xr++;
			j++;
			if (j>r) break;
		}

		xl=xc;
		xr=xc+1;
		j=0;
		//fill left down
		while((pixcolor=GetColorPixel(xl,yd))!=color)
		{
			DrawPixel(xl,yd,color);
			xl--;
			j++;
			if (j>r) break;
		}
		j=0;
		//fill right down
		while((pixcolor=GetColorPixel(xr,yd))!=color)
		{
			DrawPixel(xr,yd,color);
			xr++;
			j++;
			if (j>r) break;
		}
		yu--;
		yd++;
	}
}

//---------------------------------------------------------------------------------
//	libGUI function drawing filled circle on screen with finition
//---------------------------------------------------------------------------------
void DrawFilledCircleFinit(struct FINITION *fin,int xc, int yc, int r,DWORD color)
{
	int		xl,yu,xr,yd;
	int		i,j;
	DWORD	pixcolor;

	DrawCircleFinit(fin,xc,yc,r,color);

	yu=yc;
	yd=yc;
	for(i=0;i<r;i++)
	{
		xl=xc;
		xr=xc+1;
		//fill left up
		for(j=0;j<r;j++)
		{
			if (xl>=fin->x && xl<=fin->x+fin->sizex &&
				yu>=fin->y && yu<=fin->y+fin->sizey)
			{
				pixcolor=GetColorPixel(xl,yu);
				if (pixcolor!=color)	DrawPixel(xl,yu,color);
					else break;
			}
			xl--;
		}
		//fill right up
		for(j=0;j<r;j++)
		{
			if (xr>=fin->x && xr<=fin->x+fin->sizex &&
				yu>=fin->y && yu<=fin->y+fin->sizey)
			{
				pixcolor=GetColorPixel(xr,yu);
				if (pixcolor!=color)	DrawPixel(xr,yu,color);
					else break;
			}
			xr++;
		}

		xl=xc;
		xr=xc+1;
		//fill left down
		for(j=0;j<r;j++)
		{
			if (xl>=fin->x && xl<=fin->x+fin->sizex &&
				yd>=fin->y && yd<=fin->y+fin->sizey)
			{
				pixcolor=GetColorPixel(xl,yd);
				if (pixcolor!=color)	DrawPixel(xl,yd,color);
					else break;
			}
			xl--;
		}
		//fill right down
		for(j=0;j<r;j++)
		{
			if (xr>=fin->x && xr<=fin->x+fin->sizex &&
				yd>=fin->y && yd<=fin->y+fin->sizey)
			{
				pixcolor=GetColorPixel(xr,yd);
				if (pixcolor!=color)	DrawPixel(xr,yd,color);
					else break;
			}
			xr++;
		}
		yu--;
		yd++;
	}
}

//---------------------------------------------------------------------------------
//			libGUI function drawing ellipse on screen
//---------------------------------------------------------------------------------
void DrawEllipse(int x,int y,int a,int b,DWORD color)
{
	int	col,i,row,bnew;
	long	a_square,b_square,two_a_square,two_b_square,four_a_square,four_b_square,d;

	b_square=b*b;
	a_square=a*a;
	row=b;
	col=0;
	two_a_square=a_square<<1;
	four_a_square=a_square<<2;
	four_b_square=b_square<<2;
	two_b_square=b_square<<1;
	d=two_a_square*((row-1)*(row))+a_square+two_b_square*(1-a_square);
	while(a_square*(row)>b_square*(col))
	{
		DrawPixel(col+x,row+y,color);
		DrawPixel(col+x,y-row,color);
		DrawPixel(x-col,row+y,color);
		DrawPixel(x-col,y-row,color);

		if (d>=0)
		{
			row--;
			d-=four_a_square*(row);
		}
		d+=two_b_square*(3+(col<<1));
		col++;
	}

	d=two_b_square*(col+1)*col+two_a_square*(row*(row-2)+1)+(1-two_a_square)*b_square;
	while ((row) + 1)
	{
		DrawPixel(col+x, row+y, color);
		DrawPixel(col+x, y-row, color);
		DrawPixel(x-col, row+y, color);
		DrawPixel(x-col, y-row, color);

		if (d<=0)
		{
			col++;
			d+=four_b_square*col;
		}
		row--;
		d+=two_a_square*(3-(row <<1));
	}
}

//---------------------------------------------------------------------------------
//		libGUI function drawing ellipse on screen with finition
//---------------------------------------------------------------------------------
void DrawEllipseFinit(struct FINITION *fin,int x,int y,int a,int b,DWORD color)
{
	int	col,i,row,bnew;
	long	a_square,b_square,two_a_square,two_b_square,four_a_square,four_b_square,d;

	b_square=b*b;
	a_square=a*a;
	row=b;
	col=0;
	two_a_square=a_square<<1;
	four_a_square=a_square<<2;
	four_b_square=b_square<<2;
	two_b_square=b_square<<1;
	d=two_a_square*((row-1)*(row))+a_square+two_b_square*(1-a_square);

	while(a_square*(row)>b_square*(col))
	{
		DrawPixelFinit(fin,col+x,row+y,color);
		DrawPixelFinit(fin,col+x,y-row,color);
		DrawPixelFinit(fin,x-col,row+y,color);
		DrawPixelFinit(fin,x-col,y-row,color);

		if (d>=0)
		{
			row--;
			d-=four_a_square*(row);
		}
		d+=two_b_square*(3+(col<<1));
		col++;
	}

	d=two_b_square*(col+1)*col+two_a_square*(row*(row-2)+1)+(1-two_a_square)*b_square;
	while ((row) + 1)
	{
		DrawPixelFinit(fin,col+x, row+y, color);
		DrawPixelFinit(fin,col+x, y-row, color);
		DrawPixelFinit(fin,x-col, row+y, color);
		DrawPixelFinit(fin,x-col, y-row, color);

		if (d<=0)
		{
			col++;
			d+=four_b_square*col;
		}
		row--;
		d+=two_a_square*(3-(row <<1));
	}
}

//---------------------------------------------------------------------------------
//		libGUI function drawing filled ellips on screen
//---------------------------------------------------------------------------------
void DrawFilledEllipse(int xc, int yc, int a,int b,DWORD color)
{
	int		xl,yu,xr,yd;
	int		i,j;
	DWORD	pixcolor;

	DrawEllipse(xc,yc,a,b,color);

	yu=yc;
	yd=yc;
	for(i=0;i<b;i++)
	{
		xl=xc;
		xr=xc+1;
		j=0;
		//fill left up
		while((pixcolor=GetColorPixel(xl,yu))!=color)
		{
			DrawPixel(xl,yu,color);
			xl--;
			j++;
			if (j>a) break;
		}
		j=0;
		//fill right up
		while((pixcolor=GetColorPixel(xr,yu))!=color)
		{
			DrawPixel(xr,yu,color);
			xr++;
			j++;
			if (j>a) break;
		}

		xl=xc;
		xr=xc+1;
		j=0;
		//fill left down
		while((pixcolor=GetColorPixel(xl,yd))!=color)
		{
			DrawPixel(xl,yd,color);
			xl--;
			j++;
			if (j>a) break;
		}
		j=0;
		//fill right down
		while((pixcolor=GetColorPixel(xr,yd))!=color)
		{
			DrawPixel(xr,yd,color);
			xr++;
			j++;
			if (j>a) break;
		}
		yu--;
		yd++;
	}
}

//---------------------------------------------------------------------------------
//	libGUI function drawing filled circle on screen with finition
//---------------------------------------------------------------------------------
void DrawFilledEllipseFinit(struct FINITION *fin,int xc, int yc, int a,int b,DWORD color)
{
	int		xl,yu,xr,yd;
	int		i,j;
	DWORD	pixcolor;

	DrawEllipseFinit(fin,xc,yc,a,b,color);

	yu=yc;
	yd=yc;
	for(i=0;i<b;i++)
	{
		xl=xc;
		xr=xc+1;
		//fill left up
		for(j=0;j<a;j++)
		{
			if (xl>=fin->x && xl<=fin->x+fin->sizex &&
				yu>=fin->y && yu<=fin->y+fin->sizey)
			{
				pixcolor=GetColorPixel(xl,yu);
				if (pixcolor!=color)	DrawPixel(xl,yu,color);
					else break;
			}
			xl--;
		}
		//fill right up
		for(j=0;j<a;j++)
		{
			if (xr>=fin->x && xr<=fin->x+fin->sizex &&
				yu>=fin->y && yu<=fin->y+fin->sizey)
			{
				pixcolor=GetColorPixel(xr,yu);
				if (pixcolor!=color)	DrawPixel(xr,yu,color);
					else break;
			}
			xr++;
		}

		xl=xc;
		xr=xc+1;
		//fill left down
		for(j=0;j<a;j++)
		{
			if (xl>=fin->x && xl<=fin->x+fin->sizex &&
				yd>=fin->y && yd<=fin->y+fin->sizey)
			{
				pixcolor=GetColorPixel(xl,yd);
				if (pixcolor!=color)	DrawPixel(xl,yd,color);
					else break;
			}
			xl--;
		}
		//fill right down
		for(j=0;j<a;j++)
		{
			if (xr>=fin->x && xr<=fin->x+fin->sizex &&
				yd>=fin->y && yd<=fin->y+fin->sizey)
			{
				pixcolor=GetColorPixel(xr,yd);
				if (pixcolor!=color)	DrawPixel(xr,yd,color);
					else break;
			}
			xr++;
		}
		yu--;
		yd++;
	}
}

//---------------------------------------------------------------------------------
//		libGUI function drawing image on screen
//---------------------------------------------------------------------------------
void	DrawImage(int x,int y,int sizex,int sizey,char bits_per_pixel,char *img)
{
	char			r,g,b;
	int			i,j,countx,county;
	int			x2,y2;
	DWORD			pitch_src,pitch_screen,add_src,add_screen;
	char			*ptr_src,*ptr_screen,ptr_max;
	char			*ptr_src2,*ptr_screen2;
	DWORD			*ptr_src3;

	if (img==NULL)	return;
	if (x>screen.size_x-1) return;
	if (y>screen.size_y-1) return;

	x2=x+sizex-1;
	y2=y+sizey-1;


	if (y2>screen.size_y-1) {y2=screen.size_y-1;}
	if (x2>screen.size_x-1) {x2=screen.size_x-1;}
	if (y<0) y=0;
	if (y2<0) y2=0;
	if (x<0) x=0;
	if (x2<0) x2=0;

	countx=x2-x+1;
	county=y2-y+1;

	switch(screen.draw_output)
	{
		case DRAW_OUTPUT_SCREEN:
		{
			x+=screen.x;
			y+=screen.y;

			switch(screen.bits_per_pixel)
			{
				case 32:
				case 24:
				{
					switch(bits_per_pixel)
					{//check bits per pixel in picture
						case 32:
						{//convert 32 bit image to 24 bit image
							j=sizex*sizey;
							ptr_src=img;
							ptr_screen=malloc(j*3);
							ptr_screen2=ptr_screen;
							for(i=0;i<j;i++)
							{
								*(char*)ptr_screen=*(char*)ptr_src;
								*((char*)ptr_screen+1)=*((char*)ptr_src+1);
								*((char*)ptr_screen+2)=*((char*)ptr_src+2);

								ptr_src+=4;
								ptr_screen+=3;
							}
							gui_ksys_put_image_window(ptr_screen2,x,y,countx,county);
							free(ptr_screen2);
							break;
						}
						case 24:
						{
							gui_ksys_put_image_window(img,x,y,countx,county);
							break;
						}
					}
					break;
				}
				default: break;
			}
			break;
		}
		case DRAW_OUTPUT_BUFFER:
		{
			switch(screen.bits_per_pixel)
			{
				case 24:
				{//check source image resolution
					switch(bits_per_pixel)
					{
						case 32://display 32 bit image in 24 bit mode
						{//convert and draw 32 bit image in 24 bit buffer
							ptr_screen=screen.buffer+(screen.size_x*y+x)*3;
							add_screen=(screen.size_x-countx)*3;
							ptr_src=img;
							//copy line of byte with size x
							for(i=0;i<county;i++)
							{
								for(j=0;j<countx;j++)
								{
									*(char*)ptr_screen=*(char*)ptr_src;
									*((char*)ptr_screen+1)=*((char*)ptr_src+1);
									*((char*)ptr_screen+2)=*((char*)ptr_src+2);
		
									ptr_src+=4;
									ptr_screen+=3;
								}
								ptr_screen+=add_screen;
							}
							break;
						}
						case 24:
						{//display 24 bit image in 24 bit buffer
							ptr_screen=screen.buffer+(screen.size_x*y+x)*3;
							add_screen=screen.size_x*3;
							add_src=countx*3;

							//copy line of byte with size x
							for(i=0;i<county;i++)
							{
								memmove(ptr_screen,img,add_src);
								ptr_screen+=add_screen;
								img+=add_src;
							}
							break;
						}
						default: break;
					}
					break;
				}
				case 32:
				{	//check source image resolution
					switch(bits_per_pixel)
					{
						case 32:
						{//display 32 bit image in 32 bit mode
							ptr_screen=screen.buffer+(screen.size_x*y+x)*4;
							add_screen=screen.size_x*4;
							add_src=countx*4;

							//copy line of byte with size x
							for(i=0;i<county;i++)
							{
								memmove(ptr_screen,img,add_src);
								ptr_screen+=add_screen;
								img+=add_src;
							}

							break;
						}
						case 24://display 24 bit image in 32 bit mode
						{
							ptr_screen=screen.buffer+(screen.size_x*y+x)*4;
							add_screen=(screen.size_x-countx)*4;
							ptr_src=img;
							//copy line of byte with size x
							for(i=0;i<county;i++)
							{
								for(j=0;j<countx;j++)
								{
									*(char*)ptr_screen=*(char*)ptr_src;
									*((char*)ptr_screen+1)=*((char*)ptr_src+1);
									*((char*)ptr_screen+2)=*((char*)ptr_src+2);
		
									ptr_src+=3;
									ptr_screen+=4;
								}
								ptr_screen+=add_screen;
							}
							break;
						}
						default: break;
					}
					default: break;
				}
			}
		}
	}
}

//---------------------------------------------------------------------------------
//	libGUI function drawing image on screen with finition
//---------------------------------------------------------------------------------
void	DrawImageFinit(finition_t *fin,int x,int y,int sizex,int sizey,char bits_per_pixel,char *img)
{
	int		x1,y1,x2,y2,sx,sy;
	int		xmin,xmax,ymin,ymax;
	int		countline,countx,county,i,j;
	DWORD	pitch_src,add_src;
	char		*ptr_dest,*ptr_src;
	char		bytes_per_pixel;
	char		*buf;

	xmin=fin->x;
	xmax=fin->x+fin->sizex-1;
	ymin=fin->y;
	ymax=fin->y+fin->sizey-1;

	x1=x;
	y1=y;
	x2=x1+sizex-1;
	y2=y1+sizey-1;

	if (x2<xmin) return;
	if (x1>xmax) return;
	if (y2<ymin) return;
	if (y1>ymax) return;

	if (x1>=xmin && x2<=xmax && y1>=ymin && y2<=ymax)
	{
		DrawImage(x,y,sizex,sizey,bits_per_pixel,img);
		return;
	}

	//finit x coordinates and sizex
	sx=x1-xmin;

	if (sx>=0)
	{
		if (x2>xmax) x2=xmax;
	}
	else
	{
		x1=xmin;
		if (x2>xmax) x2=xmax;
	}

	//finit y coordinates and sizey
	sy=y1-ymin;

	if (sy>=0)
	{
		if (y2>ymax) y2=ymax;
	}
	else
	{
		y1=ymin;
		if (y2>ymax) y2=ymax;
	}
	countx=x2-x1+1;
	county=y2-y1+1;

	//cut finited rectangle from image and move them into buffer

	bytes_per_pixel=bits_per_pixel >> 3;
	buf=malloc(countx*county*bytes_per_pixel);
	ptr_dest=buf;

	pitch_src=sizex*bytes_per_pixel;
	ptr_src=img+pitch_src*(y1-y)+(x1-x)*bytes_per_pixel;
	add_src=sizex*bytes_per_pixel;
	countline=countx*bytes_per_pixel;

	switch(bits_per_pixel)
	{
		case 32:
		case 24:
		case 16:
		case 8:
		{
			for(i=0;i<county;i++)
			{
				memmove(ptr_dest,ptr_src,countline);
				ptr_src+=add_src;
				ptr_dest+=countline;
			}
			break;
		}
		default: break;
	}
	//draw cutted by finition image
	DrawImage(x1,y1,countx,county,bits_per_pixel,buf);
	free(buf);
}

//---------------------------------------------------------------------------------
//				libGUI function draw
//---------------------------------------------------------------------------------
void Draw(struct FINITION *finition,int tool_name,...)
{
	va_list arguments;
	char		*img;
	char		first,second,bits_per_pixel;
	int		xmin,xmax,ymin,ymax;
	int		radius;
	int		x,y,x2,y2,x_i,y_i;
	int		sizex,sizey,sx,sy;
	float		x_l1,y_l1,x_l2,y_l2,k,b;
	DWORD	color,color_from,color_to;

	arguments=(va_list)&tool_name+sizeof(int);

	xmin=finition->x;
	xmax=finition->x+finition->sizex-1;
	ymin=finition->y;
	ymax=finition->y+finition->sizey-1;

	switch(tool_name)
	{
		//tool Pixel
		case TOOL_PIXEL:
		{
			x=va_arg(arguments,int);
			y=va_arg(arguments,int);
			color=va_arg(arguments,DWORD);

			if (finition->flags & FINITION_ON)
			{
				if (x>=finition->x && x<=finition->x+finition->sizex
					&& y>=finition->y && y<=finition->y+finition->sizey)
				{	//pixel inside finition arrea
					DrawPixel(x,y,color);
				}
			}
			else
			{	//no finition
				DrawPixel(x,y,color);
			}
			break;
		}
		//tool Line
		case TOOL_LINE:
		{
			x=va_arg(arguments,int);
			y=va_arg(arguments,int);
			x2=va_arg(arguments,int);
			y2=va_arg(arguments,int);
			color=va_arg(arguments,DWORD);

			if (finition->flags & FINITION_ON)
			{
				sizex=abs(x2-x);
				sizey=abs(y2-y);

				if (CheckCrossRectangles(finition->x,finition->y,finition->sizex,
								finition->sizey,x,y,sizex,sizey)==FALSE) break;

				if (x>=finition->x && x<=finition->x+finition->sizex &&
					y>=finition->y && y<=finition->y+finition->sizey &&
					x2>=finition->x && x2<=finition->x+finition->sizex &&
					y2>=finition->y && y2<=finition->y+finition->sizey)
				{
					//line inside finition arrea
					DrawLine(x,y,x2,y2,color);
					break;
				}


				if (x==x2) break;

				//find coefficients of line
				x_l1=x;
				y_l1=y;
				x_l2=x2;
				y_l2=y2;
				k=(y_l1-y_l2)/(x_l1-x_l2);
				b=(y_l2*x_l1-y_l1*x_l2)/(x_l1-x_l2);

				sx=x2-x;//vectore x
				sy=y2-y;//vectore y

				if (x>=finition->x && x<=finition->x+finition->sizex &&
					y>=finition->y && y<=finition->y+finition->sizey)
				{	//point x,y inside finition arrea
					//check cross with left vertical line of finition
					y_l1=k*finition->x+b;
					y_i=(int)y_l1;
					if (y_i>=finition->y && y_i<=finition->y+finition->sizey && sx<0)
					{	//first point for finited line
						x2=finition->x;
						y2=y_i;
						DrawLine(x,y,x2,y2,color);
						break;
					}
					//check cross with up horizontal line of finition
					x_l1=(finition->y-b)/k;
					x_i=(int)x_l1;
					if (x_i>=finition->x && x_i<=finition->x+finition->sizex && sy<0)
					{
						x2=x_i;
						y2=finition->y;
						DrawLine(x,y,x2,y2,color);
						break;
					}
					//check cross with right vertical line of finition
					y_l1=k*(finition->x+finition->sizex)+b;
					y_i=(int)y_l1;
					if (y_i>=finition->y && y_i<=finition->y+finition->sizey && sx>0)
					{
						x2=finition->x+finition->sizex;
						y2=y_i;
						DrawLine(x,y,x2,y2,color);
						break;
					}
					//check cross with down horizontal line of finition
					x_l1=((finition->y+finition->sizey)-b)/k;
					x_i=(int)x_l1;
					if (x_i>=finition->x && x_i<=finition->x+finition->sizex && sy>0)
					{
						x2=x_i;
						y2=finition->y+finition->sizey;
						DrawLine(x,y,x2,y2,color);
						break;
					}
				}

				if (x2>=finition->x && x2<=finition->x+finition->sizex &&
					y2>=finition->y && y2<=finition->y+finition->sizey)
				{	//point x,y inside finition arrea
					//check cross with left vertical line of finition
					y_l1=k*finition->x+b;
					y_i=(int)y_l1;
					if (y_i>=finition->y && y_i<=finition->y+finition->sizey && sx>0)
					{	//first point for finited line
						x=finition->x;
						y=y_i;
						DrawLine(x,y,x2,y2,color);
						break;
					}
					//check cross with up horizontal line of finition
					x_l1=(finition->y-b)/k;
					x_i=(int)x_l1;
					if (x_i>=finition->x && x_i<=finition->x+finition->sizex && sy>0)
					{
						x=x_i;
						y=finition->y;
						DrawLine(x,y,x2,y2,color);
						break;
					}
					//check cross with right vertical line of finition
					y_l1=k*(finition->x+finition->sizex)+b;
					y_i=(int)y_l1;
					if (y_i>=finition->y && y_i<=finition->y+finition->sizey && sx<0)
					{
						x=finition->x+finition->sizex;
						y=y_i;
						DrawLine(x,y,x2,y2,color);
						break;
					}
					//check cross with down horizontal line of finition
					x_l1=((finition->y+finition->sizey)-b)/k;
					x_i=(int)x_l1;
					if (x_i>=finition->x && x_i<=finition->x+finition->sizex && sy<0)
					{
						x=x_i;
						y=finition->y+finition->sizey;
						DrawLine(x,y,x2,y2,color);
						break;
					}
				}

				first=FALSE;
				second=FALSE;
				//check cross with left vertical line of finition
				y_l1=k*finition->x+b;
				y_i=(int)y_l1;
				if (y_i>=finition->y && y_i<=finition->y+finition->sizey)
				{	//first point for finited line
					x=finition->x;
					y=y_i;
					first=TRUE;
				}
				//check cross with up horizontal line of finition
				x_l1=(finition->y-b)/k;
				x_i=(int)x_l1;
				if (x_i>=finition->x && x_i<=finition->x+finition->sizex)
				{
					if (first==FALSE)
					{
						x=x_i;
						y=finition->y;
					}
					else
					{
						x2=x_i;
						y2=finition->y;
						second=TRUE;
					}
				}
				//check cross with right vertical line of finition
				y_l1=k*(finition->x+finition->sizex)+b;
				y_i=(int)y_l1;
				if (y_i>=finition->y && y_i<=finition->y+finition->sizey)
				{
					if (first==FALSE)
					{
						x=finition->x+finition->sizex;
						y=y_i;
					}
					else
					{
						x2=finition->x+finition->sizex;
						y2=y_i;
						second=TRUE;
					}
				}
				//check cross with down horizontal line of finition
				x_l1=((finition->y+finition->sizey)-b)/k;
				x_i=(int)x_l1;
				if (x_i>=finition->x && x_i<=finition->x+finition->sizex)
				{
					if (first==FALSE)
					{
						x=x_i;
						y=finition->y+finition->sizey;
					}
					else
					{
						x2=x_i;
						y2=finition->y+finition->sizey;
						second=TRUE;
					}
				}

				if (first==TRUE && second==TRUE)
				{
					//draw finited line
					DrawLine(x,y,x2,y2,color);
				}
				break;

			}
			else
			{	//no finition
				DrawLine(x,y,x2,y2,color);
			}
			break;
		}
		//tool VerticalLine
		case TOOL_VERTICAL_LINE:
		{
			x=va_arg(arguments,int);
			y=va_arg(arguments,int);
			y2=va_arg(arguments,int);
			color=va_arg(arguments,DWORD);

			if (finition->flags & FINITION_ON)
			{
				DrawVerticalLineFinit(finition,x,y,y2,color);
			}
			else
			{
				DrawVerticalLine(x,y,y2,color);
			}
			break;
		}
		//tool HorizontalLine
		case TOOL_HORIZONTAL_LINE:
		{
			x=va_arg(arguments,int);
			x2=va_arg(arguments,int);
			y=va_arg(arguments,int);
			color=va_arg(arguments,DWORD);

			if (finition->flags & FINITION_ON)
			{
				DrawHorizontalLineFinit(finition,x,x2,y,color);
			}
			else
			{
				DrawHorizontalLine(x,x2,y,color);
			}
			break;
		}

		//tool Rectangle
		case TOOL_RECTANGLE:
		{
			x=va_arg(arguments,int);
			y=va_arg(arguments,int);
			sizex=va_arg(arguments,int);
			sizey=va_arg(arguments,int);
			color=va_arg(arguments,DWORD);

			if (finition->flags & FINITION_ON)
			{
				x2=x+sizex-1;
				y2=y+sizey-1;

				if (x2<xmin) return;
				if (x>xmax) return;
				if (y2<ymin) return;
				if (y>ymax) return;

				DrawRectangleFinit(finition,x,y,sizex,sizey,color);
			}
			else
			{
				DrawRectangle(x,y,sizex,sizey,color);
			}
			break;
		}
		//tool FilledRectangle
		case TOOL_FILLED_RECTANGLE:
		{
			x=va_arg(arguments,int);
			y=va_arg(arguments,int);
			sizex=va_arg(arguments,int);
			sizey=va_arg(arguments,int);
			color=va_arg(arguments,DWORD);

			if (finition->flags & FINITION_ON)
			{
				x2=x+sizex-1;
				y2=y+sizey-1;

				if (x2<xmin) return;
				if (x>xmax) return;
				if (y2<ymin) return;
				if (y>ymax) return;

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

				sizex=x2-x+1;
				sizey=y2-y+1;

				DrawFilledRectangle(x,y,sizex,sizey,color);
			}
			else
			{
				DrawFilledRectangle(x,y,sizex,sizey,color);
			}
			break;
		}
		//tool GradientUpFilledRectangle
		case TOOL_GRADIENT_UP_FILLED_RECTANGLE:
		{
			x=va_arg(arguments,int);
			y=va_arg(arguments,int);
			sizex=va_arg(arguments,int);
			sizey=va_arg(arguments,int);
			color_from=va_arg(arguments,DWORD);
			color_to=va_arg(arguments,DWORD);

			if (finition->flags & FINITION_ON)
			{
				x2=x+sizex-1;
				y2=y+sizey-1;

				if (x2<xmin) return;
				if (x>xmax) return;
				if (y2<ymin) return;
				if (y>ymax) return;

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

				sizex=x2-x+1;
				sizey=y2-y+1;

				DrawGradientUpDownFilledRectangle(TRUE,x,y,sizex,sizey,color_from,color_to);
			}
			else
			{
				DrawGradientUpDownFilledRectangle(TRUE,x,y,sizex,sizey,color_from,color_to);
			}
			break;
		}
		//tool GradientDownFilledRectangle
		case TOOL_GRADIENT_DOWN_FILLED_RECTANGLE:
		{
			x=va_arg(arguments,int);
			y=va_arg(arguments,int);
			sizex=va_arg(arguments,int);
			sizey=va_arg(arguments,int);
			color_from=va_arg(arguments,DWORD);
			color_to=va_arg(arguments,DWORD);

			if (finition->flags & FINITION_ON)
			{
				x2=x+sizex-1;
				y2=y+sizey-1;

				if (x2<xmin) return;
				if (x>xmax) return;
				if (y2<ymin) return;
				if (y>ymax) return;

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

				sizex=x2-x+1;
				sizey=y2-y+1;

				DrawGradientUpDownFilledRectangle(FALSE,x,y,sizex,sizey,color_from,color_to);
			}
			else
			{
				DrawGradientUpDownFilledRectangle(FALSE,x,y,sizex,sizey,color_from,color_to);
			}
			break;
		}
		//tool GradientLeftFilledRectangle
		case TOOL_GRADIENT_LEFT_FILLED_RECTANGLE:
		{
			x=va_arg(arguments,int);
			y=va_arg(arguments,int);
			sizex=va_arg(arguments,int);
			sizey=va_arg(arguments,int);
			color_from=va_arg(arguments,DWORD);
			color_to=va_arg(arguments,DWORD);

			if (finition->flags & FINITION_ON)
			{
				x2=x+sizex-1;
				y2=y+sizey-1;

				if (x2<xmin) return;
				if (x>xmax) return;
				if (y2<ymin) return;
				if (y>ymax) return;

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

				sizex=x2-x+1;
				sizey=y2-y+1;

				DrawGradientLeftRightFilledRectangle(TRUE,x,y,sizex,sizey,color_from,color_to);
			}
			else
			{
				DrawGradientLeftRightFilledRectangle(TRUE,x,y,sizex,sizey,color_from,color_to);
			}
			break;
		}
		//tool GradientRightFilledRectangle
		case TOOL_GRADIENT_RIGHT_FILLED_RECTANGLE:
		{
			x=va_arg(arguments,int);
			y=va_arg(arguments,int);
			sizex=va_arg(arguments,int);
			sizey=va_arg(arguments,int);
			color_from=va_arg(arguments,DWORD);
			color_to=va_arg(arguments,DWORD);

			if (finition->flags & FINITION_ON)
			{
				x2=x+sizex-1;
				y2=y+sizey-1;

				if (x2<xmin) return;
				if (x>xmax) return;
				if (y2<ymin) return;
				if (y>ymax) return;

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

				sizex=x2-x+1;
				sizey=y2-y+1;

				DrawGradientLeftRightFilledRectangle(FALSE,x,y,sizex,sizey,color_from,color_to);
			}
			else
			{
				DrawGradientLeftRightFilledRectangle(FALSE,x,y,sizex,sizey,color_from,color_to);
			}
			break;
		}
		//tool Circle
		case TOOL_CIRCLE:
		{
			x=va_arg(arguments,int);
			y=va_arg(arguments,int);
			radius=va_arg(arguments,int);
			color=va_arg(arguments,DWORD);

			if (finition->flags & FINITION_ON)
			{
				DrawCircleFinit(finition,x,y,radius,color);
			}
			else
			{
				DrawCircle(x,y,radius,color);
			}
			break;
		}
		//tool FilledCircle
		case TOOL_FILLED_CIRCLE:
		{
			x=va_arg(arguments,int);
			y=va_arg(arguments,int);
			radius=va_arg(arguments,int);
			color=va_arg(arguments,DWORD);

			if (finition->flags & FINITION_ON)
			{
				DrawFilledCircleFinit(finition,x,y,radius,color);
			}
			else
			{
				DrawFilledCircle(x,y,radius,color);
			}
			break;
		}
		//tool Ellipse
		case TOOL_ELLIPSE:
		{
			x=va_arg(arguments,int);
			y=va_arg(arguments,int);
			sizex=va_arg(arguments,int);
			sizey=va_arg(arguments,int);
			color=va_arg(arguments,DWORD);

			if (finition->flags & FINITION_ON)
			{
				DrawEllipseFinit(finition,x,y,sizex,sizey,color);
			}
			else
			{
				DrawEllipse(x,y,sizex,sizey,color);
			}
			break;
		}
		//tool FilledEllipse
		case TOOL_FILLED_ELLIPSE:
		{
			x=va_arg(arguments,int);
			y=va_arg(arguments,int);
			sizex=va_arg(arguments,int);
			sizey=va_arg(arguments,int);
			color=va_arg(arguments,DWORD);

			if (finition->flags & FINITION_ON)
			{
				DrawFilledEllipseFinit(finition,x,y,sizex,sizey,color);
			}
			else
			{
				DrawFilledEllipse(x,y,sizex,sizey,color);
			}
			break;
		}
		//tool Image
		case TOOL_IMAGE:
		{
			x=va_arg(arguments,int);
			y=va_arg(arguments,int);
			sizex=va_arg(arguments,int);
			sizey=va_arg(arguments,int);
			bits_per_pixel=(char)va_arg(arguments,int);
			img=va_arg(arguments,char*);

			if (finition->flags & FINITION_ON)
			{
				DrawImageFinit(finition,x,y,sizex,sizey,bits_per_pixel,img);
			}
			else
			{
				DrawImage(x,y,sizex,sizey,bits_per_pixel,img);
			}
			break;
		}
	}
	va_end(arguments);
}


