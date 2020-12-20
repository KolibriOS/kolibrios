#include <kos32sys.h>
#include <stdio.h>
#include <stdlib.h>

#include <import.h>

static void _ButtonCreate()
{
    define_button(js_touint32(J, 1),js_touint32(J,2), js_touint32(J,3), js_touint32(J,4));
}

static void _WindowCreate()
{
    sys_create_window(js_toint(J, 1), js_toint(J, 2), js_toint(J, 3), js_toint(J, 4), js_tostring(J,5), js_touint32(J,6), js_touint32(J,7));
}

static  void _StartDraw()
{
   begin_draw();
}

static void _EndDraw()
{
    end_draw();
}

static void _DebugPrintS()
{
    puts(js_tostring(J,1));

}

static void _GetEvent()
{   
    js_pushnumber(J, get_os_event()); 
}

static void _GetButtonEvent()
{
    js_pushnumber(J,get_os_button());
}

static void _WriteText() 
{
    draw_text_sys(js_tostring(J,1), js_toint32(J,2), js_toint32(J,3), js_toint32(J,4), js_touint32(J,5));
}

void import_functions()
{
    J = js_newstate(NULL, NULL, JS_STRICT);
    
    js_newcfunction(J, _WindowCreate, "WindowCreate", 7);
    js_setglobal(J, "WindowCreate");
    
    js_newcfunction(J, _StartDraw, "StartDraw", 0);
    js_setglobal(J, "StartDraw");

    js_newcfunction(J, _EndDraw, "EndDraw", 0);
    js_setglobal(J, "EndDraw");

    js_newcfunction(J, _GetEvent, "GetEvent", 0);
    js_setglobal(J, "GetEvent");

    js_newcfunction(J, _DebugPrintS, "DebugPrintS", 0);
    js_setglobal(J, "DebugPrintS");

    js_newcfunction(J, _ButtonCreate, "ButtonCreate", 4);
    js_setglobal(J, "ButtonCreate");

    js_newcfunction(J, _GetButtonEvent, "GetButtonEvent", 0);
    js_setglobal(J, "GetButtonEvent");

    js_newcfunction(J, (void*)exit, "Exit", 0);
    js_setglobal(J, "Exit");

    js_newcfunction(J, _WriteText, "WriteText", 5);
    js_setglobal(J, "WriteText");

}
