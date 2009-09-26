/*
	test libGUI library
*/
#include "stdarg.h"
#include "libGUI.h"
#include "stdlib.h"
#include "stdio.h"

void	callback_func_delete_window(header_t *control,void *data)
{
	QuitLibGUI((parent_t*)control);
}

int main(int argc, char *argv[])
{
	parent_t				*window;
	gui_image_data_t			imdata;
	gui_image_t				*image;
	int					i,j;
	unsigned int				*img;

	//load libGUI library
	LoadLibGUI(NULL);//use default system path to library
	//create main window
	window=CreateWindow();
	//change window size
	SetWindowSizeRequest(window,220,142);
	//set callback function for close window button
	SetCallbackFunction(window,DELETE_EVENT,&callback_func_delete_window,NULL);
	//create image
	imdata.x=5;
	imdata.y=5;
	imdata.width=200;
	imdata.height=100;
	imdata.bits_per_pixel=32;//bits per pixel

	image=CreateImage(&imdata);
	img=(unsigned int*)image->img;
	//generate 32 bits image
	for(i=0;i<GetControlSizeY(image);i++)
	{
		for(j=0;j<GetControlSizeX(image);j++)
		{
			*img=100*(i*i+j*j-i*3+2*j);
			img++;
		}
	}
	//pack image in window
	PackControls(window,image);
	//start main libGUI loop
	LibGUImain(window);
}
