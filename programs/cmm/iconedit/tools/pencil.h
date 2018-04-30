
bool PencilTool_Drawing = false;

void PencilTool_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	if (canvas.hovered()) 
	{
		if ((PencilTool_Drawing == true) && (!mouse.key)) {
			PencilTool_Drawing = false;
		}

		if (mouse.key) {
			image.set_pixel(hoverY, hoverX, tool_color);
			DrawCanvasPixel(hoverY, hoverX, tool_color);
			PencilTool_Drawing = true;
		}
		if (mouse.up) {
			DrawPreview();
			actionsHistory.saveCurrentState();	
		}
	}
}

void PencilTool_reset() {
	PencilTool_Drawing = false;
}
