#include <stdio.h>

int main()
{
    const   int     font_height     =   9;
            int     font_width[2]   =   {5,7},
                    char_num,i,e;
            char    ch,data;

    freopen("char.txt","rt",stdin);
    freopen("CHAR.MT","wb",stdout);

    for(char_num=256;char_num;char_num--)
    {
        fseek(stdin,8,SEEK_CUR);
        for(e=font_height;e;e--)
        {
            data    =   0;
            for(i=0;i<font_width[0];i++)
            {
                data    |=  getchar()==' '?0:1<<i;
            }
            fseek(stdin,3,SEEK_CUR);
            putchar(data);
        }
    }

    freopen("char2.txt","rt",stdin);
    freopen("CHAR2.MT","wb",stdout);

    for(char_num=256;char_num;char_num--)
    {
        fseek(stdin,6,SEEK_CUR);
        ch  =   getchar();
        putchar(ch==' '?0x08:ch-47);
        fseek(stdin,3,SEEK_CUR);
        for(e=font_height;e;e--)
        {
            data    =   0;
            for(i=0;i<font_width[1];i++)
            {
                data    |=  getchar()==' '?0:1<<i;
            }
            putchar(data);
            fseek(stdin,3,SEEK_CUR);
        }
    }
    return 0;
}