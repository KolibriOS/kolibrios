; SPDX-License-Identifier: NOASSERTION
;
; The settings of SETUP: read them from the kernel or from system.ini,
; apply them to the kernel and save them to system.ini

; ====================================================================

; take the current values from the kernel, the mouse ones at system
; start only
settings_read_boot:
        mcall   SF_SYSTEM, SSF_MOUSE_SETTINGS, SSSF_GET_SPEED
        mov     [mouseSpeed], eax
        mcall   SF_SYSTEM, SSF_MOUSE_SETTINGS, SSSF_GET_SPEEDUP
        mov     [mouseAccel], eax
        mcall   SF_SYSTEM, SSF_MOUSE_SETTINGS, SSSF_GET_DOUBLE_CLICK_DELAY
        mov     [mouseDelay], eax

; 26.5, 26.11 and 26.12 return what 21.5, 21.11 and 21.12 set
settings_read_system:
        mcall   SF_SYSTEM_GET, 5
        dec     eax                     ; the kernel counts languages from 1
        cmp     eax, LANGUAGES
        jb      .language
        xor     eax, eax

.language:
        mov     [language], eax
        mcall   SF_SYSTEM_GET, 11
        mov     [lba], eax
        mcall   SF_SYSTEM_GET, 12
        mov     [pci], eax
        mcall   SF_SYSTEM, SSF_SPEAKER, SSSF_GET_STATE
        mov     [speakerMute], eax
        mcall   SF_STYLE_SETTINGS, SSF_GET_FONT_SMOOTH
        mov     [fontSmoothing], eax
        mcall   SF_STYLE_SETTINGS, SSF_GET_FONT_SIZE
        mov     [fontHeight], eax

; keep the font height within what the window shows and steps through
font_height_limits:
        mov     eax, [fontHeight]
        cmp     eax, FONT_H_MIN
        jge     .min
        mov     eax, FONT_H_MIN

.min:
        cmp     eax, FONT_H_MAX
        jle     .max
        mov     eax, FONT_H_MAX

.max:
        mov     [fontHeight], eax
        ret

; ====================================================================

; override the values with the ones stored in system.ini; a missing or
; unknown value keeps the current one
settings_read_ini:
        mov     esi, iniSettings

.next:
        mov     edi, [esi + INI_SETTING.words]
        test    edi, edi
        jnz     .word
        mov     eax, [esi + INI_SETTING.variable]
        invoke  ini.get_int, sz_ini, [esi + INI_SETTING.ini_section], \
                [esi + INI_SETTING.key], [eax]
        jmp     .store

.word:
        and     [iniWord], 0
        invoke  ini.get_str, sz_ini, [esi + INI_SETTING.ini_section], \
                [esi + INI_SETTING.key], iniWord, 4, 0
        mov     eax, [iniWord]
        or      eax, 0x20202020         ; lowercase, zeroes become spaces
        xor     edx, edx

.find:
        cmp     eax, [edi + 4 + edx*4]
        je      .found
        inc     edx
        cmp     edx, [edi]
        jb      .find
        jmp     .skip

.found:
        mov     eax, edx

.store:
        mov     edx, [esi + INI_SETTING.variable]
        mov     [edx], eax

.skip:
        add     esi, sizeof.INI_SETTING
        cmp     esi, iniSettings.end
        jb      .next
        jmp     font_height_limits

; ====================================================================

; store the values to system.ini, all but the mouse ones; not ebx for
; the loop: ini_set_str returns with ebx = 13
settings_save_ini:
        mov     esi, iniSettings

.next:
        mov     eax, [esi + INI_SETTING.variable]
        mov     eax, [eax]
        mov     edi, [esi + INI_SETTING.words]
        test    edi, edi
        jnz     .word
        invoke  ini.set_int, sz_ini, [esi + INI_SETTING.ini_section], \
                [esi + INI_SETTING.key], eax
        jmp     .done

.word:
        mov     eax, [edi + 4 + eax*4]
        mov     [iniWord], eax
        xor     ecx, ecx                ; the length of the word

.length:
        cmp     byte [iniWord + ecx], ' '
        je      .save
        inc     ecx
        cmp     ecx, 4
        jb      .length

.save:
        invoke  ini.set_str, sz_ini, [esi + INI_SETTING.ini_section], \
                [esi + INI_SETTING.key], iniWord, ecx

.done:
        add     esi, sizeof.INI_SETTING
        cmp     esi, iniSettings.saved
        jb      .next
        ret

; ====================================================================

; pass the values to the kernel, the mouse ones at system start only
settings_apply_boot:
        mcall   SF_SYSTEM, SSF_MOUSE_SETTINGS, SSSF_SET_SPEED, [mouseSpeed]
        mcall   SF_SYSTEM, SSF_MOUSE_SETTINGS, SSSF_SET_SPEEDUP, [mouseAccel]
        mcall   SF_SYSTEM, SSF_MOUSE_SETTINGS, SSSF_SET_DOUBLE_CLICK_DELAY, [mouseDelay]

settings_apply:
        mov     ecx, [language]
        inc     ecx
        mcall   SF_SYSTEM_SET, SSF_SYS_LANG
        mcall   SF_SYSTEM_SET, SSF_ACCESS_HD_LBA, [lba]
        mcall   SF_SYSTEM_SET, SSF_ACCESS_PCI, [pci]
        mcall   SF_STYLE_SETTINGS, SSF_SET_FONT_SMOOTH, [fontSmoothing]
        mcall   SF_STYLE_SETTINGS, SSF_SET_FONT_SIZE, [fontHeight]

        ; the speaker can only be toggled
        mcall   SF_SYSTEM, SSF_SPEAKER, SSSF_GET_STATE
        cmp     eax, [speakerMute]
        je      .done
        mcall   SF_SYSTEM, SSF_SPEAKER, SSSF_TOGGLE

.done:
        ret

; ====================================================================

; the button style, the desktop background and the skin from the [style]
; section, at system start only
style_apply:
        invoke  ini.get_int, sz_ini, sz_style, sz_buttonStyle, 0
        mov     ecx, eax
        mcall   SF_STYLE_SETTINGS, SSF_SET_BUTTON_STYLE

        ; the background is set by a program: bg_program with bg_param
        invoke  ini.get_str, sz_ini, sz_style, sz_bgProgram, bgProgram, PATH_MAX, 0
        cmp     byte [bgProgram], 0
        je      .skin
        invoke  ini.get_str, sz_ini, sz_style, sz_bgParam, bgParam, PATH_MAX, 0
        cmp     byte [bgParam], '\'     ; a KIV parameter like '\S__/path/to/img'
        jne     .background
        mov     [fileInfo.path], bgParam + 4
        call    wait_for_file

.background:
        mcall   SF_FILE, runBgProgram

.skin:
        invoke  ini.get_str, sz_ini, sz_style, sz_skin, skinPath, PATH_MAX, 0
        cmp     byte [skinPath], 0
        je      .done
        mov     [fileInfo.path], skinPath
        call    wait_for_file
        mcall   SF_STYLE_SETTINGS, SSF_SET_SKIN, skinPath

.done:
        ret

; files on /kolibrios and /usbhd may appear a few seconds after boot:
; try to read the file for up to 5 seconds
wait_for_file:
        mov     edi, 5

.try:
        mcall   SF_FILE, fileInfo
        test    eax, eax
        jz      .done
        mcall   SF_SLEEP, 100
        dec     edi
        jnz     .try

.done:
        ret

; ====================================================================

; the languages of the kernel, in the order of their ids 1..11 (kernel
; data32.inc SYSLANG)
LANGUAGES       = 11
FONT_H_MIN      = 9
FONT_H_MAX      = 99                    ; two digits in the window
PATH_MAX        = 256

; a value stored in system.ini: its section and key, its variable and
; its words, 0 for a number, else the list of the words for 0, 1, 2...
struct INI_SETTING
        ini_section     dd ?            ; 'section' is a directive of fasm
        key             dd ?
        variable        dd ?
        words           dd ?
ends

align 4
iniSettings:
        dd      sz_system,   sz_language,    language,      wordsLanguage
        dd      sz_system,   sz_fontHeight,  fontHeight,    0
        dd      sz_system,   sz_fontSmooth,  fontSmoothing, wordsSmoothing
        dd      sz_system,   sz_speakerMute, speakerMute,   wordsOnOff
        dd      sz_lowLevel, sz_lba,         lba,           wordsOnOff
        dd      sz_lowLevel, sz_pci,         pci,           wordsOnOff
.saved:                                 ; the mouse is configured by its own application
        dd      sz_mouse,    sz_speed,       mouseSpeed,    0
        dd      sz_mouse,    sz_accel,       mouseAccel,    0
        dd      sz_mouse,    sz_dblClick,    mouseDelay,    0
.end:

; a word list: the number of the words, then each of them in 4 bytes
wordsOnOff      dd      2, 'off ', 'on  '
wordsSmoothing  dd      3, 'off ', 'on  ', 'sbp '
wordsLanguage   dd      LANGUAGES
                dd      'en  ', 'fi  ', 'de  ', 'ru  ', 'fr  ', 'et  '
                dd      'uk  ', 'it  ', 'nl  ', 'es  ', 'ca  '
assert $ - wordsLanguage = (1 + LANGUAGES)*4

runBgProgram:
        dd      SSF_START_APP, 0, bgParam, 0, 0
        db      0
        dd      bgProgram

fileInfo:
        dd      SSF_READ_FILE, 0, 0, 4, fileBuf
        db      0
.path   dd      ?

sz_ini          db      "/sys/settings/system.ini", 0

sz_system       db      "system", 0
sz_language     db      "language", 0
sz_fontHeight   db      "font height", 0
sz_fontSmooth   db      "font smoothing", 0
sz_speakerMute  db      "speaker mute", 0

sz_lowLevel     db      "low-level", 0
sz_lba          db      "LBA", 0
sz_pci          db      "PCI", 0

sz_mouse        db      "mouse", 0
sz_speed        db      "speed", 0
sz_accel        db      "acceleration", 0
sz_dblClick     db      "double_click_delay", 0

sz_style        db      "style", 0
sz_buttonStyle  db      "buttons_gradient", 0
sz_bgProgram    db      "bg_program", 0
sz_bgParam      db      "bg_param", 0
sz_skin         db      "skin", 0

; not stored in the program file: placed by IncludeUGlobals
uglobal
language        dd      ?
lba             dd      ?
pci             dd      ?
speakerMute     dd      ?
fontSmoothing   dd      ?
fontHeight      dd      ?
mouseSpeed      dd      ?
mouseAccel      dd      ?
mouseDelay      dd      ?

iniWord         dd      ?               ; a word of system.ini
fileBuf         dd      ?
bgProgram       rb      PATH_MAX
bgParam         rb      PATH_MAX
skinPath        rb      PATH_MAX
endg
