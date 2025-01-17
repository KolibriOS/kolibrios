#include "smalllibc/kosSyst.h"
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
	for (int y = position.Y; y < position.Y + height; y++)
		for (int x = position.X; x < position.X + width; x++)
			if (x >= 0 && y >= 0 && x < this->width && y < this->height)
				this->buffer[y * this->width + x] = img[(y - position.Y) * width + (x - position.X)];
}

int CKosRender::getPixel(int x, int y)
{
	return y * this->width + x;
}
