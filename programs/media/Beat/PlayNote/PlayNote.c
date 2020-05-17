
/*
 * Author: JohnXenox aka Aleksandr Igorevich.
 *
 * Programme name: PlayNote
 * Description: The programme to play a note.
 *
 * Works from command line, only!
*/

// To generate .wav file with sox (to listen):
//  sox -n -L -c 1 -b 16 -r 48000 Note_C5.wav synth 1 sine 1046.4
// To generate .raw file with sox (to PlayNote):
//  sox -n -L -c 1 -b 16 -r 48000 Note_C5.raw synth 1 sine 1046.4

#define CREATION_DATE "2020.05.17"

#include <conio.h>
#include <stdio.h>
#include <string.h>

#include "PlayNote_lib1.h"
#include "PlayNote_lib2.h"


unsigned int drv_ver = 0;
unsigned int buffer = 0;

unsigned int *raw_file_data = 0;

char raw_file[4096] = {0};



int main(int argc, char** argv)
{
// ============================================================ //
// checks memory availability ================================= //

    if(initMemory() == 0) return -1;

// ============================================================ //
// sets a current path to a raw file ========================== //

    //setCurrentPathToARawFile(raw_file, argv[0], "Note_C6_0.5.raw");

_ksys_set_wanted_events(0);

// ============================================================ //
// processes the command line arguments ======================= //

    if (argc > 1)
    {
        //printfOnADebugBoard("ARGV1: %s\n", argv[1]);
        strcpy(raw_file, argv[1]);

        // checks to a full path to a file?
        if(*argv[1] != '/')
        {
            setCurrentPathToARawFile(raw_file, argv[0], argv[1]);
        }
    }
    else
    {
        if (con_init_console_dll()) return 1; // init fail

        con_set_title("Useful info!");

        con_printf("\n Name: PlayNote");
        con_printf("\n Date: %s", CREATION_DATE);
        con_printf("\n Description: The programme to play a note.\n");

        con_printf("\n Author: JohnXenox\n");

        con_printf("\n Usage: PlayNote <path>\n");
        con_printf("  path - path to a file to be played.\n\n");

        con_printf(" Examples:\n");
        con_printf("  PlayNote note.raw\n");
        con_printf("  PlayNote /tmp0/1/note.raw\n");

        return 0;
    }

    //printfOnADebugBoard("raw_file path: %s\n", argv[1]);

// ============================================================ //
// plays a note =============================================== //

    int raw_file_length = 0;
    raw_file_data = openFile(&raw_file_length,raw_file);
    if(raw_file_data == 0) return -2;

    _InitSound(&drv_ver);
    _CreateBuffer((PCM_STATIC | PCM_1_16_48), raw_file_length, &buffer);
    _SetBuffer((int*)buffer, (int*)raw_file_data, 0, raw_file_length);
    _SetBufferPos((int*)buffer, 0);

    _PlayBuffer((int*)buffer, 0);

    makeDelay(30);

    _StopBuffer((int*)buffer);

    return 0;
}








