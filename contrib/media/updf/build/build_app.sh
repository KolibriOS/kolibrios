#!/bin/bash
# Build the uPDF KolibriOS application (GUI) against the MuPDF 1.19 libs.
# Prereq: build_kos.sh, build_third.sh, link.sh (libs in build/obj).
set -e
export PATH="/c/MinGW/msys/1.0/home/autobuild/tools/win32/bin:$PATH"
HERE="$(cd "$(dirname "$0")" && pwd)"
SRC="$HERE/../mupdf-1.19.0-source"
APP="$HERE/../apps"
SDK=/c/Kolibri/git/contrib/sdk
TOOL=/c/MinGW/msys/1.0/home/autobuild/tools/win32
OBJ="$HERE/obj"

INC="-I $SRC/include -I $HERE/shim -I $SDK/sources/newlib/libc/include -I $SDK/sources/freetype/include"
CF="-c -fno-ident -O2 -fomit-frame-pointer -U__WIN32__ -U_WIN32 -U__MINGW32__ -UWIN32"

echo "compiling app..."
kos32-gcc $CF $INC -o "$OBJ/app_kolibri.o" "$APP/kolibri.c"
kos32-gcc $CF $INC -o "$OBJ/app_pdfapp.o"  "$APP/pdfapp.c"
kos32-gcc $CF $INC -o "$OBJ/app_kos_main.o" "$APP/kos_main.c"

echo "linking uPDF..."
kos32-ld -static -nostdlib -T "$SDK/sources/newlib/app.lds" --image-base 0 --subsystem native \
	-L "$SDK/lib" -L "$TOOL/lib" \
	-o "$HERE/uPDF" \
	"$OBJ/app_kos_main.o" "$OBJ/app_pdfapp.o" "$OBJ/app_kolibri.o" "$OBJ/kosshim.o" \
	"$OBJ/libmupdf.a" "$OBJ/libthird.a" \
	-lc.dll -lgcc
kos32-objcopy "$HERE/uPDF" -O binary
echo "built: build/uPDF  ($(wc -c < "$HERE/uPDF") bytes)  magic=$(head -c 8 "$HERE/uPDF")"
