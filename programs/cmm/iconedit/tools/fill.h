
void FillTool_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	if (canvas.hovered()) && (mouse.up)
	{
		image.fill(mouseY-canvas.y/zoom.value, 
				mouseX-canvas.x/zoom.value, tool_color);
		actionsHistory.saveCurrentState();			
		DrawCanvas();
	}
}
