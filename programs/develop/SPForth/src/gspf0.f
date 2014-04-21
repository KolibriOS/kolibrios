\ TSET-OPT
 0x10 TOMM_SIZE
HERE TO TSAVE_LIMIT
USER-HERE-SET TO RESERVE
USER-HERE-SET USER-OFFS !
\ DECIMAL
\ TRUE WARNING ! \ чтобы было
\ ==============================================================
\ Начало образа Форт-системы
\ S" src/USER.F"                INCLUDED
\ S" src/spf_defkern.f"                INCLUDED
CR .( S" src/spf_forthproc.f"              INCLUDED)

S" src/spf_forthproc.f"              INCLUDED

\ S" src/spf_floatkern.f"              INCLUDED

S" src/spf_forthproc_hl.f"           INCLUDED


S" src/kol/spf_kol_const.f"          INCLUDED 

\ ==============================================================

S" src/kol\spf_kol_sys.f"	INCLUDED

\ Трансляция исходных текстов.
\ Обработка ошибок.
\ Определяющие слова.
\ Числовые литералы.
\ Управление компиляцией.
\ Компиляция управляющих структур.

S" src/compiler/spf_immed_lit.f"     INCLUDED
S" src/compiler/spf_defwords.f"      INCLUDED

S" src/compiler/spf_immed_loop.f"    INCLUDED

S" src/compiler/spf_error.f"         INCLUDED
 TDIS-OPT
S" src/compiler/spf_translate.f"     INCLUDED
S" src/compiler/spf_immed_transl.f"  INCLUDED
S" src/compiler/spf_literal.f"       INCLUDED
S" src/compiler/spf_immed_control.f" INCLUDED

\ ==============================================================
\ Компиляция чисел и строк в словарь.
\ Создание словарных статей.
\ Поиск слов в словарях.
\ Печать словарей.
\ S" src/temps4_.f"                    INCLUDED
\ EOF
S" src/compiler/spf_wordlist.f"      INCLUDED
S" src/compiler/spf_find.f"          INCLUDED
S" src/compiler/spf_words.f"         INCLUDED
S" src/compiler/spf_compile0.f"       INCLUDED
S" ~mak/~af/lib/c/zstr.f"       INCLUDED
' ALITERAL TO ALITERAL-CODE

\ Структурированная обработка исключений (см.также init)

S" src/spf_except.f"                 INCLUDED

\ ==============================================================
\ Печать чисел

S" src/spf_print.f"                  INCLUDED
S" src/kol\spf_kol_module.f"         INCLUDED

\ ==============================================================
\ Файловый и консольный ввод-вывод (kol-зависимые)

S" src/kol/spf_kol_con_io.f"         INCLUDED
S" src/spf_con_io.f"                 INCLUDED
S" src/kol/spf_kol_io.f"             INCLUDED
\ ==============================================================
\ Управление памятью

S" src/kol/spf_kol_memory.f"         INCLUDED

\ ==============================================================
\  Макроподстановщик-оптимизатор
TRUE TO INLINEVAR
CR .( S" src/macroopt.f"                   INCLUDED)
MM_SIZE DUP H. 
0x10 TOMM_SIZE
S" src/macroopt.f"                   INCLUDED
DUP H. TOMM_SIZE 
S" src/compiler/spf_compile.f"       INCLUDED

\ ==============================================================
\ Парсер исходного текста форт-программ
S" src/compiler/spf_parser.f"        INCLUDED
S" src/compiler/spf_read_source.f"   INCLUDED

\ ==============================================================
\ Инициализация переменных, startup
CR .( S" src/spf_init.f"              INCLUDED)
S" src/spf_init.f"              INCLUDED

\ ==============================================================
S" src/compiler/spf_modules.f"       INCLUDED
 S" src/MEFORTH.F"           INCLUDED 
S" src/spf_last.f"              INCLUDED
\ S" src/tst.f"           INCLUDED \EOF

\ EOF

CR .( =============================================================)
CR .( Done.
CR .( =============================================================)
MM_SIZE H.