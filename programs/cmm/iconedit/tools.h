enum {
	TOOL_NONE = -1,
	TOOL_PENCIL,
	TOOL_PIPETTE,
	TOOL_FILL,
	TOOL_LINE,
	TOOL_RECT,
	TOOL_SELECT
};

struct Tool {
	int id;
	
	void (*activate)();
	void (*deactivate)();
	void (*onMouseEvent)(int x, int y, int lkm, int pkm);
	void (*onKeyEvent)(dword keycode);
	void (*onCanvasDraw)();
};

int currentTool = -1;
Tool tools[6];


void resetCurrentTool() {
	if ((currentTool != TOOL_NONE) && (tools[currentTool].deactivate != 0)) {
		tools[currentTool].deactivate();
	}
	
	currentTool = TOOL_NONE;
}

void setCurrentTool(int index) {
	resetCurrentTool();

	currentTool = index;
	
	if ((index != TOOL_NONE) && (tools[index].activate != 0))
		tools[index].activate();

	DrawLeftPanel();
	DrawCanvas();
}

//===================================================//
//                                                   //
//                    FUNTIONS                       //
//                                                   //
//===================================================//

void FillTool_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	if (canvas.hovered()) && (currentTool==TOOL_FILL) && (mouse.up)
	{
		image.fill(mouseY-canvas.y/zoom.value, 
				mouseX-canvas.x/zoom.value, tool_color);
		actionsHistory.saveCurrentState();			
		DrawCanvas();
	}
}

void PipetteTool_activate() {
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE);
}

void PipetteTool_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	tool_color = GetPixelUnderMouse();
	DrawBar(Form.cwidth-30, 5, 20, 20, tool_color);
	
	if (mouse.down) {
		DrawBar(Form.cwidth-30, 5, 20, 20, system.color.work);
		SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
		if (mouse.key&MOUSE_LEFT) EventSetActiveColor(1, tool_color);
		if (mouse.key&MOUSE_RIGHT) EventSetActiveColor(2, tool_color);
		
		setCurrentTool(TOOL_PENCIL);
	}
}

bool PencilTool_Drawing = false;

void PencilTool_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	if (canvas.hovered()) 
	{
		if ((PencilTool_Drawing == true) && (!mouse.key)) {
			actionsHistory.saveCurrentState();
			PencilTool_Drawing = false;
		}

		if (mouse.key) {
			image.set_pixel(mouseY-canvas.y/zoom.value, 
				mouseX-canvas.x/zoom.value, tool_color);
			PencilTool_Drawing = true;
		}
		DrawCanvas();
	}
}

void PencilTool_reset() {
	PencilTool_Drawing = false;
}

// Line tool
struct SimpleFigureTool_State {
	int startX, startY;
	int lastTempPosX, lastTempPosY;
};

enum {
	TOOL_LINE_STATE,
	TOOL_RECT_STATE
};

dword currentFigToolState = -1;
SimpleFigureTool_State figTool_States[2];

void SimpleFigureTool_Reset() {
	if (currentTool == TOOL_LINE)
		currentFigToolState = TOOL_LINE_STATE;
	else if (currentTool == TOOL_RECT)
		currentFigToolState = TOOL_RECT_STATE;

	figTool_States[currentFigToolState].startX = -1;
	figTool_States[currentFigToolState].startY = -1;
	figTool_States[currentFigToolState].lastTempPosX = -1;
	figTool_States[currentFigToolState].lastTempPosY = -1;
}

int mouseX_last;
int mouseY_last;
bool first_click_in_canvas = false;

void SimpleFigureTool_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	if (mouse.down) && (canvas.hovered()) first_click_in_canvas = true;
	if (first_click_in_canvas)
	{
		if (mouseX>canvas.x+canvas.w-zoom.value) mouseX = canvas.x+canvas.w-zoom.value;
		if (mouseY>canvas.y+canvas.h-zoom.value) mouseY = canvas.y+canvas.h-zoom.value;
		if (mouseX<canvas.x) mouseX = canvas.x;
		if (mouseY<canvas.y) mouseY = canvas.y;

		if (mouse.key) {
			if ((figTool_States[currentFigToolState].startX < 0) 
			|| (figTool_States[currentFigToolState].startY < 0)) {
				figTool_States[currentFigToolState].startX = mouseX;
				figTool_States[currentFigToolState].startY = mouseY;
			}
			else {
				if ((calc(mouseX - canvas.x/zoom.value) != figTool_States[currentFigToolState].lastTempPosX)
					|| (calc(mouseY - canvas.y/zoom.value) != figTool_States[currentFigToolState].lastTempPosY)) 
				{
					DrawCanvas();
				}
			}
			mouseX_last = mouseX;
			mouseY_last = mouseY;
		}
		if (mouse.up) {
			if ((figTool_States[currentFigToolState].startX >= 0) 
			&& (figTool_States[currentFigToolState].startY >= 0)) {
				// Draw line from start position to current position
				if (currentTool == TOOL_LINE) {
					DrawLine(figTool_States[currentFigToolState].startX - canvas.x/zoom.value, 
						figTool_States[currentFigToolState].startY - canvas.y/zoom.value, 
						mouseX - canvas.x/zoom.value, 
						mouseY - canvas.y/zoom.value, 
						tool_color, 
						1);
				}
				else if (currentTool == TOOL_RECT) {
					DrawRectangleInCanvas(figTool_States[currentFigToolState].startX - canvas.x/zoom.value, 
						figTool_States[currentFigToolState].startY - canvas.y/zoom.value, 
						mouseX - canvas.x/zoom.value, 
						mouseY - canvas.y/zoom.value, tool_color, 1);
				}

				DrawCanvas();

				actionsHistory.saveCurrentState();

				// Reset start position
				figTool_States[currentFigToolState].startX = -1;
				figTool_States[currentFigToolState].startY = -1;

				first_click_in_canvas = false;
			}
		}
	}
}

void SimpleFigureTool_onCanvasDraw() {
	if ((figTool_States[currentFigToolState].startX >= 0) 
	&& (figTool_States[currentFigToolState].startY >= 0) && (mouse.key)) {
		if (currentTool == TOOL_LINE) {
			DrawLine(figTool_States[currentFigToolState].startX - canvas.x/zoom.value, 
				figTool_States[currentFigToolState].startY - canvas.y/zoom.value, 
				mouseX_last - canvas.x/zoom.value, 
				mouseY_last - canvas.y/zoom.value, 
				tool_color, 
				2);
		}
		else if (currentTool == TOOL_RECT) {
			DrawRectangleInCanvas(figTool_States[currentFigToolState].startX - canvas.x/zoom.value, 
				figTool_States[currentFigToolState].startY - canvas.y/zoom.value, 
				mouseX_last - canvas.x/zoom.value, 
				mouseY_last - canvas.y/zoom.value, 
				tool_color, 
				2);
		}

		figTool_States[currentFigToolState].lastTempPosX = mouseX_last - canvas.x/zoom.value;
		figTool_States[currentFigToolState].lastTempPosY = mouseY_last - canvas.y/zoom.value;
	}
}

// Selection
int selection_start_x = -1;
int selection_start_y = -1;
int selection_end_x = -1;
int selection_end_y = -1;
bool selection_active = false;

dword SelectionTool_buffer = 0;
dword SelectionTool_buffer_r = 0;
dword SelectionTool_buffer_c = 0;

bool selection_moving_started = false;
int selection_pivot_x = -1;
int selection_pivot_y = -1;

void SelectTool_normalizeSelection() {
	int t;

	// Restructuring of the selection coordinates
	if (selection_end_x < selection_start_x) {
		t = selection_start_x;
		selection_start_x = selection_end_x;
		selection_end_x = t;
	} 

	if (selection_end_y < selection_start_y) {
		t = selection_end_y;
		selection_end_y = selection_start_y;
		selection_start_y = t;
	}
}

void reset_selection_moving() {
	if (selection_moving_started) {
		SelectTool_drawBuffer(selection_start_x, selection_start_y, 1);

		selection_pivot_x = -1;
		selection_pivot_y = -1;
		
		selection_moving_started = false;
		
		actionsHistory.saveCurrentState();
		DrawCanvas();
	}
}

bool is_selection_moving() {
	return selection_moving_started;
}

void reset_selection() {
	reset_selection_moving();
	
	selection_start_x = -1;
	selection_start_y = -1;
	selection_end_x = -1;
	selection_end_y = -1;	
}

void SelectTool_activate() {
	reset_selection();

	selection_active = false;
}

void SelectTool_deactivate() {
	reset_selection_moving();
}

bool SelectTool_pointInSelection(int x, int y) {
	if (x >= selection_start_x) && (x <= selection_end_x) && (y >= selection_start_y) && (y <= selection_end_y)
		return true;
	else 
		return false;
}


void SelectTool_copyToBuffer() {
	dword offset, r, c;

		if (SelectionTool_buffer != 0)
			free(SelectionTool_buffer);

		SelectionTool_buffer_r = selection_end_y - selection_start_y + 1;
		SelectionTool_buffer_c = selection_end_x - selection_start_x + 1;
		SelectionTool_buffer = malloc(SelectionTool_buffer_r * SelectionTool_buffer_c * 4);

		for (r = selection_start_y; r <= selection_end_y; r++) {
			for (c = selection_start_x; c <= selection_end_x; c++) {
				offset = calc(SelectionTool_buffer_c * calc(r - selection_start_y) + calc(c - selection_start_x)) * 4;

				ESDWORD[SelectionTool_buffer + offset] = image.get_pixel(r, c);
			}
		}
}

void SelectTool_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	int click_x, click_y, dx, dy, m_x, m_y, r, c, color;
	dword pixel;
	
	m_x = TO_CANVAS_X(mouseX);
	m_y = TO_CANVAS_Y(mouseY);
	
	if (mouse.down) && (canvas.hovered()) && (!selection_active) {
		if (selection_start_x != -1) && (SelectTool_pointInSelection(m_x, m_y)) {
			if (selection_pivot_x == -1) {
				selection_pivot_x = m_x;
				selection_pivot_y = m_y;
				
				GetKeys();
				
				if (!selection_moving_started) && ( !(key_modifier&KEY_LSHIFT) ) {
					for (r = selection_start_y; r <= selection_end_y; r++)
						for (c = selection_start_x; c <= selection_end_x; c++) {
							image.set_pixel(r, c, color2);
						}
				}
				
				selection_moving_started = true;
			}
		}
		else {
		
			reset_selection();
			selection_active = true;
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
	
	if (selection_active)
	{
		if (mouseX>canvas.x+canvas.w-zoom.value) mouseX = canvas.x+canvas.w-zoom.value;
		if (mouseY>canvas.y+canvas.h-zoom.value) mouseY = canvas.y+canvas.h-zoom.value;

		if (mouseX<canvas.x) mouseX = canvas.x;
		if (mouseY<canvas.y) mouseY = canvas.y;

		if (mouse.key) {
			selection_end_x = TO_CANVAS_X(mouseX);
			selection_end_y = TO_CANVAS_Y(mouseY);

			if ((selection_start_x < 0) || (selection_start_y < 0)) {
				selection_start_x = TO_CANVAS_X(mouseX);
				selection_start_y = TO_CANVAS_Y(mouseY);
			}
			else {
				DrawCanvas();

				/**if ((calc(TO_CANVAS_X(mouseX)) != selection_end_x)
					|| (calc(TO_CANVAS_Y(mouseY)) != selection_end_y)) 
				{
					DrawCanvas();
				}*/
			}

		}
		
		if (mouse.up) {			
			selection_active = false;
			
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

void SelectTool_onCanvasDraw() {	
	if (selection_moving_started)
		SelectTool_drawBuffer(selection_start_x, selection_start_y, 2);

	if ((selection_start_x >= 0) && (selection_start_y >= 0) && (selection_end_x >= 0) && (selection_end_y >= 0)) {
		DrawSelection(selection_start_x, selection_start_y, selection_end_x, selection_end_y);
	}	
}

void SelectTool_drawBuffer(int insert_x, int insert_y, int target) {
	dword color;
	dword offset, r, c;
	dword insert_to_x, insert_to_y;
	
	if (SelectionTool_buffer != 0) {
		insert_to_x = insert_x + SelectionTool_buffer_c - 1;
			
		if (insert_to_x >= image.columns)
			insert_to_x = image.columns-1;

		insert_to_y = insert_y + SelectionTool_buffer_r - 1;
			
		if (insert_to_y >= image.rows)
			insert_to_y = image.rows-1;

		for (r = insert_y; r <= insert_to_y; r++) {
			for (c = insert_x; c <= insert_to_x; c++) {
					offset = calc(SelectionTool_buffer_c * calc(r - insert_y) + calc(c - insert_x)) * 4;

					color = ESDWORD[SelectionTool_buffer + offset];
					
					if (target == 1)
						image.set_pixel(r, c, color);
					else
						DrawBar(c*zoom.value + canvas.x, r*zoom.value + canvas.y, 
							zoom.value, zoom.value, color);
			}
		}	
	}	
}

void SelectTool_onKeyEvent(dword keycode) {
	dword offset, r, c;
	dword insert_x, insert_y, insert_to_x, insert_to_y;

	if (keycode == SCAN_CODE_KEY_V) {
		if (SelectionTool_buffer != 0) {
			reset_selection();
			
			selection_moving_started = true;
			selection_start_x = 0;
			selection_end_x = SelectionTool_buffer_c - 1;
			
			selection_start_y = 0;
			selection_end_y = SelectionTool_buffer_r - 1;
			
			DrawCanvas();
	
		}
	}
}


//===================================================//
//                                                   //
//                      DRAWs                        //
//                                                   //
//===================================================//



// target - image (1) or canvas (2)
void DrawLine(int x1, int y1, int x2, int y2, dword color, int target) {
	int dx, dy, signX, signY, error, error2;

	if (1==target) {
		image.draw_line(x1, y1, x2, y2, color);
		return;
	}
	
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

	DrawBar(x2*zoom.value + canvas.x, y2*zoom.value + canvas.y, 
			zoom.value, zoom.value, color);

	while((x1 != x2) || (y1 != y2)) 
	{
		DrawBar(x1*zoom.value + canvas.x, y1*zoom.value + canvas.y, 
			zoom.value, zoom.value, color);
		
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

void DrawRectangleInCanvas(int x1, int y1, int x2, int y2, dword color, int target) {
	DrawLine(x1, y1, x2, y1, color, target);
	DrawLine(x2, y1, x2, y2, color, target);
	DrawLine(x2, y2, x1, y2, color, target);
	DrawLine(x1, y2, x1, y1, color, target);
}

#define SELECTION_COLOR 0xAAE5EF

void DrawSelection(int x1, int y1, int x2, int y2) {
	int p1x, p1y, p2x, p2y, r, c, old_color, new_color;
	dword offset;

	if (x1 <= x2) {
		p1x = x1;
		p2x = x2;
	}
	else {
		p1x = x2;
		p2x = x1;
	}

	if (y1 <= y2) {
		p2y = y1;
		p1y = y2;
	}
	else {
		p2y = y2;
		p1y = y1;
	}

	for (r = p1y; r >= p2y; r--) {
		for (c = p1x; c <= p2x; c++) {
			
			if (selection_moving_started) && (SelectTool_pointInSelection(c, r)) {
				offset = calc(SelectionTool_buffer_c * calc(r - selection_start_y) + calc(c - selection_start_x)) * 4;
				old_color = ESDWORD[SelectionTool_buffer + offset];
			}
			else {
				old_color = image.get_pixel(r, c);
			}
			
			new_color = MixColors(old_color, SELECTION_COLOR, 64);
			
			DrawBar(c*zoom.value + canvas.x, r*zoom.value + canvas.y, 
				zoom.value, zoom.value, new_color);

		}
	}
}