MODULE Dialogs;

IMPORT
	KOSAPI, SYSTEM, OpenDlg, ColorDlg;


CONST
	btnNone  =  0;
	btnClose =  1;
	btnOpen  = 17;
	btnColor = 18;


VAR
	header: ARRAY 1024 OF CHAR;
	back_color: INTEGER;


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


PROCEDURE WaitForEvent (): INTEGER;
	RETURN KOSAPI.sysfunc1(10)
END WaitForEvent;


PROCEDURE ExitApp;
BEGIN
	KOSAPI.sysfunc1(-1)
END ExitApp;


PROCEDURE pause (t: INTEGER);
BEGIN
	KOSAPI.sysfunc2(5, t)
END pause;


PROCEDURE Buttons;

	PROCEDURE Button (id, X, Y, W, H: INTEGER; Caption: ARRAY OF CHAR);
	VAR
		n: INTEGER;
	BEGIN
		n := LENGTH(Caption);
		KOSAPI.sysfunc5(8, X*65536 + W, Y*65536 + H, id, 00C0C0C0H);
		X := X + (W - 8*n) DIV 2;
		Y := Y + (H - 14) DIV 2;
		KOSAPI.sysfunc6(4, X*65536 + Y, LSL(48, 24), SYSTEM.ADR(Caption[0]), n, 0)
	END Button;

BEGIN
	Button(btnOpen,   5, 5, 70, 25, "open");
	Button(btnColor, 85, 5, 70, 25, "color");
END Buttons;


PROCEDURE draw_window;
BEGIN
	BeginDraw;
	DefineAndDrawWindow(200, 200, 500, 100, back_color, 51, 0, 0, header);
	Buttons;
	EndDraw;
END draw_window;


PROCEDURE OpenFile (Open: OpenDlg.Dialog);
BEGIN
	IF Open # NIL THEN
		OpenDlg.Show(Open, 500, 450);
		WHILE Open.status = 2 DO
			pause(30)
		END;
		IF Open.status = 1 THEN
			COPY(Open.FilePath, header)
		END
	END
END OpenFile;


PROCEDURE SelColor (Color: ColorDlg.Dialog);
BEGIN
	IF Color # NIL THEN
		ColorDlg.Show(Color);
		WHILE Color.status = 2 DO
			pause(30)
		END;
		IF Color.status = 1 THEN
			back_color := Color.color
		END
	END
END SelColor;


PROCEDURE GetButton (): INTEGER;
VAR
	btn: INTEGER;
BEGIN
	btn := KOSAPI.sysfunc1(17);
	IF btn MOD 256 = 0 THEN
		btn := btn DIV 256
	ELSE
		btn := btnNone
	END
	RETURN btn
END GetButton;


PROCEDURE main;
CONST
	EVENT_REDRAW = 1;
	EVENT_KEY    = 2;
	EVENT_BUTTON = 3;
VAR
	Open: OpenDlg.Dialog;
	Color: ColorDlg.Dialog;
BEGIN
	back_color := 00FFFFFFH;
	header := "Dialogs";
	Open := OpenDlg.Create(draw_window, 0, "/sys", "ASM|TXT|INI");
	Color := ColorDlg.Create(draw_window);

	WHILE TRUE DO
		CASE WaitForEvent() OF
		|EVENT_REDRAW:
			draw_window

		|EVENT_KEY:

		|EVENT_BUTTON:
			CASE GetButton() OF
			|btnNone:
			|btnClose: ExitApp
			|btnOpen:  OpenFile(Open)
			|btnColor: SelColor(Color)
			END
		END
	END
END main;


BEGIN
	main
END Dialogs.