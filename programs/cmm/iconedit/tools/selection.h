//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

_image selection;

enum {
	STATE_INACTIVE=0,
	STATE_CHOSING=1,
	STATE_SELECTED=2
};
int selection_state = STATE_INACTIVE;

int selection_start_x = -1;
int selection_start_y = -1;
int selection_end_x = -1;
int selection_end_y = -1;

int selection_pivot_x = -1;
int selection_pivot_y = -1;

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void SelectTool_normalizeSelection() {
	// Restructuring of the selection coordinates
	if (selection_end_x < selection_start_x) {
		selection_start_x >< selection_end_x;
	} 
	if (selection_end_y < selection_start_y) {
		selection_end_y >< selection_start_y;
	}
}

void SelectTool_drawBufferToImage(int insert_x, int insert_y) {
	dword r, c;
	dword insert_to_x, insert_to_y;
	
		insert_to_x = math.min(insert_x + selection.columns - 1, image.columns-1);
		insert_to_y = math.min(insert_y + selection.rows - 1, image.rows-1);

		for (r = insert_y; r <= insert_to_y; r++) {
			for (c = insert_x; c <= insert_to_x; c++) {
					image.set_pixel(r, c, selection.get_pixel(r - insert_y, c - insert_x) );
			}
		}	
}

void ApplySelectionToImage() {
	if (STATE_SELECTED != selection_state) return;

	SelectTool_drawBufferToImage(selection_start_x, selection_start_y);

	selection_pivot_x = -1;
	selection_pivot_y = -1;
	
	actionsHistory.saveCurrentState();
	DrawCanvas();
}

bool is_selection_moving() {
	if (STATE_SELECTED == selection_state) return true;
	return false;
}

void reset_selection() {
	ApplySelectionToImage();
	
	selection_start_x = -1;
	selection_start_y = -1;
	selection_end_x = -1;
	selection_end_y = -1;	
}

void SelectTool_activate() {
	reset_selection();
	selection_state = STATE_INACTIVE;
}

void SelectTool_deactivate() {
	ApplySelectionToImage();
	selection_state = STATE_INACTIVE;
}

bool SelectTool_pointInSelection(int x, int y) {
	if (x >= selection_start_x) && (x <= selection_end_x) && (y >= selection_start_y) && (y <= selection_end_y)
		return true;
	else 
		return false;
}


void SelectTool_copyToBuffer() {
	dword r, c;

	selection_state = STATE_SELECTED;
	selection.rows = selection_end_y - selection_start_y + 1;
	selection.columns = selection_end_x - selection_start_x + 1;

	for (r = selection_start_y; r <= selection_end_y; r++) {
		for (c = selection_start_x; c <= selection_end_x; c++) {
			selection.set_pixel(r - selection_start_y, c - selection_start_x, image.get_pixel(r, c) );
		}
	}
}

void SelectTool_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	int dx, dy, m_x, m_y;
	
	m_x = TO_CANVAS_X(mouseX);
	m_y = TO_CANVAS_Y(mouseY);

	if (mouse.down) && (canvas.hovered()) 
	{
		if (selection_start_x != -1) && (SelectTool_pointInSelection(m_x, m_y)) {
			if (selection_pivot_x == -1) {
				selection_pivot_x = m_x;
				selection_pivot_y = m_y;
				
				GetKeys();
				if ( (key_modifier&KEY_LSHIFT) || (key_modifier&KEY_RSHIFT) ) {
					DrawBarIcon(selection_start_x, selection_start_y, selection_end_x, 
						selection_end_y, color2, TOIMAGE);
				}

				selection_state = STATE_SELECTED;
			}
		}
		else {	
			reset_selection();
			selection_state = STATE_CHOSING;
		}
	}

	if (selection_pivot_x != -1) {
		dx = m_x - selection_pivot_x;
		dy = m_y - selection_pivot_y;

		if (selection_start_x + dx < 0)
			dx = selection_start_x;
		
		if (selection_end_x + dx >= image.columns)
			dx = image.columns-1 - selection_end_x;
		
		if (selection_start_y + dy < 0)
			dy = selection_start_y;
		
		if (selection_end_y + dy >= image.rows)
			dy = image.rows-1 - selection_end_y;
		
		selection_start_x += dx;
		selection_end_x += dx;
		
		selection_start_y += dy;
		selection_end_y += dy;
		
		selection_pivot_x += dx;
		selection_pivot_y += dy;
		
		DrawCanvas();
	}
	
	if (STATE_CHOSING == selection_state)
	{
		mouseX = math.in(mouseX, canvas.x, canvas.x+canvas.w-zoom.value);
		mouseY = math.in(mouseY, canvas.y, canvas.y+canvas.h-zoom.value);

		if (mouse.key) {
			selection_end_x = TO_CANVAS_X(mouseX);
			selection_end_y = TO_CANVAS_Y(mouseY);

			if ((selection_start_x < 0) || (selection_start_y < 0)) {
				selection_start_x = TO_CANVAS_X(mouseX);
				selection_start_y = TO_CANVAS_Y(mouseY);
			}
			else {
				DrawCanvas();
			}

		}
		
		if (mouse.up) {			
			SelectTool_normalizeSelection();
			SelectTool_copyToBuffer();
		}
	}
	
	if (mouse.up) {
		if (selection_pivot_x != -1) {
			selection_pivot_x = -1;
			selection_pivot_y = -1;
		}
	}
}

void SelectTool_onKeyEvent(dword keycode) {
	dword r, c;

	if (SCAN_CODE_DEL == keycode) {
		selection_start_x = -1;
		selection_start_y = -1;
		selection_end_x = -1;
		selection_end_y = -1;
		selection_state = STATE_INACTIVE;
		DrawCanvas();
	}

	if (SCAN_CODE_ESC == keycode) {
		reset_selection();
		DrawCanvas();
	}

	if (SCAN_CODE_KEY_V == keycode) {
		if (STATE_SELECTED == selection_state) {

			selection_start_x = 0;
			selection_start_y = 0;
			selection_end_x = selection.columns - 1;			
			selection_end_y = selection.rows - 1;
			
			DrawCanvas();
		}
	}
}

void SelectTool_onCanvasDraw() 
{	
	#define SELECTION_COLOR 0xAAE5EF
	int p1x, p1y, p2x, p2y, r, c, old_color, new_color;

	if ((selection_start_x >= 0) && (selection_start_y >= 0) && (selection_end_x >= 0) && (selection_end_y >= 0)) {

		p1x = math.min(selection_start_x, selection_end_x);
		p2x = math.max(selection_start_x, selection_end_x);

		p1y = math.min(selection_start_y, selection_end_y);
		p2y = math.max(selection_start_y, selection_end_y);

		for (r = p1y; r <= p2y; r++) {
			for (c = p1x; c <= p2x; c++) {
				image.pixel_state.set_drawable_state(r, c, false);
				
				if (STATE_SELECTED == selection_state) && (SelectTool_pointInSelection(c, r)) {
					old_color = selection.get_pixel(r - selection_start_y, c - selection_start_x);
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


