// Line tool
struct SimpleFigureTool_State {
	int startX, startY;
	int lastTempPosX, lastTempPosY;
};

enum {
	TOOL_LINE_STATE,
	TOOL_RECT_STATE
};

SimpleFigureTool_State figTool;

void SimpleFigureTool_Reset() {
	figTool.startX = -1;
	figTool.startY = -1;
	figTool.lastTempPosX = -1;
	figTool.lastTempPosY = -1;
}

int mouseX_last;
int mouseY_last;
bool first_click_in_canvas = false;

void SimpleFigureTool_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	int x1, y1, x2, y2;

	if (mouse.down) && (canvas.hovered()) first_click_in_canvas = true;
	if (first_click_in_canvas)
	{
		if (mouseX>canvas.x+canvas.w-zoom.value) mouseX = canvas.x+canvas.w-zoom.value;
		if (mouseY>canvas.y+canvas.h-zoom.value) mouseY = canvas.y+canvas.h-zoom.value;
		if (mouseX<canvas.x) mouseX = canvas.x;
		if (mouseY<canvas.y) mouseY = canvas.y;

		if (mouse.key) {
			if ((figTool.startX < 0) 
			|| (figTool.startY < 0)) {
				figTool.startX = mouseX;
				figTool.startY = mouseY;
			}
			else {
				if ((calc(mouseX - canvas.x/zoom.value) != figTool.lastTempPosX)
					|| (calc(mouseY - canvas.y/zoom.value) != figTool.lastTempPosY)) 
				{
					DrawCanvas();
				}
			}
			mouseX_last = mouseX;
			mouseY_last = mouseY;
		}
		if (mouse.up) {
			if ((figTool.startX >= 0) 
			&& (figTool.startY >= 0)) {

				x1 = TO_CANVAS_X(figTool.startX);
				y1 = TO_CANVAS_Y(figTool.startY);
				x2 = TO_CANVAS_X(mouseX);
				y2 = TO_CANVAS_Y(mouseY);

				// Draw line from start position to current position
				if (currentTool == TOOL_LINE) {
					DrawLineIcon(x1, y1, x2, y2, tool_color, TOIMAGE);
				}
				else if (currentTool == TOOL_RECT) {
					DrawRectangleIcon(x1, y1, x2, y2, tool_color, TOIMAGE);
				}
				else if (currentTool == TOOL_BAR) {
					DrawBarIcon(x1, y1, x2, y2, tool_color, TOIMAGE);
				}

				DrawCanvas();

				actionsHistory.saveCurrentState();

				// Reset start position
				figTool.startX = -1;
				figTool.startY = -1;

				first_click_in_canvas = false;
			}
		}
	}
}

void SimpleFigureTool_onCanvasDraw() {
	int x1, y1, x2, y2;
 	if ((figTool.startX >= 0) 
	&& (figTool.startY >= 0) && (mouse.key)) {

		x1 = TO_CANVAS_X(figTool.startX);
		y1 = TO_CANVAS_Y(figTool.startY);
		x2 = TO_CANVAS_X(mouseX_last);
		y2 = TO_CANVAS_Y(mouseY_last);

		if (currentTool == TOOL_LINE)
			DrawLineIcon(x1, y1, x2, y2, tool_color, TOCANVAS);
		else if (currentTool == TOOL_RECT)
			DrawRectangleIcon(x1, y1, x2, y2, tool_color, TOCANVAS);
		else if (currentTool == TOOL_BAR)
			DrawBarIcon(x1, y1, x2, y2, tool_color, TOCANVAS);

		figTool.lastTempPosX = mouseX_last - canvas.x/zoom.value;
		figTool.lastTempPosY = mouseY_last - canvas.y/zoom.value;
	}
}


//===================================================//
//                                                   //
//                      DRAWs                        //
//                                                   //
//===================================================//



// target - TOMAGE, TOCANVAS
void DrawLineIcon(int x1, int y1, int x2, int y2, dword color, int target) {
	int dx, dy, signX, signY, error, error2;

	if (TOIMAGE == target) {
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

	DrawCanvasPixel(y2, x2, color);
	image.pixel_state.set_drawable_state(y2, x2, false);

	while((x1 != x2) || (y1 != y2)) 
	{
		DrawCanvasPixel(y1, x1, color);

		image.pixel_state.set_drawable_state(y1, x1, false);
		
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

void DrawRectangleIcon(int x1, int y1, int x2, int y2, dword color, int target) {
	DrawLineIcon(x1, y1, x2, y1, color, target);
	DrawLineIcon(x2, y1, x2, y2, color, target);
	DrawLineIcon(x2, y2, x1, y2, color, target);
	DrawLineIcon(x1, y2, x1, y1, color, target);
}

void DrawBarIcon(int x1, int y1, int x2, int y2, dword color, int target) {
	signed signY;
	if (y1 < y2)
		signY = 1;
	else
		signY = -1;

	while (y1 != y2+signY) {
		DrawLineIcon(x1, y1, x2, y1, color, target);
		y1 += signY;
	}
}