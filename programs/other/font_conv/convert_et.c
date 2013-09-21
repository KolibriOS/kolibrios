/*
    Copyright 2011 dunkaist <dunkaist@gmail.com>
    Distributed under the terms of the GNU General Public License v3.
    See http://www.gnu.org/licenses/gpl.txt for the full license text.
*/


#include <stdio.h>

#define	FONT_HEIGHT	    9
#define FONT_WIDTH_MONO	    5
#define FONT_WIDTH_VAR	    7   /* max symbol width */

    short int   char_num, row, col;
    char        ch, data;


int do_symbol(short int font_width)
{
    for(row=FONT_HEIGHT; row; row--)
    {
        data    =   0;
        for(col=0; col<font_width; col++)
        {
            data    |=  getchar()==' '? 0 : 1<<col;
        }
        putchar(data);
        fseek(stdin, 3, SEEK_CUR);
    }
    return 0;
}


int main()
{
    freopen("char_et.txt", "rt", stdin);
    freopen("char_et.mt", "wb", stdout);

    for(char_num=256; char_num; char_num--)
    {
        fseek(stdin, 8, SEEK_CUR);
        do_symbol(FONT_WIDTH_MONO);
    }

    freopen("char2_et.txt", "rt", stdin);
    freopen("char2_et.mt", "wb", stdout);

    for(char_num=256; char_num; char_num--)
    {
        fseek(stdin, 6, SEEK_CUR);
        ch  =   getchar();
        putchar(ch==' '? 0x08 : ch-47);
        fseek(stdin, 3, SEEK_CUR);
        do_symbol(FONT_WIDTH_VAR);
    }
    return 0;
}
