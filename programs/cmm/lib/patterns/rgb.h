#ifndef INCLUDE_RGB_H
#define INCLUDE_RGB_H

:struct _rgb
{
	byte b,g,r;
	void DwordToRgb();
	void SetRgb();
	dword RgbToDword();
} rgb;

:void _rgb::DwordToRgb(dword _dword)
{
	b = _dword & 0xFF; _dword >>= 8;
	g = _dword & 0xFF; _dword >>= 8;
	r = _dword & 0xFF; _dword >>= 8;
}

:void _rgb::SetRgb(dword _r, _g, _b)
{
	r = _r;
	g = _g;
	b = _b;
}

:dword _rgb::RgbToDword()
{
	/*
	dword _r, _g, _b;
	_r = r << 16;
	_g = g << 8;
	_b = b;
	return _r + _g + _b;
	*/
	EAX = r << 16;
	EAX += g << 8;
	EAX += b;
}

:dword MixColors(dword _base, _overlying, dword a) 
{
	_rgb rgb1, rgb2, rgb_final;
	dword n_a;

	rgb1.DwordToRgb(_base);
	rgb2.DwordToRgb(_overlying);

	n_a = 255 - a;

	rgb_final.b = calc(rgb1.b*a/255) + calc(rgb2.b*n_a/255);
	rgb_final.g = calc(rgb1.g*a/255) + calc(rgb2.g*n_a/255);
	rgb_final.r = calc(rgb1.r*a/255) + calc(rgb2.r*n_a/255);

	return rgb_final.RgbToDword();
}

#endif