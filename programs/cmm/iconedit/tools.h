#define TOIMAGE 1
#define TOCANVAS 2


struct Tool {
	int id;
	void (*activate)();
	void (*deactivate)();
	void (*onMouseEvent)(int x, int y, int lkm, int pkm);
	void (*onKeyEvent)(dword keycode);
	void (*onCanvasDraw)();
};

int previousTool = -1;
int currentTool = -1;
Tool tools[6];

enum {
	TOOL_NONE = -1,
	TOOL_PENCIL,
	TOOL_PIPETTE,
	TOOL_FILL,
	TOOL_LINE,
	TOOL_RECT,
	TOOL_SELECT
};
#include "tools/pencil.h";
#include "tools/pipette.h";
#include "tools/fill.h";
#include "tools/selection.h";
#include "tools/simple_figure.h";


void initTools() 
{
	tools[0].id = TOOL_PENCIL;
	tools[0].onMouseEvent = #PencilTool_onMouseEvent;
	tools[0].deactivate = #PencilTool_reset;
	
	tools[1].id = TOOL_PIPETTE;
	tools[1].activate = #PipetteTool_activate;
	tools[1].onMouseEvent = #PipetteTool_onMouseEvent;
	
	tools[2].id = TOOL_FILL;
	tools[2].onMouseEvent = #FillTool_onMouseEvent;
	
	tools[3].id = TOOL_LINE;
	tools[3].activate = #SimpleFigureTool_Reset;
	tools[3].deactivate = #SimpleFigureTool_Reset;
	tools[3].onMouseEvent = #SimpleFigureTool_onMouseEvent;
	tools[3].onCanvasDraw = #SimpleFigureTool_onCanvasDraw;
	
	tools[4].id = TOOL_RECT;
	tools[4].activate = #SimpleFigureTool_Reset;
	tools[4].deactivate = #SimpleFigureTool_Reset;
	tools[4].onMouseEvent = #SimpleFigureTool_onMouseEvent;
	tools[4].onCanvasDraw = #SimpleFigureTool_onCanvasDraw;	

	tools[5].id = TOOL_SELECT;
	tools[5].activate = #SelectTool_activate;
	tools[5].deactivate = #SelectTool_deactivate;
	tools[5].onMouseEvent = #SelectTool_onMouseEvent;
	tools[5].onCanvasDraw = #SelectTool_onCanvasDraw;	
	tools[5].onKeyEvent = #SelectTool_onKeyEvent;	
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

	DrawLeftPanel();
	DrawCanvas();
}

//===================================================//
//                                                   //
//                    FUNTIONS                       //
//                                                   //
//===================================================//

