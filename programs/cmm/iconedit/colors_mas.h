struct _colors
{

	unsigned rows, columns;
	dword mas[32*32];
	dword img;
	void set_pixel();
	dword get_pixel();
	dword get_image();
	void move();
};

void _colors::set_pixel(int _r, _c, _color)
{
	mas[columns*_r + _c] = _color;
}

dword _colors::get_pixel(int _r, _c)
{
	return mas[columns*_r + _c];
}

dword _colors::get_image()
{
	int r=0, c=0;
	dword i;

	free(img);
	i = img = malloc(rows*columns*3);

	for (r = 0; r < rows; r++)
	{
		for (c = 0; c < columns; c++)
		{
			rgb.DwordToRgb(get_pixel(r,c));
			ESBYTE[i] = rgb.b;
			ESBYTE[i+1] = rgb.g;
			ESBYTE[i+2] = rgb.r;
			i += 3;
		}
	}
	return img;
}

enum {
	MOVE_LEFT,
	MOVE_RIGHT,
	MOVE_UP,
	MOVE_DOWN,
	FLIP_VER,
	FLIP_HOR,
	ROTE
};
void _colors::move(int _direction)
{
	int r, c;
	dword first_element_data;

	if (_direction == MOVE_LEFT)
	{
		for (r = 0; r < rows; r++)
		{
			first_element_data = get_pixel(r, 0);
			for (c = 0; c < columns-1; c++) set_pixel(r, c, get_pixel(r, c+1));
			set_pixel(r, columns-1, first_element_data);
		}		
	}
	if (_direction == MOVE_RIGHT)
	{
		for (r = 0; r < rows; r++)
		{
			first_element_data = get_pixel(r, columns-1);
			for (c = columns-1; c > 0; c--) set_pixel(r, c, get_pixel(r, c-1));
			set_pixel(r, 0, first_element_data);
		}		
	}
	if (_direction == MOVE_UP)
	{
		for (c = 0; c < columns; c++)
		{
			first_element_data = get_pixel(0, c);
			for (r = 0; r < rows-1; r++) set_pixel(r, c, get_pixel(r+1, c));
			set_pixel(rows-1, c, first_element_data);
		}		
	}
	if (_direction == MOVE_DOWN)
	{
		for (c = 0; c < columns; c++)
		{
			first_element_data = get_pixel(rows-1, c);
			for (r = rows-1; r > 0; r--) set_pixel(r, c, get_pixel(r-1, c));
			set_pixel(0, c, first_element_data);
		}		
	}


	if (_direction == FLIP_HOR)
	{
		for (r = 0; r < rows; r++)
		{
			for (c = 0; c < columns/2; c++) {
				first_element_data = get_pixel(r, c);
				set_pixel(r, c, get_pixel(r, columns-c));
				set_pixel(r, columns-c, first_element_data);
			}
		}		
	}
}







