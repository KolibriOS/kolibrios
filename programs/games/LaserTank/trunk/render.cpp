#include "kosSyst.h"
#include "render.h"

CKosRender::CKosRender(int width, int height)
{
	this->width = width;
	this->height = height;
	this->buffer = new RGB[width * height];
	for (int i = 0; i < width * height; i++)
		this->buffer[i] = 0x000000;
}

CKosRender::~CKosRender(void)
{
	//delete this->buffer;
}

void CKosRender::Draw(Point position)
{
	kos_PutImage((RGB*)this->buffer, this->width, this->height, position.X, position.Y);
}

void CKosRender::RenderImg(RGB *img, Point position, int width, int height)
{
	for (int i = 0; i < width * height; i++)
		this->buffer[i] = img[i];
		//	if ( )
		//		this->buffer[getPixel(x, y)]
}

int CKosRender::getPixel(int x, int y)
{
	return y * this->width + x;
}
