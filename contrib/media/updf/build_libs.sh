#!/bin/bash
# Build uPDF for KolibriOS: third-party libs from the SDK, the MuPDF
# libraries (fitz, pdf, draw) and the application itself.
# Needs only the kos32-gcc toolchain, no make.
#
# Usage: ./build_libs.sh        - build missing libraries, then the app
#        ./build_libs.sh all    - force-rebuild everything

set -e
cd "$(dirname "$0")"
SDK=$(cd ../../sdk && pwd)

# locate the kos32 toolchain
if ! command -v kos32-gcc >/dev/null 2>&1; then
	for p in /home/autobuild/tools/win32/bin \
	         /c/MinGW/msys/1.0/home/autobuild/tools/win32/bin; do
		if [ -x "$p/kos32-gcc" ] || [ -x "$p/kos32-gcc.exe" ]; then
			export PATH="$p:$PATH"
			break
		fi
	done
fi
command -v kos32-gcc >/dev/null 2>&1 || { echo "error: kos32-gcc not found"; exit 1; }
TOOLLIB="$(dirname "$(command -v kos32-gcc)")/../lib"

CFLAGS="-c -fno-ident -O2 -fomit-frame-pointer -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32"
NEWLIB_INC="-I $SDK/sources/newlib/libc/include"

FORCE=0
[ "$1" = "all" ] && FORCE=1

build_lib() { # <src dir> <lib name> <dest dir> <cflags...>
	local dir="$1" lib="$2" dest="$3"
	shift 3
	if [ $FORCE = 0 ] && [ -f "$dest/$lib" ]; then
		echo "skip $lib (already in $dest)"
		return
	fi
	echo "building $lib..."
	( cd "$dir"
	  rm -f *.o
	  for f in *.c; do
		kos32-gcc $CFLAGS "$@" -o "${f%.c}.o" "$f"
	  done
	  kos32-ar rcs "$lib" *.o
	  rm -f *.o
	  mkdir -p "$dest"
	  mv -f "$lib" "$dest/" )
}

build_lib "$SDK/sources/libjbig2dec" libjbig2dec.a "$SDK/lib" \
	-DHAVE_CONFIG_H $NEWLIB_INC -I "$SDK/sources/freetype/include" \
	-I "$SDK/sources/libpng" -I "$SDK/sources/zlib" -I .

build_lib "$SDK/sources/libopenjpeg" libopenjpeg.a "$SDK/lib" \
	$NEWLIB_INC -I "$SDK/sources/freetype/include" -I "$SDK/sources/zlib" -I .

build_lib fitz libfitz.a "$PWD/lib" \
	$NEWLIB_INC -I "$SDK/sources/freetype/include" -I "$SDK/sources/libjpeg" \
	-I "$SDK/sources/zlib" -I "$SDK/sources/libopenjpeg" -I "$SDK/sources/libjbig2dec"

build_lib pdf libmupdf.a "$PWD/lib" \
	$NEWLIB_INC -I "$PWD/fitz" -I "$SDK/sources/freetype/include"

build_lib draw libdraw.a "$PWD/lib" \
	$NEWLIB_INC -I "$PWD/fitz"

echo "building updf..."
cd apps
rm -f *.o updf
for f in kolibri.c kos_main.c pdfapp.c; do
	kos32-gcc $CFLAGS $NEWLIB_INC -I "$SDK/sources/freetype/include" \
		-I "$SDK/sources/zlib" -I ../fitz -I ../pdf -o "${f%.c}.o" "$f"
done
kos32-ld -static -nostdlib -T "$SDK/sources/newlib/app.lds" --image-base 0 \
	-L "$SDK/lib" -L "$TOOLLIB" -L ../lib --subsystem native \
	-o updf kolibri.o pdfapp.o kos_main.o \
	-lmupdf -lfitz -lgcc -lfitz -ldraw -ljpeg -ljbig2dec -lfreetype -lopenjpeg -lz.dll -lc.dll
kos32-objcopy updf -O binary
rm -f *.o
echo "done: apps/updf"
