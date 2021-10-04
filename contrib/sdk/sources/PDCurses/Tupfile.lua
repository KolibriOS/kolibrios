if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

CFLAGS = " -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -D_KOLIBRI -DSDL_strlcpy=strncpy "
INCLUDES =  INCLUDES .. " -I. " .. "-I ../SDL-1.2.2_newlib/include "

compile_gcc {
    "sdl1/pdcclip.c",
    "sdl1/pdcgetsc.c",
    "sdl1/pdcscrn.c",
    "sdl1/pdcsetsc.c",  
    "sdl1/pdcdisp.c", 
    "sdl1/pdckbd.c",
    "sdl1/pdcutil.c",
    "pdcurses/addstr.c",
    "pdcurses/move.c",
    "pdcurses/attr.c",
    "pdcurses/panel.c",
    "pdcurses/initscr.c",
    "pdcurses/slk.c",
    "pdcurses/inopts.c",
    "pdcurses/keyname.c",
    "pdcurses/clear.c",
    "pdcurses/touch.c",
    "pdcurses/refresh.c",
    "pdcurses/window.c",
    "pdcurses/delch.c",
    "pdcurses/beep.c",
    "pdcurses/mouse.c",
    "pdcurses/inchstr.c",
    "pdcurses/overlay.c",
    "pdcurses/deleteln.c",
    "pdcurses/termattr.c",
    "pdcurses/kernel.c",
    "pdcurses/inch.c",
    "pdcurses/getstr.c",
    "pdcurses/getch.c",
    "pdcurses/scroll.c",
    "pdcurses/border.c",
    "pdcurses/scr_dump.c",
    "pdcurses/pad.c",
    "pdcurses/addch.c",
    "pdcurses/insstr.c",
    "pdcurses/scanw.c",
    "pdcurses/outopts.c",
    "pdcurses/insch.c",
    "pdcurses/color.c",
    "pdcurses/printw.c",
    "pdcurses/instr.c",
    "pdcurses/bkgd.c",
    "pdcurses/getyx.c",
    "pdcurses/util.c",
    "pdcurses/addchstr.c",
    "pdcurses/debug.c",
}

tup.rule(OBJS, "kos32-ar -rcs %o %f", "../../lib/libcurses.a", OBJS)

