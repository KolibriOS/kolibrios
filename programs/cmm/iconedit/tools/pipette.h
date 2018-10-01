
void PipetteTool_activate() {
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE);
}

void PipetteTool_onKeyEvent() {
	if (key_scancode == SCAN_CODE_ESC) setCurrentTool(previousTool);
}

void PipetteTool_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	//if (!canvas.hovered()) return; //TODO: option "Restrict pipette to canvas area"
	tool_color = GetPixelUnderMouse();
	DrawBar(Form.cwidth-30, 5, 20, 20, tool_color);
	
	if (mouse.down) {
		DrawBar(Form.cwidth-30, 5, 20, 20, system.color.work);
		SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
		if (mouse.key&MOUSE_LEFT) EventSetActiveColor(1, tool_color);
		if (mouse.key&MOUSE_RIGHT) EventSetActiveColor(2, tool_color);
		
		setCurrentTool(previousTool);
	}
}
