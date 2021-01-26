/*
    This is wrapper for Inputbox.obj sys library
    https://board.kolibrios.org/viewtopic.php?f=24&t=3767&sid=fd2ca95b24eec430db0c61d977f5d8ba#p71585

    Adapted for TCC's dynamic API by Magomed Kostoev, 2020
*/

#ifndef __KOS__INPUTBOX__H________
#define __KOS__INPUTBOX__H________

#define cdecl   __attribute__ ((cdecl))
#define stdcall __attribute__ ((stdcall))

extern unsigned stdcall (*InputBox)(void* Buffer, char* Caption, char* Prompt, char* Default,
                                    unsigned long Flags, unsigned long BufferSize, void* RedrawProc);

#endif // __KOS__INPUTBOX__H________
