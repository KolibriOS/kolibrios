#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <kos32sys1.h>
#include "gh_core.c"

#define CMD_LEN 255
#define TITLE "GameHack 1.0 ALPHA "

char cmd_line[CMD_LEN];
char cmd_line_tmp[CMD_LEN];

void notify_show(char *text)
{
   start_app("/sys/@notify", text);
}

void cmd_processing()
{
    strcpy(cmd_line_tmp, cmd_line);
    char *cmd = strtok(cmd_line_tmp, " \n");
    if(!strcmp(cmd, "pause")){
        kdebugger_pause(PID);
    }
    else if(!strcmp(cmd, "play")){
        kdebugger_play(PID);
    }
    else if(!strcmp(cmd, "exit")){
        exit(0);
    }
    else if(!strcmp(cmd, "write")){
        unsigned addr=0;
        int val =0;
        if(sscanf(cmd_line, "%s %x %d %d",cmd_line, &addr, &val, &val)==3){   
            if(kdebugger_write(PID, sizeof(int), addr, &val)==-1){
                puts("Memory write error!");
            }
        }else{
            puts("Invalid arguments!");
        }
    }
    else if(!strcmp(cmd, "read")){
        unsigned addr=0;
        int val =0;
        if(sscanf(cmd_line, "%s %x %x",cmd_line, &addr, &addr)==2){   
            if(kdebugger_read(PID, sizeof(int), addr, &val)==-1){
                puts("Memory read error!");
            }
            printf("0x%.8X: %d\n", addr, val);
        }else{
            puts("Invalid arguments!");
        }
    }


    else if(!strcmp(cmd, "help"))
    {
        puts("Commands:");
        puts("  write [addres] [value] - Write DWORD value by address.");
        puts("  read  [addres] [value] - Read DWORD value by address.");
        puts("  pause                  - Suspend the game (process)."  );
        puts("  play                   - Resume running the game(process).");
        puts("  find  [value]          - Search for DWORD value in memory(VIP).");
    }
    else if(!strcmp(cmd, "find"))
    {
        puts("Not yet implemented ...");
    }
    else if(cmd != NULL){
        puts("Unknown command!");
    }
}

int main(int argc, char* argv[])
{
    if (argc!=2 ){
        notify_show("'No game selected!' -E");
        exit(0);
    }
    con_init_console_dll();
    con_set_title(TITLE);
    PID = load_game(argv[1], NULL);
    PID = 2;
    if(PID<0){
        notify_show("'Game not loaded!' -E");
        exit(0);
    }
    kdebugger_play(PID);
    while (1){       
        printf("GameHack> ");
        con_gets(cmd_line, CMD_LEN);
        cmd_processing();
        memset(cmd_line, '\n', CMD_LEN);
    }
}  