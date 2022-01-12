/* Copyright (C) 2019-2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <sys/ksys.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <cryptal/md5.h>
#include <cryptal/sha1.h>
#include <cryptal/sha256.h>
#include <clayer/dialog.h>
#include <clayer/boxlib.h>

#define TRUE 1;
#define FALSE 0;
#define MAX_HASH_LEN 65 // Максимальная длина строки
#define WINDOW_W 665
#define VERSION "%s - thashview 2.6.2"
#define EDIT_TEXT_SIZE 0x10000000
#define DATA(type, addr, offset) *((type*)((uint8_t*)addr+offset))

ksys_colors_table_t sys_color_table;

char hash_str_md5[MAX_HASH_LEN]=   "Click the 'MD5:' button to show the md5-checksum!"; //Вывод MD5
char hash_str_sha1[MAX_HASH_LEN]=  "Click the 'SHA1:' button to show the sha1-checksum!"; //Вывод SHA1
char hash_str_sha256[MAX_HASH_LEN]="Click the 'SHA256:' button to show the sha256-checksum!"; //Вывод SHA256
char edit_box_buff[MAX_HASH_LEN]; // Буффер для ввода
char *filename; // Имя обрабатываемого файла
char *title; // Заголовок окна

enum MYCOLORS // Цвета
{
    GREEN = 0x067D06 | EDIT_TEXT_SIZE,
    RED   = 0xFF0000 | EDIT_TEXT_SIZE,
    BLACK = 0x000000 | EDIT_TEXT_SIZE,
    WHITE = 0xFFFFFF,
    GREY  = 0x919191
};

edit_box hash_edit_box={WINDOW_W-140,10,121,WHITE,0,0,GREY,EDIT_TEXT_SIZE, MAX_HASH_LEN-1, edit_box_buff,NULL,ed_focus}; // Создаём структуру edit_box
int md5_flag=0, sha1_flag=0, sha256_flag=0; // Флаги показывающие была ли уже рассчитана котрольная сумма в функции check_sum()

enum BUTTONS // Кнопки в интрефейсе
{
    BTN_QUIT=1,        //Выход
    BTN_MD5 = 10,      //Рассчитать md5-контрольную сумму
    BTN_SHA1 = 20,     //Рассчитать sha1-контрольную сумму
    BTN_SHA256 = 30,   //Рассчитать sha256-контрольную сумму
    BTN_COPY_MD5= 11,  //Скопировать в буффер обмена
    BTN_COPY_SHA1= 21,
    BTN_COPY_SHA256=31,
    BTN_CMP=40,        //Сравнить edit_box и контрольную сумму
    BTN_PASTE=50       //Вставить в edit_box(пока в разработке)
};

void notify_show(char *text)
{
   _ksys_exec("/sys/@notify", text);
}

void* safe_malloc(size_t size) // Безопасный malloc. Показывает уведомление об ошибке и закрывает программу если память не была выделена
{
    void *p=malloc(size);
    if(p==NULL){
       notify_show("'Memory allocation error!' -E");
       exit(0);
    }else{
        return p;
    }
}

void global_var_init(unsigned int size)  // Инициализируются глобальные массивы
{
  filename=safe_malloc(size);
  title=safe_malloc(size+20);
}

/* Функции генерации контрольных сумм */
void md5_hash(FILE* input, BYTE* hash )
{
    int input_size;
    BYTE *temp_buffer;
    temp_buffer=safe_malloc(1024);
    MD5_CTX ctx;
    md5_init(&ctx);
    while((input_size = fread(temp_buffer, 1, 1024, input)) > 0){
                md5_update(&ctx, temp_buffer, input_size);
    }
    md5_final(&ctx, hash);
    free(temp_buffer);
}

void sha1_hash(FILE* input, BYTE* hash )
{
    int input_size;
    BYTE *buffer;
    buffer=safe_malloc(1024);
    SHA1_CTX ctx;
    sha1_init(&ctx);
    while((input_size = fread(buffer, 1, 1024, input)) > 0){
                sha1_update(&ctx, buffer, input_size);
    }
    sha1_final(&ctx, hash);
    free(buffer);
}

void sha256_hash(FILE* input, BYTE* hash )
{
    int input_size;
    BYTE *buffer;
    buffer=safe_malloc(1024);
    SHA256_CTX ctx;
    sha256_init(&ctx);
    while((input_size = fread(buffer, 1, 1024, input)) > 0){
                sha256_update(&ctx, buffer, input_size);
    }
    sha256_final(&ctx, hash);
    free(buffer);
}


BYTE* check_sum(int alg) // Генерируем контрольные суммы используя один из алгоритмов
{
    FILE* input_file;
    BYTE *hash;
    input_file=fopen(filename,"rb");
    hash = safe_malloc(alg);
    switch (alg)
    {
        case MD5_BLOCK_SIZE :
            md5_hash(input_file, hash);
            md5_flag=1;
        break;

        case SHA1_BLOCK_SIZE :
            sha1_hash(input_file, hash);
            sha1_flag=1;
        break;

        case SHA256_BLOCK_SIZE :
            sha256_hash(input_file, hash);
            sha256_flag=1;
        break;
    }
    fclose(input_file);
    return hash;
}

void sprint_hash(BYTE *hash, char* hash_str, int hash_size) //Преобрауем двоичные данные из hash в строку hash_str
{
    char block[3];
    memset(hash_str, 0, MAX_HASH_LEN); // Очищаем строку для strcat
    for(int i=0; i<hash_size; i++)
    {
        sprintf(block,"%02x", hash[i]);
        strcat(hash_str,block);
    }
    free(hash);
}

void redraw_window() //Рисуем окно
{
    sprintf(title,VERSION, filename); // Устанавливаем заголовок окна
    ksys_pos_t win_pos = _ksys_get_mouse_pos(KSYS_MOUSE_SCREEN_POS); // Получаем координаты курсора
    _ksys_start_draw(); //Начинаем рисование интерфейса )
    _ksys_create_window(win_pos.x, win_pos.y, WINDOW_W, 150, title, sys_color_table.work_area, 0x14); // Создаём окно.
    edit_box_draw(&hash_edit_box); // Рисуем edit_box

    _ksys_define_button(10, 30, 60, 20, BTN_MD5, GREEN); // Определяем кнопку md5
    _ksys_define_button(10, 60, 60, 20, BTN_SHA1, GREEN);// Определяем кнопку sha1
    _ksys_define_button(10, 90, 60, 20, BTN_SHA256, GREEN);// Определяем кнопку sha256

    _ksys_draw_text("MD5:", 15, 34, 0,   0x90000000 | sys_color_table.work_button_text); // Пищем текст на кнопках
    _ksys_draw_text("SHA1:", 15, 64, 0,  0x90000000 | sys_color_table.work_button_text);
    _ksys_draw_text("SHA256:", 15,94, 0, 0x90000000 | sys_color_table.work_button_text);

    _ksys_draw_text(hash_str_md5, 80, 34, 0, 0x90000000 | sys_color_table.work_text); // Выводим контрольные суммы в окно
    _ksys_draw_text(hash_str_sha1, 80, 64, 0, 0x90000000 | sys_color_table.work_text);
    _ksys_draw_text(hash_str_sha256, 80, 94, 0, 0x90000000| sys_color_table.work_text);

    
    _ksys_define_button(610, 30, 42, 20, BTN_COPY_MD5, sys_color_table.work_button); // Определяем кнопки для копирования
    _ksys_define_button(610, 60, 42, 20, BTN_COPY_SHA1, sys_color_table.work_button);
    _ksys_define_button(610, 90, 42, 20, BTN_COPY_SHA256, sys_color_table.work_button);

    _ksys_draw_text("Copy", 615, 34, 0,   0x90000000 | sys_color_table.work_button_text); // Пишем copy на всех кнопках для копирования
    _ksys_draw_text("Copy", 615, 64, 0,  0x90000000 | sys_color_table.work_button_text);
    _ksys_draw_text("Copy", 615, 94, 0, 0x90000000 | sys_color_table.work_button_text);

    _ksys_define_button(592, 120, 60, 20, BTN_CMP, GREEN); // Определяем кнопку для сравнения контольных сумм
    _ksys_draw_text("Compare", 595, 124 , 0,0x90000000 | sys_color_table.work_button_text); // Пишем текс на кнопке.
    _ksys_define_button(540, 120, 45, 20, BTN_PASTE, sys_color_table.work_button); //Кнопка для вставки (неработает)
    _ksys_draw_text("Paste", 543, 124 , 0,0x90000000 | sys_color_table.work_button_text); // Текст paste на кнопке
    _ksys_end_draw();
}

void paste_to_edit_buffer()    // Вставить из буффера обмена
{
    char *temp_buff=NULL;
    if(_ksys_clip_num()>0){
        temp_buff=_ksys_clip_get(_ksys_clip_num()-1);
        memset(edit_box_buff,0,MAX_HASH_LEN);
        if(DATA(int, temp_buff,0)>0 && DATA(int,temp_buff,4)==KSYS_CLIP_TEXT && DATA(int,temp_buff,8)==KSYS_CLIP_CP866){
            strncpy(edit_box_buff,temp_buff+12, MAX_HASH_LEN-1);
            edit_box_set_text(&hash_edit_box,edit_box_buff);
            notify_show("'Pasted from clipboard!' -I");
            hash_edit_box.text_color = BLACK;
            free(temp_buff);
        }
    }
}

void copy_to_clipboard(char *text) // Копировать в буфер обмена
{
    int text_hash_len = strlen(text)/2;
    if(text_hash_len==MD5_BLOCK_SIZE  ||
       text_hash_len==SHA1_BLOCK_SIZE || 
       text_hash_len==SHA256_BLOCK_SIZE) // Если текст является хэш-строкой
    {
        char *temp_buffer=safe_malloc(MAX_HASH_LEN+12); // Выделяем память для времнного буфера
        memset(temp_buffer, 0, MAX_HASH_LEN);   // Зануляем буфер
        DATA(char,temp_buffer,4)=KSYS_CLIP_TEXT;     // Устанавливаем TEXT для буфера(Смещение 4 байта)
        DATA(char,temp_buffer,8)=KSYS_CLIP_CP866;    // Устанавливаем кодировку CP866(Смещение 8 байт)
        strncpy(temp_buffer+12, text, MAX_HASH_LEN-1); // Копируем данные из text во временный буфер(Смещение 12 байт)
        _ksys_clip_set(strlen(text)+12, temp_buffer); // Выполняем системный вызов и перемещаем данные из временного буфера в буфер обмена
        notify_show("'Copied to clipboard!' -I");   
        free(temp_buffer); // Освобожаем временный буфер.
    }
}

void print_pending_calc(char *str) // Выводим сообщение о том что контрольная суммма вычисляется.
{
  strcpy(str, "Please wait! Calculating checksum...");
  redraw_window();
}

bool calc_and_cmp(char *hash_str_universal,int alg) // Вычисляем контрольную сумму и сравниваем с edit_box_buff.
{
   print_pending_calc(hash_str_universal);
   sprint_hash(check_sum(alg),hash_str_universal, alg);
   return !strcmp(edit_box_buff, hash_str_universal);
}

bool hash_compare() // Главная функция для сравнения
{
   int alg=strlen(edit_box_buff)/2;
   switch (alg){ // Если вычисления ещё небыло
        case MD5_BLOCK_SIZE:
            if(md5_flag){
                return !strcmp(edit_box_buff,hash_str_md5);
            }else{
                return calc_and_cmp(hash_str_md5,alg);
            }
            break;
        case SHA1_BLOCK_SIZE:
            if(sha1_flag){
                return !strcmp(edit_box_buff,hash_str_sha1);
            }else{
                return calc_and_cmp(hash_str_sha1,alg);
            }
            break;
        case SHA256_BLOCK_SIZE:
            if(sha256_flag){
                return !strcmp(edit_box_buff,hash_str_sha256);
            }else{
                return calc_and_cmp(hash_str_sha256,alg);
            }
            break;
        default:
            return FALSE;
            break;
    }
}

int main(int argc, char** argv)
{
    if(argc<2){ // Если аргументов нет, то запускаем диалог выбора фа
        open_dialog* dialog = kolibri_new_open_dialog(OPEN,0, 0, 420, 320);
        OpenDialog_init(dialog);
        OpenDialog_start(dialog); 
        if(dialog->status==SUCCESS){ // Если файл выбран
            global_var_init(strlen(dialog->openfile_path));
            strcpy(filename, dialog->openfile_path);  
        }else{ // Если файл не выбран
            notify_show("'No file selected!' -E");
            exit(0);
        }
        free(dialog);
    }else{
        global_var_init(strlen(argv[1]));
        strcpy(filename, argv[1]);
    }

    if(NULL==fopen(filename,"rb")){ // Если файла нет или не открывается
        notify_show("'File not found!' -E");
        exit(0);
    }
    if(_ksys_screen_size().x<WINDOW_W){
        notify_show("'Low screen resolution! Program will not display correctrly!' -W");
    }

    int gui_event; // Переменная для хранения события
    uint32_t pressed_button = 0; // Код нажатой кнопки в окне

    _ksys_get_system_colors(&sys_color_table);
    hash_edit_box.shift_color=sys_color_table.work_button;

    _ksys_set_event_mask(0xC0000027);// Устанавливаем маску событий
    do // Цикл обработки событий
    {
        gui_event = _ksys_get_event(); // Получаем событие
        switch(gui_event){ // Обрабатываем события
            case KSYS_EVENT_NONE:
                break;
            case KSYS_EVENT_REDRAW:
                redraw_window();
                break;
            case KSYS_EVENT_MOUSE:
                edit_box_mouse(&hash_edit_box);
                break;        
            case KSYS_EVENT_KEY:
                hash_edit_box.text_color = BLACK;
                edit_box_key_safe(&hash_edit_box, _ksys_get_key());
                break;
            case KSYS_EVENT_BUTTON: // Событие обработки кнопок
                pressed_button = _ksys_get_button(); // Получение кода нажатой кнопки.
                switch (pressed_button){ // Проверка какая кнопка была нажата
                    case BTN_MD5:
                        print_pending_calc(hash_str_md5);
                        sprint_hash(check_sum(MD5_BLOCK_SIZE),hash_str_md5, MD5_BLOCK_SIZE);
                        break;
                    case BTN_SHA1:
                        print_pending_calc(hash_str_sha1);
                        sprint_hash(check_sum(SHA1_BLOCK_SIZE),hash_str_sha1, SHA1_BLOCK_SIZE);
                        break;
                    case BTN_SHA256:
                        print_pending_calc(hash_str_sha256);
                        sprint_hash(check_sum(SHA256_BLOCK_SIZE),hash_str_sha256, SHA256_BLOCK_SIZE);
                        break;
                    case BTN_COPY_MD5:
                        copy_to_clipboard(hash_str_md5);
                        break;
                    case BTN_COPY_SHA1:
                        copy_to_clipboard(hash_str_sha1);
                        break;
                    case BTN_COPY_SHA256:
                        copy_to_clipboard(hash_str_sha256);
                        break;
                    case BTN_PASTE:
                        paste_to_edit_buffer();
                        break;
                    case BTN_CMP:
                        if(hash_compare()){
                            notify_show("'The checksum matches :)' -OK");
                            hash_edit_box.text_color = GREEN; // Устанавливаем текст ввода зелёным если контрольная сумма совпадает
                        }else{
                            notify_show("'The checksum does not match! :(' -W");
                            hash_edit_box.text_color = RED; // Устанавливаем текст ввода красным если контрольная суммы не совпадает
                        }
                        break;
                    case BTN_QUIT:
                        exit(0);
                        break;
                }
                redraw_window();
                break; 
        }
    }while(1);
    return 0;
}
