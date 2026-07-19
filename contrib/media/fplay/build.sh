#!/usr/bin/env bash
#
# Полная сборка FPlay (медиаплеер KolibriOS).
# Full build for the FPlay KolibriOS media player.
#
# Поднимает окружение kos32-тулчейна (PATH, fasm, путь к либам) и собирает
# ВСЕ объекты заново + линкует + objcopy в MENUET-бинарь. Голый `make` этого
# не делает: в Makefile нет -L и он ждёт тулчейн уже в PATH.
#
# Запуск из Git Bash / MSYS:
#   ./build.sh          - полная сборка (all)
#   ./build.sh clean    - удалить .o, бинарь, .map
#
# Пути можно переопределить переменными окружения, если тулчейн лежит иначе:
#   TOOLCHAIN_BIN=... LIB_DIR=... FASM=... ./build.sh
#
set -u

# ---------------------------------------------------------------------------
# Расположение инструментов (значения по умолчанию = как на этой машине)
# ---------------------------------------------------------------------------
TOOLCHAIN_BIN="${TOOLCHAIN_BIN:-/c/MinGW/msys/1.0/home/autobuild/tools/win32/bin}"
LIB_DIR="${LIB_DIR:-/c/MinGW/msys/1.0/home/autobuild/tools/win32/lib}"
FASM="${FASM:-/c/Kolibri/Infrastructure/C--/fasm.exe}"

# Каталог скрипта = корень исходников fplay; работаем относительно него.
cd "$(dirname "$0")" || { echo "ERROR: cannot cd to script dir"; exit 1; }
SDK_DIR="$(cd ../../sdk && pwd)" || { echo "ERROR: SDK dir (../../sdk) not found"; exit 1; }

export PATH="$TOOLCHAIN_BIN:$PATH"

CC=kos32-gcc
LD=kos32-ld
OBJCOPY=kos32-objcopy
NAME=fplay

# fasm: явный путь, иначе из PATH
if [ -x "$FASM" ]; then FASM_CMD="$FASM"; else FASM_CMD="fasm"; fi

# ---------------------------------------------------------------------------
# Флаги (идентичны Makefile проекта)
# ---------------------------------------------------------------------------
CFLAGS="-U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fno-ident -std=c99 -fomit-frame-pointer -mno-ms-bitfields"
DEFINES="-DDEBUG=1 -D_GNU_SOURCE -DHAVE_VAAPI -DPACKAGE_NAME=\"Fplay-vaapi\""
INCLUDES="-I. -I$SDK_DIR/sources/newlib/libc/include -I$SDK_DIR/sources/ffmpeg/ffmpeg-2.8"
INCLUDES="$INCLUDES -I$SDK_DIR/sources/freetype/include -I$SDK_DIR/sources/vaapi/libva-1.6.2"

LIBS="-lavdevice.dll -lavformat.dll -lavcodec.dll -lavutil.dll -lswscale.dll"
LIBS="$LIBS -lswresample.dll -lsound -lpixlib3 -lfreetype.dll -lva.dll -lgcc -lc.dll"

LDFLAGS="-static --subsystem native --stack 0x200000 -Map fplay.map -Tapp-dynamic.lds --image-base 0"

# Исходники
C_SOURCES="audio.c decoder.c fplay.c vaapi.c video.c utils.c \
           winlib/button.c winlib/caption.c winlib/fontlib.c \
           winlib/frame.c winlib/panel.c winlib/window.c"
ASM_SOURCES="opendial.asm skin/skin.asm"

# Объекты в том порядке линковки, что использует проект
OBJECTS="opendial.o audio.o decoder.o fplay.o vaapi.o video.o utils.o \
         skin/skin.o winlib/button.o winlib/caption.o winlib/fontlib.o \
         winlib/frame.o winlib/panel.o winlib/window.o"

# ---------------------------------------------------------------------------
# clean
# ---------------------------------------------------------------------------
if [ "${1:-}" = "clean" ]; then
    rm -f $OBJECTS "$NAME" fplay.map
    echo "cleaned: objects, $NAME, fplay.map"
    exit 0
fi

# ---------------------------------------------------------------------------
# Проверка инструментов
# ---------------------------------------------------------------------------
command -v "$CC"      >/dev/null 2>&1 || { echo "ERROR: $CC not found (TOOLCHAIN_BIN=$TOOLCHAIN_BIN)"; exit 1; }
command -v "$LD"      >/dev/null 2>&1 || { echo "ERROR: $LD not found (TOOLCHAIN_BIN=$TOOLCHAIN_BIN)"; exit 1; }
command -v "$OBJCOPY" >/dev/null 2>&1 || { echo "ERROR: $OBJCOPY not found (TOOLCHAIN_BIN=$TOOLCHAIN_BIN)"; exit 1; }
command -v "$FASM_CMD" >/dev/null 2>&1 || { echo "ERROR: fasm not found (FASM=$FASM)"; exit 1; }
[ -d "$LIB_DIR" ] || { echo "ERROR: LIB_DIR not found: $LIB_DIR"; exit 1; }

echo "SDK_DIR = $SDK_DIR"
echo "toolchain = $TOOLCHAIN_BIN"
echo

# ---------------------------------------------------------------------------
# Компиляция всех объектов заново (полная сборка)
# ---------------------------------------------------------------------------
fail=0

for src in $C_SOURCES; do
    obj="${src%.c}.o"
    printf '  CC   %s\n' "$src"
    $CC $CFLAGS $DEFINES $INCLUDES -o "$obj" "$src" || { echo "  ^^ FAILED: $src"; fail=1; }
done

for src in $ASM_SOURCES; do
    obj="${src%.asm}.o"
    printf '  ASM  %s\n' "$src"
    "$FASM_CMD" "$src" "$obj" || { echo "  ^^ FAILED: $src"; fail=1; }
done

if [ $fail -ne 0 ]; then
    echo
    echo "COMPILE FAILED - see errors above"
    exit 1
fi

# ---------------------------------------------------------------------------
# Линковка + objcopy в бинарь MENUET
# ---------------------------------------------------------------------------
echo
printf '  LINK %s\n' "$NAME"
$LD $LDFLAGS -L"$LIB_DIR" -o "$NAME" $OBJECTS $LIBS || { echo "LINK FAILED"; exit 1; }

printf '  BIN  %s\n' "$NAME"
$OBJCOPY "$NAME" -O binary || { echo "OBJCOPY FAILED"; exit 1; }

# ---------------------------------------------------------------------------
# Итог
# ---------------------------------------------------------------------------
size=$(stat -c %s "$NAME" 2>/dev/null || wc -c < "$NAME")
hdr=$(head -c 6 "$NAME" 2>/dev/null)
echo
echo "BUILD OK"
echo "  file  : $(pwd)/$NAME"
echo "  size  : $size bytes"
echo "  header: $hdr"
[ "$hdr" = "MENUET" ] || echo "  WARNING: header is not MENUET - check app-dynamic.lds"
