//===================================================//
//                                                   //
//                   TRANSLATIONS                    //
//                                                   //
//===================================================//

#ifdef LANG_RUS

#define INTRO_TEXT "Это простой просмотрщик текста.\nПопробуйте открыть какой-нибудь текстовый файл."
#define VERSION "Text Reader v1.41"
#define ABOUT "Идея: Leency, punk_joker
Код: Leency, Veliant, KolibriOS Team

Горячие клавиши:
Ctrl+O - открыть файл
Ctrl+I - показать информацию о файле
Ctrl+Плюс - увеличить шрифт
Ctrl+Минус - уменьшить шрифт
Ctrl+Tab - выбрать кодировку текста
Ctrl+E - открыть файл в другой программе

Поиск:
Ctrl+F - открыть панель поиска
F3 - искать далее
Esc - скрыть панель поиска
 
Нажмите любую клавишу..."

char color_scheme_names[] =
"Черный на белом
Черный на сером   |RtfRead
Черный на льне    |Horst  
Черный на холсте  |Pocket 
Черный на желтом  |Fb2Read
Серый на темном1  |Godot  
Серый на темном2  |Monokai";

#else

#define INTRO_TEXT "This is a plain Text Reader.\nTry to open some text file."
#define VERSION "Text Reader v1.41"
#define ABOUT "Idea: Leency, punk_joker
Code: Leency, Veliant, KolibriOS Team

Hotkeys:
Ctrl+O - open file
Ctrl+I - show file properties
Ctrl+Up - bigger font
Ctrl+Down - smaller font
Ctrl+Tab - select charset
Ctrl+E - reopen current file in another app

Search:
Ctrl+F - open search
F3 - search next
Esc - hide search bar
 
Press any key..."

char color_scheme_names[] =
"Black & White
Black & Grey    |RtfRead
Black & Linen   |Horst  
Black & Antique |Pocket 
Black & Lemon   |Fb2Read
Grey & DarkGrey |Godot  
Grey & DarkGrey |Monokai";

#endif

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

dword color_schemes[] = {
0xFFFfff, 0,
0xF0F0F0, 0,
0xFDF6E3, 0x101A21,
0xFCF0DA, 0x171501,
0xF0F0C7, 0,
0x282C34, 0xABB2BF,
0x282923, 0xD8D8D2
};

char default_dir[] = "/rd/1";
od_filter filter2 = { 8, "TXT\0\0" };