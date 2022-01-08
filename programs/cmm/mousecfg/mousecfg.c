// Mouse Configuration Utility ver 1.7

#define MEMSIZE 4096*11

#include "..\lib\strings.h"
#include "..\lib\mem.h"
#include "..\lib\fs.h"
#include "..\lib\gui.h"
#include "..\lib\obj\libini.h"
#include "..\lib\patterns\restart_process.h"

// Translatiions
#ifdef LANG_RUS
	?define WINDOW_TITLE "Проверка и настройка параметров мыши"
	?define CHECK_MOUSE_1 "Нажмите на этой области"
	?define CHECK_MOUSE_2 "для проверки кнопок мыши"
	?define POINTER_SPEED "Делитель скорости указателя мыши"
	?define ACCELERATION_TEXT "Чувствительность указателя мыши"
	?define DOUBLE_CLICK_TEXT "Задержка двойного клика мышью"
	?define MOUSE_EMULATION "Управление указателем мыши через клавиатуру (F1)"
	?define COMMOUSE "Загрузить драйвер мыши для COM-порта (F2)"
	?define MADMOUSE "Сквозные для курсора стороны экрана"
	?define COMMOUSE_LOADED "'Драйвер для COM мыши был загружен' -O"
	?define COMMOUSE_CAN_NOT_UNLOAD "'Остановка драйвера невозможна' -W"
#else
	?define WINDOW_TITLE "Mouse testing and configuration"
	?define CHECK_MOUSE_1 "Click on this area to"
	?define CHECK_MOUSE_2 "check your mouse buttons"
	?define POINTER_SPEED "Mouse pointer speed divider"
	?define ACCELERATION_TEXT "Mouse pointer sensitivity"
	?define DOUBLE_CLICK_TEXT "Mouse double click delay"
	?define MOUSE_EMULATION "Enable mouse emulation using keyboard NumPad (F1)"
	?define COMMOUSE "Load mouse driver for COM-port (F2)"
	?define MADMOUSE "Through screen sides for pointer"
	?define COMMOUSE_LOADED "'Driver for COM mouse loaded' -O"
	?define COMMOUSE_CAN_NOT_UNLOAD "'Driver stop is impossible' -W"
#endif

#define FRAME_X 18
#define FRAME_Y 18
:block mouse_frame = { FRAME_X, FRAME_Y, NULL, 130 };
:more_less_box pointer_speed      = { NULL, 0, 64, POINTER_SPEED };
:more_less_box acceleration       = { NULL, 0, 64, ACCELERATION_TEXT };
:more_less_box double_click_delay = { NULL, 0, 999, DOUBLE_CLICK_TEXT, 8 };
:checkbox emulation = { MOUSE_EMULATION, NULL };
:checkbox madmouse = { MADMOUSE, NULL };
:checkbox com_mouse = { COMMOUSE, NULL };

char ini_path[] = "/sys/settings/system.ini";
_ini ini_drivers = { #ini_path, "loaded drivers" };
_ini ini_mouse = { #ini_path, "mouse" };

void main() {
	proc_info Form;
	int id;

	load_dll(libini, #lib_init,1);

	LoadCfg();

	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);

	loop() switch(@WaitEvent())
	{
		case evMouse:
				mouse.get();
				IF (mouse_frame.hovered()) DrawMouseImage(mouse.lkm,mouse.pkm,mouse.mkm,mouse.vert);
				IF (mouse.click) || (mouse.up) DrawMouseImage(0,0,0,0);
				break;

		CASE evButton:
				id = @GetButtonID();
				IF (1 == id) ExitApp();
				else IF (pointer_speed.click(id)) ApplyCfg();
				else IF (acceleration.click(id)) ApplyCfg();
				else IF (double_click_delay.click(id)) ApplyCfg();
				else IF (emulation.click(id)) {
					EventClickEmulation();
				}
				else IF (madmouse.click(id)) {
					IF (madmouse.checked == true) RunProgram("/sys/madmouse", 0);
					ELSE KillProcessByName("madmouse", SINGLE);
					break;
				}
				else IF (id == com_mouse.id) {
					EventClickComMouse();
				}
				break;

		case evKey:
				@GetKeyScancode();
				IF (AL == SCAN_CODE_ESC) ExitApp();
				IF (AL == SCAN_CODE_F1) {
					emulation.click(emulation.id);
					EventClickEmulation();
				}
				IF (AL == SCAN_CODE_F2) EventClickComMouse();
				break;

		case evReDraw:
				sc.get();
				DefineAndDrawWindow(430, 150, 460, 343+skin_h,0x34,sc.work,WINDOW_TITLE,0);
				GetProcessInfo(#Form, SelfInfo);
				if (Form.status_window&ROLLED_UP) break;
				mouse_frame.w = - FRAME_X * 2 + Form.cwidth;
				DefineButton(FRAME_X, FRAME_Y, mouse_frame.w,
					mouse_frame.h, 99+BT_NOFRAME, 0xF0F2F3); //needed to handle mouse_up and refresh mouse image
				WriteText(FRAME_X + 110, FRAME_Y + 25, 0x90, 0x2C343C, CHECK_MOUSE_1);
				WriteText(FRAME_X + 110, FRAME_Y + 45, 0x90, 0x2C343C, CHECK_MOUSE_2);
				DrawMouseImage(0,0,0,0);
				DrawControls();
	}
}

:byte panels_img_data[] = FROM "mouse_image.raw";

#define red    0xff0000
#define yellow 0xfff600
#define white  0xffffff
#define dgrey  0x2d353d

:struct IMG_PAL{ dword back, shad1, contour, left,  right, middle, mwhite; }
         pal = { 0xF0F2F3,0xABB0B2, dgrey,   white, white, dgrey,  white  };

void DrawMouseImage(dword l,r,m,v) {
	#define IMG_W 59
	#define IMG_H 100

	IF (l) pal.left = red;
	IF (m) pal.middle = red;
	IF (r) pal.right = red;
	IF (v) pal.middle = yellow;

	PutPaletteImage(#panels_img_data,IMG_W,IMG_H,18+30,18+16,8,#pal);
	pal.left = pal.right = white;
	pal.middle = dgrey;
	IF (v) {
		pause(10);
		DrawMouseImage(l,r,m,0);
	}
}

void DrawControls() {
	incn y;
	y.n = FRAME_Y+115;
	pointer_speed.draw(FRAME_X, y.inc(30));
	acceleration.draw(FRAME_X, y.inc(30));
	double_click_delay.draw(FRAME_X, y.inc(30));
	emulation.draw(FRAME_X, y.inc(33));
	com_mouse.draw(FRAME_X, y.inc(27));
	madmouse.draw(FRAME_X, y.inc(27));
}

void LoadCfg() {
	acceleration.value = @GetMouseAcceleration();
	pointer_speed.value = @GetMouseSpeed();
	double_click_delay.value = @GetMouseDoubleClickDelay();
	com_mouse.checked = ini_drivers.GetInt("com_mouse", 0);
	madmouse.checked = CheckProcessExists("MADMOUSE");
	emulation.checked = CheckProcessExists("MOUSEMUL");
}

void ExitApp() {
	ini_drivers.SetInt("com_mouse", com_mouse.checked);
	ini_mouse.SetInt("speed", pointer_speed.value);
	ini_mouse.SetInt("acceleration", acceleration.value);
	ini_mouse.SetInt("double_click_delay", double_click_delay.value);
	@ExitProcess();
}

void ApplyCfg() {
	@SetMouseSpeed(pointer_speed.value);
	@SetMouseAcceleration(acceleration.value);
	@SetMouseDoubleClickDelay(double_click_delay.value);
}

void EventClickComMouse()
{
	if (!com_mouse.checked)
	{
		if (RunProgram("/sys/loaddrv", "COMMOUSE")>=0) {
			notify(COMMOUSE_LOADED);
			com_mouse.click(com_mouse.id);
		} else {
			notify("'Error running LOADDRV' -E");
		}
	} else {
		notify(COMMOUSE_CAN_NOT_UNLOAD);
	}
}

void EventClickEmulation()
{
	IF (emulation.checked == true) {
		RunProgram("/sys/mousemul", 0);
	}	ELSE {
		KillProcessByName("mousemul", SINGLE);
	}
}

stop:
