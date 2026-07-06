#!/bin/bash
# Compile MuPDF 1.19.0 thirdparty deps + base14 font resources for KolibriOS.
# File lists are read straight from the upstream Makelists so we build exactly
# MuPDF's chosen subset. Reports failures; does not archive yet.
set -u
export PATH="/c/MinGW/msys/1.0/home/autobuild/tools/win32/bin:$PATH"

HERE="$(cd "$(dirname "$0")" && pwd)"
SRC="$HERE/../mupdf-1.19.0-source"
SDK=/c/Kolibri/git/contrib/sdk
OBJ="$HERE/obj"
mkdir -p "$OBJ"

CFLAGS="-c -fno-ident -O2 -fomit-frame-pointer -U__WIN32__ -U_WIN32 -U__MINGW32__ -UWIN32"
NEWLIB="-I $SDK/sources/newlib/libc/include"
: > "$HERE/fail_third.log"
ok=0; fail=0

# pull "VAR += path" lists from Makelists
srclist() { grep "^$1 " "$SRC/Makelists" | sed "s#^$1 += *##"; }

compile() { # tag, cflags, file(relative to SRC)
	local tag="$1" cf="$2" f="$SRC/$3"
	local base=$(basename "$3")
	if kos32-gcc $CFLAGS $cf -o "$OBJ/${tag}_${base%.c}.o" "$f" 2>>"$HERE/fail_third.log"; then
		ok=$((ok+1))
	else
		fail=$((fail+1)); echo "FAIL $tag/$base"
	fi
}

FT="$NEWLIB -I $SRC/thirdparty/freetype/include -I $SRC/scripts/freetype \
-DFT2_BUILD_LIBRARY -DFT_CONFIG_MODULES_H=\"slimftmodules.h\" -DFT_CONFIG_OPTIONS_H=\"slimftoptions.h\""
ZL="$NEWLIB -I $SRC/thirdparty/zlib"
JB="$NEWLIB -I $SRC/include -I $SRC/thirdparty/jbig2dec -DHAVE_STDINT_H -DJBIG_EXTERNAL_MEMENTO_H=\"mupdf/memento.h\""
JP="$NEWLIB -I $SRC/thirdparty/libjpeg -I $SRC/scripts/libjpeg"
RS="$NEWLIB -I $SRC/include"

echo "--- freetype ---"; for f in $(srclist FREETYPE_SRC); do compile freetype "$FT" "$f"; done
echo "--- zlib ---";     for f in $(srclist ZLIB_SRC);     do compile zlib     "$ZL" "$f"; done
echo "--- jbig2dec ---"; for f in $(srclist JBIG2DEC_SRC); do compile jbig2dec "$JB" "$f"; done
echo "--- libjpeg ---";  for f in $(srclist LIBJPEG_SRC);  do compile libjpeg  "$JP" "$f"; done
echo "--- resources (base14 fonts) ---"
for f in $(cd "$SRC" && ls generated/resources/fonts/urw/*.cff.c); do compile res "$RS" "$f"; done

echo "=== thirdparty compiled OK: $ok, FAILED: $fail ==="