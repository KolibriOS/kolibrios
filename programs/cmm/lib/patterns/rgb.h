
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
