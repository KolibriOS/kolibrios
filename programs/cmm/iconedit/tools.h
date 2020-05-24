#define TOIMAGE 1
#define TOCANVAS 2


struct TOOL {
	int id;
	dword cursor;
	void (*activate)();
	void (*deactivate)();
	void (*onMouseEvent)(int x, int y, int lkm, int pkm);
	void (*onKeyEvent)(dword keycode);
	void (*onCanvasDraw)();
} tools[8];

int previousTool = -1;
int currentTool = -1;

enum {
	TOOL_NONE = -1,
	TOOL_PENCIL,
	TOOL_PIPETTE,
	TOOL_FILL,
	TOOL_LINE,
	TOOL_RECT,
	TOOL_BAR,
	TOOL_SELECT,
	TOOL_SCREEN_COPY
};
#include "tools/pencil.h";
#include "tools/pipette.h";
#include "tools/fill.h";
#include "tools/simple_figure.h";
#include "tools/selection.h";
#include "tools/screen_copy.h";


void initTools() 
{
	tools[TOOL_PENCIL].id = TOOL_PENCIL;
	tools[TOOL_PENCIL].cursor = #CursorPencil;
	tools[TOOL_PENCIL].onMouseEvent = #PencilTool_onMouseEvent;
	tools[TOOL_PENCIL].deactivate = #PencilTool_reset;
	
	tools[TOOL_PIPETTE].id = TOOL_PIPETTE;
	tools[TOOL_PIPETTE].cursor = #CursorPipette;
	tools[TOOL_PIPETTE].activate = #PipetteTool_activate;
	tools[TOOL_PIPETTE].onMouseEvent = #PipetteTool_onMouseEvent;
	tools[TOOL_PIPETTE].onKeyEvent = #PipetteTool_onKeyEvent;
	
	tools[TOOL_FILL].id = TOOL_FILL;
	tools[TOOL_FILL].cursor = #CursorFill;
	tools[TOOL_FILL].onMouseEvent = #FillTool_onMouseEvent;
	
	tools[TOOL_LINE].id = TOOL_LINE;
	tools[TOOL_LINE].cursor = #CursorLine;
	tools[TOOL_LINE].activate = #SimpleFigureTool_Reset;
	tools[TOOL_LINE].deactivate = #SimpleFigureTool_Reset;
	tools[TOOL_LINE].onMouseEvent = #SimpleFigureTool_onMouseEvent;
	tools[TOOL_LINE].onCanvasDraw = #SimpleFigureTool_onCanvasDraw;
	
	tools[TOOL_RECT].id = TOOL_RECT;
	tools[TOOL_RECT].cursor = #CursorRectangle;
	tools[TOOL_RECT].activate = #SimpleFigureTool_Reset;
	tools[TOOL_RECT].deactivate = #SimpleFigureTool_Reset;
	tools[TOOL_RECT].onMouseEvent = #SimpleFigureTool_onMouseEvent;
	tools[TOOL_RECT].onCanvasDraw = #SimpleFigureTool_onCanvasDraw;	

	tools[TOOL_BAR].id = TOOL_BAR;
	tools[TOOL_BAR].cursor = #CursorBar;
	tools[TOOL_BAR].activate = #SimpleFigureTool_Reset;
	tools[TOOL_BAR].deactivate = #SimpleFigureTool_Reset;
	tools[TOOL_BAR].onMouseEvent = #SimpleFigureTool_onMouseEvent;
	tools[TOOL_BAR].onCanvasDraw = #SimpleFigureTool_onCanvasDraw;	

	tools[TOOL_SELECT].id = TOOL_SELECT;
	tools[TOOL_SELECT].cursor = #CursorSelect;
	tools[TOOL_SELECT].activate = #SelectTool_activate;
	tools[TOOL_SELECT].deactivate = #SelectTool_deactivate;
	tools[TOOL_SELECT].onMouseEvent = #SelectTool_onMouseEvent;
	tools[TOOL_SELECT].onCanvasDraw = #SelectTool_onCanvasDraw;	
	tools[TOOL_SELECT].onKeyEvent = #SelectTool_onKeyEvent;	

	tools[TOOL_SCREEN_COPY].id = TOOL_SCREEN_COPY;
	tools[TOOL_SCREEN_COPY].cursor = NULL;
	tools[TOOL_SCREEN_COPY].activate = #ScreenCopy_activate;
	tools[TOOL_SCREEN_COPY].onMouseEvent = #ScreenCopy_onMouseEvent;
	tools[TOOL_SCREEN_COPY].onKeyEvent = #ScreenCopy_onKeyEvent;	
}


void resetCurrentTool() {
	if ((currentTool != TOOL_NONE) && (tools[currentTool].deactivate != 0)) {
		tools[currentTool].deactivate();
	}
	currentTool = TOOL_NONE;
}

void setCurrentTool(int index) {
	previousTool = currentTool;
	resetCurrentTool();

	currentTool = index;
	
	if ((index != TOOL_NONE) && (tools[index].activate != 0))
		tools[index].activate();

	Cursor.Restore();
	if (wrapper.hovered()) SetCursor();
	DrawCanvas();
	DrawLeftPanelSelection();
}

void SetCursor()
{
	if (tools[currentTool].cursor) && (!Cursor.CursorPointer) {
		Cursor.Load(tools[currentTool].cursor);
		Cursor.Set();
	}
}