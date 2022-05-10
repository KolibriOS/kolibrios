/*
    This is wrapper for Inputbox.obj sys library
    https://board.kolibrios.org/viewtopic.php?f=24&t=3767&sid=fd2ca95b24eec430db0c61d977f5d8ba#p71585

    Adapted for TCC's dynamic API by Magomed Kostoev, 2020
*/

#ifndef KOLIBRI_INPUTBOX_H
#define KOLIBRI_INPUTBOX_H

#include <stddef.h>

DLLAPI unsigned __stdcall InputBox(void* Buffer, char* Caption, char* Prompt, char* Default,
                                    unsigned long Flags, unsigned long BufferSize, void* RedrawProc);

#endif // KOLIBRI_INPUTBOX_H
