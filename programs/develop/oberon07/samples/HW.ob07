MODULE HW;

IMPORT
	SYSTEM, KOSAPI;


PROCEDURE BeginDraw;
BEGIN
	KOSAPI.sysfunc2(12, 1)
END BeginDraw;


PROCEDURE EndDraw;
BEGIN
	KOSAPI.sysfunc2(12, 2)
END EndDraw;


PROCEDURE DefineAndDrawWindow (left, top, width, height, color, style, hcolor, hstyle: INTEGER; header: ARRAY OF CHAR);
BEGIN
	KOSAPI.sysfunc6(0, left*65536 + width, top*65536 + height, color + LSL(style, 24), hcolor + LSL(hstyle, 24), SYSTEM.ADR(header[0]))
END DefineAndDrawWindow;


PROCEDURE WriteTextToWindow (x, y, color: INTEGER; text: ARRAY OF CHAR);
BEGIN
	KOSAPI.sysfunc6(4, x*65536 + y, color + LSL(48, 24), SYSTEM.ADR(text[0]), LENGTH(text), 0)
END WriteTextToWindow;


PROCEDURE WaitForEvent (): INTEGER;
	RETURN KOSAPI.sysfunc1(10)
END WaitForEvent;


PROCEDURE ExitApp;
BEGIN
	KOSAPI.sysfunc1(-1)
END ExitApp;


PROCEDURE draw_window (header, text: ARRAY OF CHAR);
CONST
	WHITE = 0FFFFFFH;
	RED   = 0C00000H;
	GREEN = 0008000H;
	BLUE  = 00000C0H;
	GRAY  = 0808080H;
BEGIN
	BeginDraw;
	DefineAndDrawWindow(200, 200, 300, 150, WHITE, 51, 0, 0, header);
	WriteTextToWindow( 5, 10, RED,   text);
	WriteTextToWindow(35, 30, GREEN, text);
	WriteTextToWindow(65, 50, BLUE,  text);
	WriteTextToWindow(95, 70, GRAY,  text);
	EndDraw
END draw_window;


PROCEDURE main (header, text: ARRAY OF CHAR);
CONST
	EVENT_REDRAW = 1;
	EVENT_KEY    = 2;
	EVENT_BUTTON = 3;
BEGIN
	WHILE TRUE DO
		CASE WaitForEvent() OF
		|EVENT_REDRAW: draw_window(header, text)
		|EVENT_KEY:    ExitApp
		|EVENT_BUTTON: ExitApp
		END
	END
END main;


BEGIN
	main("Hello", "Hello, world!")
END HW.