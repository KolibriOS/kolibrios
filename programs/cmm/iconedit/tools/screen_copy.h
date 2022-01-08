dword screen_copy;

void ScreenCopy_activate() {
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE);
	screen_copy = malloc(image.columns * image.rows * 3 +4);
}

void ScreenCopy_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	dword i;
	int x, y;

	x = mouse.x + Form.left + 5 - calc(image.columns/2);
	y = mouse.y + Form.top + skin_h - calc(image.rows/2);

	CopyScreen(
		screen_copy, 
		math.in(x, 0, screen.w - image.columns), 
		math.in(y, 0, screen.h - image.rows), 
		image.columns, 
		image.rows
	);

	for (i = 0; i < image.columns*image.rows; i++;) 
	{
		image.mas[i] = ESDWORD[i*3+screen_copy]; // & 0xFFFFFF;
	}
	DrawCanvas();
	
	if (mouse.down) {
		ScreenCopy_onKeyEvent(SCAN_CODE_ENTER);
	}
}

void ScreenCopy_onKeyEvent(dword keycode) {
	if (SCAN_CODE_ENTER == keycode) {
		actionsHistory.saveCurrentState();
		screen_copy = free(screen_copy);
		SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
		setCurrentTool(previousTool);
		if (!CheckActiveProcess(Form.ID)) ActivateWindow(GetProcessSlot(Form.ID));
	}
	if (SCAN_CODE_ESC == keycode) {
		ScreenCopy_onKeyEvent(SCAN_CODE_ENTER);
		actionsHistory.undoLastAction();
	}
}
