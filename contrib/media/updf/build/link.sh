#!/bin/bash
# Archive the compiled MuPDF 1.19 objects into static libs and link the
# render pipeline test, proving the PDF-only engine links for KolibriOS.
# Prereq: run build_kos.sh and build_third.sh first (populate obj/).
set -e
export PATH="/c/MinGW/msys/1.0/home/autobuild/tools/win32/bin:$PATH"
HERE="$(cd "$(dirname "$0")" && pwd)"
SRC="$HERE/../mupdf-1.19.0-source"
SDK=/c/Kolibri/git/contrib/sdk
TOOL=/c/MinGW/msys/1.0/home/autobuild/tools/win32
OBJ="$HERE/obj"

NEWLIB="-I $SDK/sources/newlib/libc/include"

# KolibriOS libc gap-fillers (abort/atexit/atoll/timegm/realpath/tls_*)
kos32-gcc -c -O2 -fno-ident -U__WIN32__ -U_WIN32 $NEWLIB -o "$OBJ/kosshim.o" "$HERE/kosshim.c"
# link-test driver
kos32-gcc -c -O2 -fno-ident -U__WIN32__ -U_WIN32 -I "$SRC/include" $NEWLIB -o "$OBJ/linktest.o" "$HERE/linktest.c"

# static libraries: the port's deliverable for the GUI wrapper
rm -f "$OBJ/libmupdf.a" "$OBJ/libthird.a"
kos32-ar rcs "$OBJ/libmupdf.a" "$OBJ"/fitz_*.o "$OBJ"/pdf_*.o "$OBJ"/gumbo_*.o "$OBJ"/res_*.o
kos32-ar rcs "$OBJ/libthird.a" "$OBJ"/freetype_*.o "$OBJ"/zlib_*.o "$OBJ"/jbig2dec_*.o "$OBJ"/libjpeg_*.o

kos32-ld -static -nostdlib -T "$SDK/sources/newlib/app.lds" --image-base 0 \
	-L "$SDK/lib" -L "$TOOL/lib" \
	-o "$HERE/updfnew_test" \
	"$OBJ/linktest.o" "$OBJ/kosshim.o" "$OBJ/libmupdf.a" "$OBJ/libthird.a" \
	-lc.dll -lgcc
# real KolibriOS apps then need: kos32-objcopy updfnew_test -O binary
echo "linked OK -> build/updfnew_test  ($(wc -c < "$HERE/updfnew_test") bytes)"
echo "libs: $(wc -c < "$OBJ/libmupdf.a") libmupdf.a  $(wc -c < "$OBJ/libthird.a") libthird.a"