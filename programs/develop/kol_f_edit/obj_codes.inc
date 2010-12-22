;
; в этом файле собраны коды объектов
;

C_TDF equ 'Type' ;typedef
C_ACO equ 'Auto' ;автоматические коды (auto code)
C_STR equ ('s' shl 24) ;struct
C_INC equ ('#' shl 24) ;include
C_TXT equ 'Code' ;блок кода (данных)
C_VAR equ ('-' shl 24) ;переменная
C_WND equ ('w' shl 24) ;окно
C_CHB equ 'ChkB' ;CheckBox
C_OPT equ ('o' shl 24) ;OptionBox
C_EDB equ ('e' shl 24) ;EditBox
C_BUT equ ('b' shl 24) ;Button
C_DTT equ 'Text' ;рисовать текст
C_REC equ 'Rect' ;рисовать прямоугольник
C_KEY equ ('k' shl 24) ;клавиатура
C_LIB equ 'Libr' ;библиотека
C_IFN equ ('f' shl 24) ;библиотечная подключаемая функция

MAX_LEN_OBJ_TXT equ 200 ;максимальная длинна текстовых к свойств объекта
MAX_OPT_CAPTION equ 250 ;максимальная длинна подписей к свойствам объекта