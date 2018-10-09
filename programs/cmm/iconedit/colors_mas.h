#define MAX_CELL_SIZE 256

//////////////////////////////////////////////////////////////////////////////////////
//                                                                                  //
//                                  DRAW PIXEL                                      //
//                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////

//The 'draw[]' is the array which holds the states should we draw a pixel or not.
//Is need to decrease redraw when using some tools like line, rectangle and selection.

struct _pixel_state
{
	unsigned image_rows, image_columns;
	bool draw[MAX_CELL_SIZE*MAX_CELL_SIZE];
	void set_drawable_state();
	bool is_drawable();
	void reset_and_set_all_drawable();
	void set_sizes();
} pixel_state;

void _pixel_state::set_drawable_state(int _r, _c, _state)
{
	draw[image_columns*_r + _c] = _state;
}

bool _pixel_state::is_drawable(int _r, _c)
{
	return draw[image_columns*_r + _c];
}

void _pixel_state::reset_and_set_all_drawable()
{
	int i;
	for (i = 0; i < image_columns*image_rows; i++) draw[i]=true;
}

void _pixel_state::set_sizes(dword _r, _c)
{
	image_rows = _r;
	image_columns = _c;
}

//////////////////////////////////////////////////////////////////////////////////////
//                                                                                  //
//                                      IMAGE                                       //
//                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////

//This stucture determines basic actions that can be done with image (icon).

struct _image
{
	unsigned rows, columns;
	dword mas[MAX_CELL_SIZE*MAX_CELL_SIZE];
	dword mas_copy[MAX_CELL_SIZE*MAX_CELL_SIZE];
	dword img;
	_pixel_state pixel_state;
	void create();
	void set_pixel();
	void draw_line();
	void fill();
	void set_image();
	dword get_pixel();
	dword get_image();
	dword get_image_with_replaced_color();
	void move();
};

void _image::create(int _rows, _columns)
{
	int i;
	rows = _rows;
	columns = _columns;
	for (i = 0; i < columns*rows; i++) mas[i]=0xBFCAD2;
	pixel_state.set_sizes(rows, columns);
}

void _image::set_pixel(int _r, _c, _color)
{
	mas[columns*_r + _c] = _color;
}

void _image::draw_line(int x1, int y1, int x2, int y2, dword color) {
	int dx, dy, signX, signY, error, error2;

	dx = x2 - x1;

	if (dx < 0)
		dx = -dx;
   
	dy = y2 - y1;

	if (dy < 0)
		dy = -dy;
   
	if (x1 < x2)
		signX = 1;
	else
		signX = -1;
   
	if (y1 < y2)
		signY = 1;
	else
		signY = -1;
   
	error = dx - dy;

	set_pixel(y2, x2, color);

	while((x1 != x2) || (y1 != y2)) 
	{
		set_pixel(y1, x1, color);
		
		error2 = error * 2;

		if(error2 > calc(-dy)) 
		{
			error -= dy;
			x1 += signX;
		}

		if(error2 < dx) 
		{
			error += dx;
			y1 += signY;
		}
	}
}

void _image::fill(int _r, _c, _color)
{
	#define MARKED 6
	int r, c, i, restart;

	dword old_color = get_pixel(_r, _c);
	set_pixel(_r, _c, MARKED);

	do {
		restart=false;	
		for (r = 0; r < rows; r++)
			for (c = 0; c < columns; c++)
			{
				IF (get_pixel(r,c) != old_color) continue;
				IF (get_pixel(r,c) == MARKED) continue;
				
				IF (c>0)               && (get_pixel(r,c-1) == MARKED) set_pixel(r,c,MARKED);
				IF (r>0)               && (get_pixel(r-1,c) == MARKED) set_pixel(r,c,MARKED);
				IF (c<columns-1) && (get_pixel(r,c+1) == MARKED) set_pixel(r,c,MARKED);
				IF (r<rows-1)    && (get_pixel(r+1,c) == MARKED) set_pixel(r,c,MARKED);
				
				IF (get_pixel(r,c)==MARKED) restart=true;
			}
	}while(restart);

	for (i=0; i<columns*rows; i++) 
			IF (mas[i]==MARKED) mas[i] = _color;
}

dword _image::get_pixel(int _r, _c)
{
	return mas[columns*_r + _c];
}

void _image::set_image(dword _inbuf)
{
	dword i;
	for (i = 0; i < columns*rows; i++;) 
	{
		// mas[i] = ESDWORD[i*4+_inbuf] & 0x00FFFFFF; //for x32 bit color
		mas[i] = ESDWORD[i*3+_inbuf] & 0xFFFFFF;
	}
}

dword _image::get_image()
{
	int r=0, c=0;
	dword i;

	free(img);
	i = img = malloc(rows*columns*3);

	for (r = 0; r < rows; r++)
		for (c = 0; c < columns; c++)
		{
			rgb.DwordToRgb(get_pixel(r,c));
			ESBYTE[i] = rgb.b;
			ESBYTE[i+1] = rgb.g;
			ESBYTE[i+2] = rgb.r;
			i += 3;
		}
	return img;
}

dword _image::get_image_with_replaced_color(dword _col_from, _col_to)
{
	int r=0, c=0;
	dword i;
	dword cur_pixel;

	free(img);
	i = img = malloc(rows*columns*3);

	for (r = 0; r < rows; r++)
		for (c = 0; c < columns; c++)
		{
			cur_pixel = get_pixel(r,c);
			if (cur_pixel == _col_from) cur_pixel = _col_to;
			rgb.DwordToRgb(cur_pixel);
			ESBYTE[i] = rgb.b;
			ESBYTE[i+1] = rgb.g;
			ESBYTE[i+2] = rgb.r;
			i += 3;
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
	ROTATE_LEFT,
	ROTATE_RIGHT
};
void _image::move(int _direction)
{
	int r, c;
	dword first_element_data;

	switch(_direction)
	{
		case MOVE_LEFT:
				for (r = 0; r < rows; r++)
				{
					first_element_data = get_pixel(r, 0);
					for (c = 0; c < columns-1; c++) set_pixel(r, c, get_pixel(r, c+1));
					set_pixel(r, columns-1, first_element_data);
				}
				break;
		case MOVE_RIGHT:
				for (r = 0; r < rows; r++)
				{
					first_element_data = get_pixel(r, columns-1);
					for (c = columns-1; c > 0; c--) set_pixel(r, c, get_pixel(r, c-1));
					set_pixel(r, 0, first_element_data);
				}	
				break;	
		case MOVE_UP:
				for (c = 0; c < columns; c++)
				{
					first_element_data = get_pixel(0, c);
					for (r = 0; r < rows-1; r++) set_pixel(r, c, get_pixel(r+1, c));
					set_pixel(rows-1, c, first_element_data);
				}	
				break;
		case MOVE_DOWN:
				for (c = 0; c < columns; c++)
				{
					first_element_data = get_pixel(rows-1, c);
					for (r = rows-1; r > 0; r--) set_pixel(r, c, get_pixel(r-1, c));
					set_pixel(0, c, first_element_data);
				}
				break;
		case FLIP_HOR:
				for (r = 0; r < rows; r++)
					for (c = 0; c < columns/2; c++) {
						first_element_data = get_pixel(r, c);
						set_pixel(r, c, get_pixel(r, columns-c-1));
						set_pixel(r, columns-c-1, first_element_data);
					}
				break;
		case FLIP_VER:
				for (c = 0; c < columns; c++)
					for (r = 0; r < rows/2; r++) {
						first_element_data = get_pixel(r, c);
						set_pixel(r, c, get_pixel(rows-r-1, c));
						set_pixel(rows-r-1, c, first_element_data);
					}
				break;
		case ROTATE_LEFT:
				//slow but the code is simple
				//need to rewrite in case of big images support
				move(ROTATE_RIGHT);
				move(ROTATE_RIGHT);
				move(ROTATE_RIGHT);
				break;	
		case ROTATE_RIGHT:
				if (columns!=rows) {
					notify("Sorry, rotate is implemented for square canvaces only!");
					break;
				}

				for (r=0; r<MAX_CELL_SIZE*MAX_CELL_SIZE; r++)  {
					mas_copy[r] = mas[r];
				}

				for (c = 0; c < columns; c++)
					for (r = 0; r < rows; r++) {
						set_pixel(c, rows-r-1, mas_copy[columns*r + c]);
					}

				columns >< rows;
				break;
	}
}

/*
1234
5678
90AB

951
0
A
B

*/



