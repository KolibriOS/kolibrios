#include "smalllibc/kosSyst.h"
#include "render.h"
#include "smalllibc/func.h"
#include "image.h"
//#include "mymath.h"

CKosImage::CKosImage(CKosRender *render, RGBA *buffer, int width, int height)
{
	this->isColor = false;
	this->width = width;
	this->height = height;
	this->buffer = buffer;
	this->render = render;
	this->mode = DRAW_ALPHA;
	this->frame = 0;
	this->frameWidth = 0;
	this->frameHeight = 0;
}

CKosImage::~CKosImage(void)
{

}

void CKosImage::SetMode(int mode)
{
	this->mode = mode;
}

void CKosImage::SetFrameSize(int width, int height)
{
	this->frameWidth = width;
	this->frameHeight = height;
}

void CKosImage::Draw(Point position, float angle, RGB color)
{
	this->isColor = true;
	this->color = color;
	this->Draw(position, angle);
	this->isColor = false;
}

int CKosImage::getPixel(int x, int y)
{
	return y * this->width + x;
}

void CKosImage::Draw(Point position, float angle, int frame)
{
	this->frame = frame;
	Draw(position, angle);
}

void CKosImage::Draw(Point position, float angle, int frame, RGB color)
{
	this->isColor = true;
	this->color = color;
	this->frame = frame;
	Draw(position, angle);
	this->isColor = false;
}

void CKosImage::Draw(Point position, float angle)
{
	float alpha;
	Point p, p1, p2;
	RGB pixel, newPixel;
	RGBA addPixel;
	int PixelID;
	Point fix;
	if (angle == 270)
		fix = Point(0, -1);
	else
		if (angle == 180)
			fix = Point(-1, -1);
		else
			if (angle == 90)
				fix = Point(-1, 0);
			else
				fix = Point(0, 0);
	
	Point center = Point(this->width / 2, this->height / 2);
	double a = -angle * (3.14 / 180);
	
	double SinRad = sin(a);
	double CosRad = cos(a);

	for (int y = 0; y < this->height; ++y)
		for (int x = 0; x < this->width; ++x)
		{
			p1 = Point(x, y) - center;

			p.X = roundInt(p1.X * CosRad - p1.Y * SinRad) + center.X;
			p.Y = roundInt(p1.X * SinRad + p1.Y * CosRad) + center.Y;

			p2 = Point(x + fix.X + position.X, y + fix.Y + position.Y);

			if (p.X >= 0 && p.X < this->width && p.Y >= 0 && p.Y < this->height 
				&& p2.X >= 0 && p2.Y >= 0 && p2.X < this->render->width && p2.Y < this->render->height)
			{
				p.Y += this->frame * this->frameHeight;
				addPixel = this->buffer[this->getPixel(p.X, p.Y)];
				PixelID = this->render->getPixel(p2.X, p2.Y);
				pixel = this->render->buffer[PixelID];

				if (addPixel.a > 0)
				{
					if (this->isColor)
					{
						addPixel.r = this->color.r;
						addPixel.g = this->color.g;
						addPixel.b = this->color.b;
					}

					alpha = (float)addPixel.a / 255.0f;

					if (this->mode == DRAW_ALPHA)
					{
						newPixel.r = di((double)(pixel.r * (1 - alpha) + addPixel.r * alpha));
						newPixel.g = di((double)(pixel.g * (1 - alpha) + addPixel.g * alpha));
						newPixel.b = di((double)(pixel.b * (1 - alpha) + addPixel.b * alpha));
					}
					else
						if (this->mode == DRAW_ALPHA_ADD)
						{
							newPixel.r = di(min(255, (double)(pixel.r * (1 - alpha) + addPixel.r * alpha)));
							newPixel.g = di(min(255, (double)(pixel.g * (1 - alpha) + addPixel.g * alpha)));
							newPixel.b = di(min(255, (double)(pixel.b * (1 - alpha) + addPixel.b * alpha)));
						}
						else
						{
							newPixel.r = addPixel.r;
							newPixel.g = addPixel.g;
							newPixel.b = addPixel.b;
						}

					this->render->buffer[PixelID] = newPixel;
				}
			}
		}
}
