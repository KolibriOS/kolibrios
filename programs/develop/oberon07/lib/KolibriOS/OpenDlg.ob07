(*
    Copyright 2016, 2018, 2020-2022 Anton Krotov

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

MODULE OpenDlg;

IMPORT sys := SYSTEM, KOSAPI;

CONST
  topen* = 0;
  tsave* = 1;
  tdir* = 2;

TYPE

  DRAW_WINDOW = PROCEDURE;

  TDialog = RECORD
    _type*,
    procinfo,
    com_area_name,
    com_area,
    opendir_path,
    dir_default_path,
    start_path: INTEGER;
    draw_window: DRAW_WINDOW;
    status*,
    openfile_path,
    filename_area: INTEGER;
    filter_area:
      POINTER TO RECORD
        size: INTEGER;
        filter: ARRAY 4096 OF CHAR
      END;
    X, Y: INTEGER;

    procinf: ARRAY 1024 OF CHAR;
    s_com_area_name: ARRAY 32 OF CHAR;
    s_opendir_path,
    s_dir_default_path,
    FilePath*,
    FileName*: ARRAY 4096 OF CHAR
  END;

  Dialog* = POINTER TO TDialog;

VAR

  Dialog_start, Dialog_init: PROCEDURE [stdcall] (od: Dialog);


PROCEDURE Show*(od: Dialog; Width, Height: INTEGER);
BEGIN
  IF od # NIL THEN
    od.X := Width;
    od.Y := Height;
    Dialog_start(od)
  END
END Show;

PROCEDURE Create*(draw_window: DRAW_WINDOW; _type: INTEGER; def_path, filter: ARRAY OF CHAR): Dialog;
VAR res: Dialog; n, i: INTEGER;

  PROCEDURE replace(VAR str: ARRAY OF CHAR; c1, c2: CHAR);
  VAR i: INTEGER;
  BEGIN
    i := LENGTH(str) - 1;
    WHILE i >= 0 DO
      IF str[i] = c1 THEN
        str[i] := c2
      END;
      DEC(i)
    END
  END replace;

BEGIN
  NEW(res);
  IF res # NIL THEN
    NEW(res.filter_area);
    IF res.filter_area # NIL THEN
      res.s_com_area_name    := "FFFFFFFF_open_dialog";
      res.com_area           := 0;
      res._type              := _type;
      res.draw_window        := draw_window;
      COPY(def_path, res.s_dir_default_path);
      COPY(filter,   res.filter_area.filter);

      n := LENGTH(res.filter_area.filter);
      FOR i := 0 TO 3 DO
        res.filter_area.filter[n + i] := "|"
      END;
      res.filter_area.filter[n + 4] := 0X;

      res.X                  := 0;
      res.Y                  := 0;
      res.s_opendir_path     := res.s_dir_default_path;
      res.FilePath           := "";
      res.FileName           := "";
      res.status             := 0;
      res.filter_area.size   := LENGTH(res.filter_area.filter);
      res.procinfo           := sys.ADR(res.procinf[0]);
      res.com_area_name      := sys.ADR(res.s_com_area_name[0]);
      res.start_path         := sys.SADR("/sys/File managers/opendial");
      res.opendir_path       := sys.ADR(res.s_opendir_path[0]);
      res.dir_default_path   := sys.ADR(res.s_dir_default_path[0]);
      res.openfile_path      := sys.ADR(res.FilePath[0]);
      res.filename_area      := sys.ADR(res.FileName[0]);

      replace(res.filter_area.filter, "|", 0X);
      Dialog_init(res)
    ELSE
      DISPOSE(res)
    END
  END
  RETURN res
END Create;

PROCEDURE Destroy*(VAR od: Dialog);
BEGIN
  IF od # NIL THEN
    DISPOSE(od.filter_area);
    DISPOSE(od)
  END
END Destroy;

PROCEDURE Load;
VAR Lib: INTEGER;

  PROCEDURE GetProc(Lib, v: INTEGER; name: ARRAY OF CHAR);
  VAR a: INTEGER;
  BEGIN
    a := KOSAPI.GetProcAdr(name, Lib);
    ASSERT(a # 0);
    sys.PUT(v, a)
  END GetProc;

BEGIN
  Lib := KOSAPI.LoadLib("/sys/Lib/Proc_lib.obj");
  GetProc(Lib, sys.ADR(Dialog_init),  "OpenDialog_init");
  GetProc(Lib, sys.ADR(Dialog_start), "OpenDialog_start");
END Load;

BEGIN
  Load
END OpenDlg.