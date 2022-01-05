(*
    Copyright 2016, 2018, 2022 Anton Krotov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*)

MODULE ConsoleLib;

IMPORT sys := SYSTEM, KOSAPI;

CONST

    COLOR_BLUE*      = 001H;
    COLOR_GREEN*     = 002H;
    COLOR_RED*       = 004H;
    COLOR_BRIGHT*    = 008H;
    BGR_BLUE*        = 010H;
    BGR_GREEN*       = 020H;
    BGR_RED*         = 040H;
    BGR_BRIGHT*      = 080H;
    IGNORE_SPECIALS* = 100H;
    WINDOW_CLOSED*   = 200H;

TYPE

    gets2_callback* = PROCEDURE [stdcall] (keycode: INTEGER; pstr: INTEGER; VAR n, pos: INTEGER);

VAR

    version*           : INTEGER;
    init*              : PROCEDURE [stdcall] (wnd_width, wnd_height, scr_width, scr_height, title: INTEGER);
    exit*              : PROCEDURE [stdcall] (bCloseWindow: BOOLEAN);
    write_asciiz*      : PROCEDURE [stdcall] (string: INTEGER);
    write_string*      : PROCEDURE [stdcall] (string, length: INTEGER);
    get_flags*         : PROCEDURE [stdcall] (): INTEGER;
    set_flags*         : PROCEDURE [stdcall] (new_flags: INTEGER): INTEGER;
    get_font_height*   : PROCEDURE [stdcall] (): INTEGER;
    get_cursor_height* : PROCEDURE [stdcall] (): INTEGER;
    set_cursor_height* : PROCEDURE [stdcall] (new_height: INTEGER): INTEGER;
    getch*             : PROCEDURE [stdcall] (): INTEGER;
    getch2*            : PROCEDURE [stdcall] (): INTEGER;
    kbhit*             : PROCEDURE [stdcall] (): INTEGER;
    gets*              : PROCEDURE [stdcall] (str, n: INTEGER): INTEGER;
    gets2*             : PROCEDURE [stdcall] (callback: gets2_callback; str, n: INTEGER): INTEGER;
    cls*               : PROCEDURE [stdcall] ();
    get_cursor_pos*    : PROCEDURE [stdcall] (VAR x, y: INTEGER);
    set_cursor_pos*    : PROCEDURE [stdcall] (x, y: INTEGER);
    set_title*         : PROCEDURE [stdcall] (title: INTEGER);

PROCEDURE open*(wnd_width, wnd_height, scr_width, scr_height: INTEGER; title: ARRAY OF CHAR);
BEGIN
  init(wnd_width, wnd_height, scr_width, scr_height, sys.ADR(title[0]))
END open;

PROCEDURE main;
VAR Lib: INTEGER;

  PROCEDURE GetProc(Lib, v: INTEGER; name: ARRAY OF CHAR);
  VAR a: INTEGER;
  BEGIN
    a := KOSAPI.GetProcAdr(name, Lib);
    ASSERT(a # 0);
    sys.PUT(v, a)
  END GetProc;

BEGIN
  Lib := KOSAPI.LoadLib("/sys/lib/Console.obj");
  ASSERT(Lib # 0);
  GetProc(Lib, sys.ADR(version),           "version");
  GetProc(Lib, sys.ADR(init),              "con_init");
  GetProc(Lib, sys.ADR(exit),              "con_exit");
  GetProc(Lib, sys.ADR(write_asciiz),      "con_write_asciiz");
  GetProc(Lib, sys.ADR(write_string),      "con_write_string");
  GetProc(Lib, sys.ADR(get_flags),         "con_get_flags");
  GetProc(Lib, sys.ADR(set_flags),         "con_set_flags");
  GetProc(Lib, sys.ADR(get_font_height),   "con_get_font_height");
  GetProc(Lib, sys.ADR(get_cursor_height), "con_get_cursor_height");
  GetProc(Lib, sys.ADR(set_cursor_height), "con_set_cursor_height");
  GetProc(Lib, sys.ADR(getch),             "con_getch");
  GetProc(Lib, sys.ADR(getch2),            "con_getch2");
  GetProc(Lib, sys.ADR(kbhit),             "con_kbhit");
  GetProc(Lib, sys.ADR(gets),              "con_gets");
  GetProc(Lib, sys.ADR(gets2),             "con_gets2");
  GetProc(Lib, sys.ADR(cls),               "con_cls");
  GetProc(Lib, sys.ADR(get_cursor_pos),    "con_get_cursor_pos");
  GetProc(Lib, sys.ADR(set_cursor_pos),    "con_set_cursor_pos");
  GetProc(Lib, sys.ADR(set_title),         "con_set_title");
END main;

BEGIN
  main
END ConsoleLib.