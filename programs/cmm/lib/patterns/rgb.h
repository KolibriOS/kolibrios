#ifndef INCLUDE_RGB_H
#define INCLUDE_RGB_H
#print "[include <rgb.h>]\n"

struct _rgb
{
	byte r,g,b;
	void DwordToRgb();
	dword RgbToDword();
} rgb;

void _rgb::DwordToRgb(dword _dword)
{
	r = _dword & 0xFF; _dword >>= 8;
	g = _dword & 0xFF; _dword >>= 8;
	b = _dword & 0xFF; _dword >>= 8;
}

dword _rgb::RgbToDword()
{
	dword _b, _g;
	_b = b << 16;
	_g = g << 8;
	return _b + _g + r;
}

:dword MixColors(dword _base, _overlying, byte a) 
{
	_rgb rgb1, rgb2, rgb_final;
	byte n_a;

	rgb1.DwordToRgb(_base);
	rgb2.DwordToRgb(_overlying);

	n_a = 255 - a;

	rgb_final.b = calc(rgb1.b*a/255) + calc(rgb2.b*n_a/255);
	rgb_final.g = calc(rgb1.g*a/255) + calc(rgb2.g*n_a/255);
	rgb_final.r = calc(rgb1.r*a/255) + calc(rgb2.r*n_a/255);

	return rgb_final.RgbToDword();
}

#endif