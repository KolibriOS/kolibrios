dword screen_copy;

void ScreenCopy_activate() {
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE);
	screen_copy = malloc(image.columns * image.rows * 3 );
}

void ScreenCopy_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	dword i;
	CopyScreen(screen_copy, mouse.x + Form.left + 5, mouse.y + Form.top + skin_height, image.columns, image.rows);
	for (i = 0; i < image.columns*image.rows; i++;) 
	{
		image.mas[i] = ESDWORD[i*3+screen_copy] & 0xFFFFFF;
	}
	DrawCanvas();
	
	if (mouse.down) {
		screen_copy = free(screen_copy);
		SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
		actionsHistory.saveCurrentState();
		setCurrentTool(previousTool);
	}
}
