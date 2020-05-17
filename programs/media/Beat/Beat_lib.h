
/*
 * Beat_lib.h
 * Author: JohnXenox aka Aleksandr Igorevich.
 */

#ifndef __Beat_lib_h__
#define __Beat_lib_h__


void __attribute__ ((noinline)) printfOnADebugBoard(const char *format,...)
{
    va_list ap;
    char log_board[300];

    va_start (ap, format);
    tiny_vsnprintf(log_board, sizeof log_board, format, ap);
    va_end(ap);

    char *str = log_board;

    while(*str) __asm__ __volatile__("int $0x40"::"a"(63), "b"(1), "c"(*str++));
}


/*
int start_new_thread(void* proc, unsigned int* stack_top)
{
    register int val;
    __asm__ __volatile__("int $0x40":"=a"(val):"a"(51), "b"(1), "c"(proc), "d"(stack_top));
    return val;
}
*/


static inline int startApp(char *args, unsigned int enc, char  *path)
{
    int val;

    char dt[28];  // basic information structure.

    (int  ) dt[0]  = 7;       // subfunction number.
    (int  ) dt[4]  = 0;       // flags field.
    (char*) dt[8]  = args;    // 0 or pointer to ASCIIZ-string with parameters.
    (int  ) dt[12] = 0;       // (reserved).
    (int  ) dt[16] = 0;       // (reserved).
    (int  ) dt[20] = 1;       // string encoding (0 = default, 1 = cp866, 2 = UTF-16LE, 3 = UTF-8).
    (char*) dt[24] = path;    // pointer to the path to the file.

    __asm__ __volatile__("int $0x40":"=a"(val):"a"(80), "b"(&dt));

    return val;
}



static inline void makeDelay(unsigned int time)
{
    __asm__ __volatile__("int $0x40"::"a"(5), "b"(time):"memory");
}



static inline void getSystemColors(struct system_colors *color_table)
{
  __asm__ volatile ("int $0x40"::"a"(48),"b"(3),"c"(color_table),"d"(40));
}



static inline short getControlKeysOnAKeyboard(void)
{
    short val;
    __asm__ __volatile__("int $0x40":"=a"(val):"a"(66),"b"(3));
    return val;
}


static inline void showButton(int x, int y, int w, int h, unsigned int style, unsigned int id, unsigned int clr)
{
    w-=1;
    h-=1;
    __asm__ __volatile__("int $0x40"::"a"(8),"b"((x << 16) | w),"c"((y << 16) | h),"d"((style << 24) | id),"S"(clr));
}



static inline void deleteButton(unsigned int id)
{
    __asm__ __volatile__("int $0x40"::"a"(8),"d"(0x80000000 | id));
}



static inline void showNumber(int x, int y, unsigned int opt1, unsigned char opt2, unsigned int clr, unsigned int number)
{
    __asm__ __volatile__("int $0x40"::"a"(47),"b"(opt1),"c"(number),"d"((x << 16) | y),"S"(clr | ((int) opt2 << 24)));
}



/*
static inline void killThreadByTID(int tid)
{
    __asm__ __volatile__("int $0x40"::"a"(18), "b"(18), "c"(tid));
}
*/


static inline void showLine(int xs, int ys, int xe, int ye, unsigned int clr)
{
    __asm__ __volatile__("int $0x40"::"a"(38), "d"(clr),"b"((xs << 16) | xe),"c"((ys << 16) | ye));
}











// ============================================================================ //

void showRectangle(int x, int y, int w, int h, unsigned int clr)
{
    // top h line.
    showLine(x + 1, y, w + x - 2, y, clr);
    // bottom h line.
    showLine(x + 1, y + h - 1, w + x - 2, y + h - 1, clr);

    // left v line.
    showLine(x, y + 1, x, h + y - 2, clr);
    // right v line.
    showLine(x + w - 1, y + 1, x + w - 1, h + y - 2, clr);
}



static inline void setCurrentPathToARawFile(char *dst_path, char *src_path, char* file_name)
{
    unsigned offset = 0;

    // cleans a dst path if not clean.
    if(dst_path[offset] != 0)
    {
        for(; dst_path[offset] != 0; offset++) dst_path[offset] = 0;
    }

    // copys current path into a buffer.
    strcpy(dst_path, src_path);

    offset = 0;

    // goes to the end of a string.
    while(dst_path[offset] != 0) offset++;

    // clears all bytes to a character '/'.
    for(; dst_path[offset] != '/'; offset--) dst_path[offset] = 0;

    // increments a variable.
    offset++;

    // stores a name of a file in a buffer.
    strcpy(dst_path + offset, file_name);
}



static inline void showNamedButton(int x, int y, int w, int h, int style, int id, int clr, \
                                   char font_style, int tx, int ty, unsigned int text_clr, \
                                   unsigned int text_len, char* text)
{
//    w--;
//    h--;

    char chr_w = 8;
    char chr_h = 16;

    deleteButton(id);
    _ksys_make_button(x, y, (w - 1), (h - 1), (style | id), clr);
    _ksys_write_text((x + ((w / 2) - ((chr_w * text_len) / 2))), (y + 1 + ((h / 2) - (chr_h / 2))), \
                    (((int)font_style << 24) | text_clr), text, text_len);

}


static inline void StartButton(int x, int y, int w, int h, unsigned int clr, unsigned int text_clr, unsigned char state)
{
    #define BTN_ID 7

#if defined (lang_en)
    char* btn_name[] = {"Start", "Stop"};
#elif defined (lang_ru)
    char* btn_name[] = {"Старт", "Стоп"};
#endif

    showNamedButton(x, y, w, h, 0, BTN_ID, clr, 0b00010000, 2, 2, text_clr, strlen(btn_name[state]), btn_name[state]);

    if(state == 1) counter = 0;

    #undef BTN_ID
}



static inline void showPlusButton(int x, int y, int w, int h, unsigned int id, unsigned int clr, unsigned int text_clr)
{
    showNamedButton(x, y, w, h, 0, id, clr, 0b00010000, 2, 2, text_clr, 1, "+");
}



static inline void showMinusButton(int x, int y, int w, int h, unsigned int id, unsigned int clr, unsigned int text_clr)
{
    showNamedButton(x, y, w, h, 0, id, clr, 0b00010000, 2, 2, text_clr, 1, "-");
}

//===================================================================================//









void counterIndicator(int x, int y, int w, int h, unsigned int clr1, unsigned int clr2, unsigned char flag)
{
    #define BTN_ID 200

    unsigned int _clr1;
    unsigned int _clr2;

    char chr_w = 8 * 3;
    char chr_h = 16 * 3;

    unsigned char text_len = 1;

    deleteButton(BTN_ID);
    _ksys_make_button(x, y, w - 1, h - 1, (0x40000000 | BTN_ID), clr1);


    if(flag == 0)
    {
        _clr1 = clr1;
        _clr2 = clr2;
    }
    if(flag != 0)
    {
        _clr1 = clr2;
        _clr2 = clr1;
    }

    showRectangle(x, y, w, h, _clr2);
    _ksys_draw_bar((x + 1), (y + 1), (w - 2), (h - 2), _clr1);

    if(counter > 9) text_len = 2;

    showNumber((x + ((w / 2) - ((chr_w * text_len) / 2))), ((y + ((h / 2) - (chr_h / 2))) - 4), 0b10000000000000110000000000000000, 0b00010011, _clr2, counter);

    #undef BTN_ID
}





void meterIndicator(int x, int y, int w, int h, unsigned int clr1, unsigned int clr2, unsigned char meter, unsigned char *accentBeatFlags)
{
    #define SPACE 2

    #define BTN_ID 100

    char chr_w = 8;
    char chr_h = 16;

    int text_len = 1;

    unsigned int btn_clr = 0;
    unsigned int num_clr = 0;

    _ksys_draw_bar(10, y, w + 5, h, sc.work);

    // deletes all possible buttons.
    for(unsigned short i = 0; (i < 12); i++)
    {
        deleteButton(BTN_ID + i);
    }


    unsigned char num_of_spaces = 0;
    unsigned short spaces_sz = 0;

    unsigned int btn_x = x;
    unsigned int btn_w = w;



//===========================================//

    char spc = 0;

    if(meter > 1)
    {
        spc = 1;
        num_of_spaces = (meter - 1);
        spaces_sz = (num_of_spaces * SPACE);
    }

    btn_w = ((btn_w / meter) - spc);

//===========================================//








    // draws button(s).
    for(unsigned short i = 0; (i < meter); i++)
    {
        if((i + 1) > 9) text_len = 2;

        if(accentBeatFlags[i] == 0)
        {
            btn_clr = clr1;
            num_clr = clr2;
        }
        else
        {
            btn_clr = clr2;
            num_clr = clr1;
        }

        showButton(btn_x, y, btn_w, h, 0, (BTN_ID + i), btn_clr);
        //_ksys_make_button(btn_x, y, (btn_w - 1), (h - 1), (0 | (BTN_ID + i)), btn_clr);
         showNumber(((btn_x + ((btn_w / 2) - ((chr_w * text_len) / 2)))), (y + 2 + ((h / 2) - (chr_h / 2))), 0b10000000000000110000000000000000, 0b00010000, num_clr, (i + 1));

        btn_x += (btn_w + SPACE);
    }

    #undef SPACE
    #undef BTN_ID
}



void tempoBar1(int x, int y, int w, int h, unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned char sel)
{
    #define BTN_ID_MINUS 10
    #define BTN_ID_PLUS 11

    char chr_w = 8;
    char chr_h = 16;

    char* text = 0;
    int text_len = 0;

    char* tempoIt[] = {"Larghissimo", "Grave", "Lento", "Larghetto", "Adagio", "Andante", \
                       "Moderato", "Allegro", "Presto", "Prestissimo"};

    _ksys_draw_bar((x + 1), y, (w - 2), h, clr1);

    text = tempoIt[sel];
    text_len = strlen(tempoIt[sel]);

    _ksys_write_text((x + ((w / 2) - ((chr_w * text_len) / 2))), (y + 0 + ((h / 2) - (chr_h / 2))), (((int)0b00010000 << 24) | clr2), text, text_len);

    showMinusButton(x, y, h, h, BTN_ID_MINUS, clr3, clr2);
    showPlusButton(((x + w) - h), y, h, h, BTN_ID_PLUS, clr3, clr2);

    #undef BTN_ID_MINUS
    #undef BTN_ID_PLUS
}



void tempoBar2(int x, int y, int w, int h, unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned short tempo)
{
    #define BTN_ID_MINUS 12
    #define BTN_ID_PLUS 13

    char chr_w = 8;
    char chr_h = 16;

    int text_len = 0;

    if(tempo < 10)
    {
        text_len = 1;
    }
    else if((tempo > 9) && (tempo < 100))
    {
        text_len = 2;
    }
    else if((tempo > 99) && (tempo < 321))
    {
        text_len = 3;
    }

    _ksys_draw_bar((x + 1), y, (w - 2), h, clr1);

    showNumber((x + ((w / 2) - ((chr_w * text_len) / 2))), (y + ((h / 2) - (chr_h / 2))), 0b10000000000000110000000000000000, 0b00010000, clr2, tempo);

    showMinusButton(x, y, h, h, BTN_ID_MINUS, clr3, clr2);
    showPlusButton(((x + w) - h), y, h, h, BTN_ID_PLUS, clr3, clr2);

    #undef BTN_ID_MINUS
    #undef BTN_ID_PLUS
}



void meterBar(int x, int y, int w, int h, unsigned int clr1, unsigned int clr2, unsigned int clr3, unsigned char meter, unsigned int divider)
{
    #define BTN_ID_MINUS 14
    #define BTN_ID_PLUS 15

    char chr_w = 8;
    char chr_h = 16;

    int text_len = 3;

    if(meter > 9) text_len = 4;

    _ksys_draw_bar((x + 1), y, (w - 2), h, clr1);

    unsigned int num_x = ((x + ((w / 2) - ((chr_w * text_len) / 2))));
    unsigned int num_y = (y + ((h / 2) - (chr_h / 2)));

    showNumber(num_x, num_y, 0b10000000000000110000000000000000, 0b00010000, clr2, meter);
    _ksys_write_text((num_x + (chr_w * (text_len - 2))), num_y, (((int)0b00010000 << 24) | clr2), "/", 1);
    showNumber((num_x + (chr_w * (text_len - 1))), num_y, 0b10000000000000110000000000000000, 0b00010000, clr2, 4);

    showMinusButton(x, y, h, h, BTN_ID_MINUS, clr3, clr2);
    showPlusButton(((x + w) - h), y, h, h, BTN_ID_PLUS, clr3, clr2);

    #undef BTN_ID_MINUS
    #undef BTN_ID_PLUS
}



//===================================================================//

void showCounterIndicator()
{
    counterIndicator(10, (skin_height + 5), 310, 60, sc.work_button, sc.work_button_text, counterIndicatorFlag);
}



void showMeterIndicator()
{
    meterIndicator(10, (skin_height + 70), 310, 21, sc.work_button, sc.work_button_text, meter, accentBeatFlags);
}





void showTempoBar1(char tempoSelector)
{
    tempoBar1(10, (skin_height + 96), 140, 20, sc.work_graph, sc.work_button_text, sc.work_button, tempoSelector);
}



void showTempoBar2(short tempo)
{
    tempoBar2((10 + 145), (skin_height + 96), 76, 20, sc.work_graph, sc.work_button_text, sc.work_button, tempo);
}



void showMeterBar(char meter)
{
    meterBar((10 + 140 + 5 + 76 + 5), (skin_height + 96), 84, 20, sc.work_graph, sc.work_button_text, sc.work_button, meter, 4);
}



void showStartButton(void)
{
    StartButton(10, (skin_height + 121), 310, 25, sc.work_button, sc.work_button_text, startButtonBit);
}

//================================================================================//








void setTempoByTempoSelector(unsigned short *tempo, unsigned char tempoSelector)
{
    enum tempoSelectors
    {
        LARGHISSIMO = 0,
        GRAVE,
        LENTO,
        LARGHETTO,
        ADAGIO,
        ANDANTE,
        MODERATO,
        ALLEGRO,
        PRESTO,
        PRESTISSIMO
    };

    // switches tempo (beats per minet).
    switch(tempoSelector)
    {
        case LARGHISSIMO: *tempo = 20;  // 1 - 25 bpm.
        break;

        case GRAVE: *tempo = (46 - ((46 - 25) / 2));  // 25 - 46 bpm.  calculates an average value.
        break;

        case LENTO: *tempo = (61 - ((61 - 46) / 2));  // 46 - 61 bpm
        break;

        case LARGHETTO: *tempo = (67 - ((67 - 61) / 2));
        break;

        case ADAGIO: *tempo = (77 - ((77 - 67) / 2));
        break;

        case ANDANTE: *tempo = (109 - ((109 - 77) / 2));
        break;

        case MODERATO: *tempo = (121 - ((121 - 109) / 2));
        break;

        case ALLEGRO: *tempo = (169 - ((169 - 121) / 2));
        break;

        case PRESTO: *tempo = (201 - ((201 - 169) / 2));
        break;

        case PRESTISSIMO: *tempo = (320 - ((320 - 201) / 2));
        break;
    }
}





void setTempoSelectorByTempo(unsigned short *tempo, unsigned char *tempoSelector)
{
    enum tempoSelectors
    {
        LARGHISSIMO = 0,
        GRAVE,
        LENTO,
        LARGHETTO,
        ADAGIO,
        ANDANTE,
        MODERATO,
        ALLEGRO,
        PRESTO,
        PRESTISSIMO
    };

    // switches a selector.
    if((*tempo > 0) && (*tempo < 25))
    {
        if(*tempoSelector != LARGHISSIMO)
        {
            *tempoSelector = LARGHISSIMO;
            showTempoBar1(*tempoSelector);
        }
    }
    else if((*tempo >= 25) && (*tempo < 46))
    {
        if(*tempoSelector != GRAVE)
        {
            *tempoSelector = GRAVE;
            showTempoBar1(*tempoSelector);
        }
    }
    else if((*tempo >= 46) && (*tempo < 61))
    {
        if(*tempoSelector != LENTO)
        {
            *tempoSelector = LENTO;
            showTempoBar1(*tempoSelector);
        }
    }
    else if((*tempo >= 61) && (*tempo < 67))
    {
        if(*tempoSelector != LARGHETTO)
        {
            *tempoSelector = LARGHETTO;
            showTempoBar1(*tempoSelector);
        }
    }
    else if((*tempo >= 67) && (*tempo < 77))
    {
        if(*tempoSelector != ADAGIO)
        {
            *tempoSelector = ADAGIO;
            showTempoBar1(*tempoSelector);
        }
    }
    else if((*tempo >= 77) && (*tempo < 109))
    {
        if(*tempoSelector != ANDANTE)
        {
            *tempoSelector = ANDANTE;
            showTempoBar1(*tempoSelector);
        }
    }
    else if((*tempo >= 109) && (*tempo < 121))
    {
        if(*tempoSelector != MODERATO)
        {
            *tempoSelector = MODERATO;
            showTempoBar1(*tempoSelector);
        }
    }
    else if((*tempo >= 121) && (*tempo < 169))
    {
        if(*tempoSelector != ALLEGRO)
        {
            *tempoSelector = ALLEGRO;
            showTempoBar1(*tempoSelector);
        }
    }
    else if((*tempo >= 169) && (*tempo < 201))
    {
        if(*tempoSelector != PRESTO)
        {
            *tempoSelector = PRESTO;
            showTempoBar1(*tempoSelector);
        }
    }
    else if((*tempo >= 201) && (*tempo < 321))
    {
        if(*tempoSelector != PRESTISSIMO)
        {
            *tempoSelector = PRESTISSIMO;
            showTempoBar1(*tempoSelector);
        }
    }
}


void drawWindow()
{
        //if(counter > 1) counter = 1;

        getSystemColors(&sc);
        skin_height = _ksys_get_skin_height();

        _ksys_window_redraw(1);
        _ksys_draw_window(10, 10, 330, 200, sc.work, 0x14, 0x5080d0, 0, (int)header);
        _ksys_window_redraw(2);


        showCounterIndicator();

        showMeterIndicator();

        setTempoSelectorByTempo(&tempo, &tempoSelector);

        showTempoBar1(tempoSelector);
        showTempoBar2(tempo);
        showMeterBar(meter);
        showStartButton();

        //int a_x = ((x + ((w / 2) - ((chr_w * text_len) / 2))) - num_offset);

        #if defined (lang_en)
            _ksys_write_text(94, (skin_height + 153), (((int)0b10010000 << 24) | sc.work_text), "Author: JohnXenox", 0);
        #elif defined (lang_ru)
            _ksys_write_text(94, (skin_height + 153), (((int)0b10010000 << 24) | sc.work_text), "Автор: JohnXenox", 0);
        #endif



//showRectangle(10, (skin_height + 5), 315, 50, 0xff0000);

}



#endif



































