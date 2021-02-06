
/*
 * Programme name: Backy
 * Description: The programme for backing up a file.
 *
 * Backy.c
 * Author: JohnXenox aka Aleksandr Igorevich.
 *
 * Works from command line, only!
 */

#define CREATION_DATE "2020.05.27"

#include <conio.h>
#include <stdlib.h>
#include <string.h>

#include "Backy_lang.h"
#include "Backy_lib.h"

int date = 0;
int time = 0;

char years = 0;
char months = 0;
char days = 0;

char hours = 0;
char minutes = 0;
char seconds = 0;

char *data = 0;
int length = 0;

char path_in[4096] = {0};
char path_out[4096] = {0};

char num[3] = {0};

char full_date[25] = {0};
char ext[] = ".bak";

char flag = 0;

char state;


int main(int argc, char** argv)
{

// ============================================================ //
// preprocessing arguments from the command line. ============= //
//
// 0 argument - name of the programme.
// 1 argument - path to the file with name that need to be backup.
// 2 argument - the key (-o).
// 3 argument - path to the output directory without the name of the file.

// printf("Number of args: %d\n", argc);
// printf("Argv 0: %s\n", argv[0]);
// sprintf("Argv 1: %s\n\n", argv[1]);


// ============================================================ //
// process the command line arguments. ======================== //

    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            // if found the key "-o", then copy output path into the array "path_out".
            if (*argv[i] == '-') // && (*(argv[i] + 1) == 'o'))
            {
                // printf("Key -o is found!\n");

                i++;

                flag = 1;

                if (i <= argc)
                {
                    // copying of a current path into the array "path_out".
                    strcpy(path_out, argv[i]);

                    // printf("Path output is copyed!\n");

                    i++;

                    break;
                }
            }

            // if input path is found, then copy it into the array "path_in".
            if (*argv[i] == '/')
            {
                flag = 2;

                // copying of a current path into the buffer.
                strcpy(path_in, argv[i]);

                if (flag != 1)
                {
                    int idx = strlen(path_in);
                    while (path_in[idx]  !=  '/')
                    {
                        idx--;
                    }

                    strncpy(path_out, path_in, idx);
                }

                // printf("Path input is copyed!\n");
            }

            // if found characters.
            if ( (*argv[i] > '0') && (*argv[i] < '9') || \
                 (*argv[i] > 'A') && (*argv[i] < 'Z') || \
                 (*argv[i] > 'a') && (*argv[i] < 'z') )
            {
                 flag = 3;

                 strcpy(path_in, argv[0]);
                 // printf("Arg 0 is copyed!\n");

                 int idx = strlen(path_in);

                 while (path_in[idx]  !=  '/')
                 {
                     path_in[idx]  =  0;
                     idx--;
                 }

                 idx++;
                 strcpy(path_out, path_in);
                 strcpy(&path_in[idx], argv[1]);
                 // printf("Arg 1 is added!\n");

            }
        }

        // if not found the flag, then copy path from "path_in" into "path_out".
        if ((flag == 0) && (flag != 2)  && (flag != 3))
        {
            // copying the input path into the output path,
            strcpy(path_out, path_in);
            //printf("Path input is copyed into the path output!\n");
        }
    }
    else
    {
    	if (con_init_console_dll()) return 1; // init fail.

        con_set_title("Useful info!");

        #if defined (lang_en)

            con_printf("\n Name: Backy");
            con_printf("\n Date: %s", CREATION_DATE);
            con_printf("\n Description: The programm for backing up a file.\n");

            con_printf("\n Author: JohnXenox\n");

            con_printf("\n Usage: backy <path1> <-o path2>\n");
            con_printf("  path1 - path to a file to be backuped.\n");
            con_printf("  -o path2 - path to the output directory without the name of a file.\n\n");

            con_printf(" Examples:\n");
            con_printf("  backy test.c\n");
            con_printf("  backy test.c -o /tmp0/1/\n");
            con_printf("  backy /hd0/1/test.c\n");
            con_printf("  backy /hd0/1/test.c -o /tmp0/1/\n");

        #elif defined (lang_ru)

            con_printf("\n Имя: Backy");
            con_printf("\n Дата: %s", CREATION_DATE);
            con_printf("\n Описание: Программа для создания резервной копии файла.\n");

            con_printf("\n Автор: JohnXenox\n");

            con_printf("\n Использование: backy <path1> <-o path2>\n");
            con_printf("  path1 - путь к файлу, который надо скопировать.\n");
            con_printf("  -o path2 - путь к директории, в которую будет скопирована резервная копия файла.\n\n");

            con_printf(" Примеры:\n");
            con_printf("  backy test.c\n");
            con_printf("  backy test.c -o /tmp0/1/\n");
            con_printf("  backy /hd0/1/test.c\n");
            con_printf("  backy /hd0/1/test.c -o /tmp0/1/\n");

        #endif

        return 0;
    }

    //printf("Path_in: %s\n", path_in);
    //printf("Path_out: %s\n", path_out);


// ============================================================ //
// getting the time in BCD. =================================== //

    time = getTime();  // time = 0x00SSMMHH.

    hours = (char)time;
    minutes = (char)(time >> 8);
    seconds = (char)(time >> 16);

// ============================================================ //
// getting the date in BCD. =================================== //

    date = getDate();  // date = 0x00DDMMYY.

    years = (char)date;
    months = (char)(date >> 8);
    days = (char)(date >> 16);

// ============================================================ //
// fills the array with the date in ASCII. ==================== //

    char ofs = 0;
    char *dta = 0;

    for (char i = 0; i < 3; i++)
    {
        if (i == 0)
        {
            dta = &years;
            full_date[ofs] = '2';
            ofs++;
            full_date[ofs] = '0';
            ofs++;
        }
        if (i == 1)
        {
            dta = &months;
        }
        if (i == 2)
        {
            dta = &days;
        }


        itoab(*dta, num, 16);

        if (num[1] == 0)
        {
            full_date[ofs] = '0';
            ofs++;
            full_date[ofs] = num[0];
        }
        else
        {
            full_date[ofs] = num[0];
            ofs++;
            full_date[ofs] = num[1];
        }

        ofs++;

        if (i != 2)
        {
            full_date[ofs] = '.';
            ofs++;
        }
    }

    full_date[ofs] = '_';
    ofs++;

// ============================================================ //
// fills the array with the time in ASCII. ==================== //

    ofs = 11;
    dta = 0;

    for (char i = 0; i < 3; i++)
    {
        if (i == 0)
            dta = &hours;
        if (i == 1)
            dta = &minutes;
        if (i == 2)
            dta = &seconds;

        itoab(*dta, num, 16);

        if (num[1] == 0)
        {
            full_date[ofs] = '0';
            ofs++;
            full_date[ofs] = num[0];
        }
        else
        {
            full_date[ofs] = num[0];
            ofs++;
            full_date[ofs] = num[1];
        }

        ofs++;

        if (i < 2)
        {
            full_date[ofs] = '.';
        }
        //else
        //{
        //    full_date[ofs] = '_';
        //}

        ofs++;
    }

// ============================================================ //
// adding the name of the input file to the output path. ====== //

    int i = 0;
    int y = 0;

    // searching for a zero terminator in the input path.
    while (path_in[i] != 0)
    {
        i++;
    }

    // searching for a slash in the input path.
    while (path_in[i] != '/')
    {
        i--;
    }

    // searching for a zero terminator in the output path.
    while (path_out[y] != 0)
    {
        y++;
    }

    // searching for a slash in the output path.
    if (path_out[y - 1] == '/')
    {
        y--;
    }

    // copying the input name of the file into the output path,
    strcpy(&path_out[y], &path_in[i]);

// ============================================================ //
// adding the extension and full date to the path. ============ //

    i = 0;

    // searching for a zero terminator in the output path.
    while (path_out[i] != 0)
    {
        i++;
    }

    path_out[i] = '_';
    i++;

    // adding full date.
    strcpy(&path_out[i], full_date);

    i += strlen(full_date);

    // adding the extension to a path.
    strncpy(&path_out[i], ext, 4);

    //printf("Path_in: %s\n", path_in);
    //printf("Path_out: %s\n", path_out);

    data = openFile(&length, path_in);

    if(data == 0)
    {
    	if (con_init_console_dll()) return 1; // init fail.
        con_set_title("Backy");

        #if defined (lang_en)

            con_printf("\nThe file isn't found!\n");

        #elif defined (lang_ru)

            con_printf("\nФайл не найден!\n");

        #endif

        return 13;
    }

    return checkStateOnSave(saveFile(length, data, 0, path_out));
}

