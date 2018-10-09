//===================================================//
//                                                   //
//                    SELECTION                      //
//                                                   //
//===================================================//

enum {
	STATE_INACTIVE=0,
	STATE_CHOSING=1,
	STATE_SELECTED=2
};

struct _selection
{
	_image buf;
	int state;

	int start_x;
	int start_y;
	int end_x;
	int end_y;

	int pivot_x;
	int pivot_y;

	void reset();
	bool is_point_in_selection();
	void copy_to_buf();
	void move_to_point();
	void apply_to_image();
} selection;

void _selection::reset() {	
	start_x = -1;
	start_y = -1;
	end_x = -1;
	end_y = -1;
	pivot_x = -1;
	pivot_y = -1;
	state = STATE_INACTIVE;
}

bool _selection::is_point_in_selection(int x, int y) {
	if (x >= start_x) && (x <= end_x) 
	&& (y >= start_y) && (y <= end_y)
		return true;
	else 
		return false;
}

void _selection::copy_to_buf() {
	dword r, c;

	// Normalize: Restructuring of the selection coordinates
	if (end_x < start_x) {
		start_x >< end_x;
	} 
	if (end_y < start_y) {
		end_y >< start_y;
	}

	state = STATE_SELECTED;
	buf.rows = end_y - start_y + 1;
	buf.columns = end_x - start_x + 1;

	for (r = start_y; r <= end_y; r++) {
		for (c = start_x; c <= end_x; c++) {
			buf.set_pixel(r - start_y, c - start_x, image.get_pixel(r, c) );
		}
	}
}

void _selection::move_to_point(int _new_x, _new_y)
{
	start_x = _new_x;
	start_y = _new_y;
	end_x = buf.columns - 1 + start_x;
	end_y = buf.rows - 1 + start_y;
}

void _selection::apply_to_image() {
	dword r, c;
	for (r = 0; r <= buf.rows - 1; r++) {
		for (c = 0; c <= buf.columns - 1; c++) {
				image.set_pixel(r + start_y, c + start_x, buf.get_pixel(r, c) );
		}
	}
}

//===================================================//
//                                                   //
//                      EVENTS                       //
//                                                   //
//===================================================//

void ApplySelectionToImage() {

	if (STATE_SELECTED != selection.state) return;

	selection.apply_to_image();
	selection.reset();
	
	actionsHistory.saveCurrentState();
	DrawCanvas();
}

bool is_selection_moving() {
	if (STATE_SELECTED == selection.state) return true;
	return false;
}

void SelectTool_activate() {
	ApplySelectionToImage();
	selection.reset();
}

void SelectTool_deactivate() {
	ApplySelectionToImage();
	selection.reset();
}

void SelectTool_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	int dx, dy, m_x, m_y;
	
	m_x = TO_CANVAS_X(mouseX);
	m_y = TO_CANVAS_Y(mouseY);

	if (mouse.down) && (canvas.hovered()) 
	{
		if (selection.start_x != -1) && (selection.is_point_in_selection(m_x, m_y)) {
			if (selection.pivot_x == -1) {
				selection.pivot_x = m_x;
				selection.pivot_y = m_y;
				
				GetKeys();
				if ( (key_modifier&KEY_LSHIFT) || (key_modifier&KEY_RSHIFT) ) {
					DrawBarIcon(selection.start_x, selection.start_y, selection.end_x, 
						selection.end_y, color2, TOIMAGE);
				}

				selection.state = STATE_SELECTED;
			}
		}
		else {	
			ApplySelectionToImage();
			selection.state = STATE_CHOSING;
		}
	}

	//Drag selection
	if (STATE_SELECTED == selection.state)
	{	
		if (selection.pivot_x != -1) {
			dx = m_x - selection.pivot_x;
			dy = m_y - selection.pivot_y;

			if (selection.start_x + dx < 0)
				dx = selection.start_x;
			
			if (selection.end_x + dx >= image.columns)
				dx = image.columns-1 - selection.end_x;
			
			if (selection.start_y + dy < 0)
				dy = selection.start_y;
			
			if (selection.end_y + dy >= image.rows)
				dy = image.rows-1 - selection.end_y;

			selection.move_to_point(selection.start_x + dx, selection.start_y + dy);
			
			selection.pivot_x += dx;
			selection.pivot_y += dy;
			
			DrawCanvas();
		}
		if (mouse.up) {
			if (selection.pivot_x != -1) {
				selection.pivot_x = -1;
				selection.pivot_y = -1;
			}
		}
	}
	
	if (STATE_CHOSING == selection.state)
	{
		mouseX = math.in(mouseX, canvas.x, canvas.x+canvas.w-zoom.value);
		mouseY = math.in(mouseY, canvas.y, canvas.y+canvas.h-zoom.value);

		if (mouse.key) {
			selection.end_x = TO_CANVAS_X(mouseX);
			selection.end_y = TO_CANVAS_Y(mouseY);

			if ((selection.start_x < 0) || (selection.start_y < 0)) {
				selection.start_x = TO_CANVAS_X(mouseX);
				selection.start_y = TO_CANVAS_Y(mouseY);
			}
			else {
				DrawCanvas();
			}
		}
		
		if (mouse.up) {
				selection.copy_to_buf();
		}
	}
/*
	//forbid to select a single pixel
	if (mouse.up) {
		if (! selection.end_x-selection.start_x) 
		&& (! selection.end_y-selection.start_y) {
			selection.reset();
		}
	}
*/
}

void SelectTool_onKeyEvent(dword keycode) {
	dword r, c;

	if (SCAN_CODE_DEL == keycode) {
		DrawBarIcon(selection.start_x, selection.start_y, selection.end_x, selection.end_y, color2, TOIMAGE);
		selection.reset();
		DrawCanvas();
	}

	if (SCAN_CODE_ESC == keycode) {
		ApplySelectionToImage();
		DrawCanvas();
	}

	if (SCAN_CODE_KEY_V == keycode) {
		if (STATE_SELECTED == selection.state) 
		{
			selection.move_to_point(0, 0);	
			DrawCanvas();
		}
	}
}

void SelectTool_onCanvasDraw() 
{	
	#define SELECTION_COLOR 0xAAE5EF
	int p1x, p1y, p2x, p2y, r, c, old_color, new_color;

	//if ((selection.start_x >= 0) && (selection.start_y >= 0) 
	//&& (selection.end_x >= 0) && (selection.end_y >= 0)) {

	if (STATE_SELECTED == selection.state) 
	|| (STATE_CHOSING == selection.state) 
	{
		p1x = math.min(selection.start_x, selection.end_x);
		p2x = math.max(selection.start_x, selection.end_x);

		p1y = math.min(selection.start_y, selection.end_y);
		p2y = math.max(selection.start_y, selection.end_y);

		for (r = p1y; r <= p2y; r++) {
			for (c = p1x; c <= p2x; c++) {
				image.pixel_state.set_drawable_state(r, c, false);
				
				if (STATE_SELECTED == selection.state) && (selection.is_point_in_selection(c, r)) {
					old_color = selection.buf.get_pixel(r - selection.start_y, c - selection.start_x);
				}
				else {
					old_color = image.get_pixel(r, c);
				}
				
				new_color = MixColors(old_color, SELECTION_COLOR, 64);

				DrawCanvasPixel(r, c, new_color);
			}
		}
	}	
}


