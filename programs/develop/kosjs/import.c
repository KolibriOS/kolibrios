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

static void _DebugPrintS()
{
    puts(js_tostring(J,1));

}

static void _GetButtonEvent()
{
    js_pushnumber(J,get_os_button());
}

static void _WriteText() 
{
    draw_text_sys(js_tostring(J,1), js_toint32(J,2), js_toint32(J,3), js_toint32(J,4), js_touint32(J,5));
}

// KolibriSyscall(EAX, EBX, ECX, EDX, EDI, ESI)
static void _KolibriSyscall()
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(js_toint32(J,1)),
     "b"(js_toint32(J,2)),
     "c"(js_toint32(J,3)),
     "d"(js_toint32(J,4)),
     "D"(js_toint32(J,5)),
     "S"(js_toint32(J,6)) : "memory");
}

static void _KolibriSyscallReturnEAX()
{
    int _eax_;
    
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(_eax_)
    :"a"(js_toint32(J,1)),
     "b"(js_toint32(J,2)),
     "c"(js_toint32(J,3)),
     "d"(js_toint32(J,4)),
     "D"(js_toint32(J,5)),
     "S"(js_toint32(J,6)) : "memory");
     
      js_pushnumber(J, _eax_);
}

void import_functions()
{
    J = js_newstate(NULL, NULL, JS_STRICT);
    
    js_newcfunction(J, _WindowCreate, "WindowCreate", 7);
    js_setglobal(J, "WindowCreate");

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
    
    js_newcfunction(J, _KolibriSyscall, "KolibriSyscall", 6);
    js_setglobal(J, "KolibriSyscall");
    
    js_newcfunction(J, _KolibriSyscallReturnEAX, "KolibriSyscallReturnEAX", 6);
    js_setglobal(J, "KolibriSyscallReturnEAX");

}
