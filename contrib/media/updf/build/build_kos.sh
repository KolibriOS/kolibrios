#!/bin/bash
# Experimental KolibriOS build of MuPDF 1.19.0 (PDF-only, minimal deps).
# Compiles fitz + pdf core with kos32-gcc, reports what fails.
set -u
export PATH="/c/MinGW/msys/1.0/home/autobuild/tools/win32/bin:$PATH"

HERE="$(cd "$(dirname "$0")" && pwd)"
SRC="$HERE/../mupdf-1.19.0-source"
SDK=/c/Kolibri/git/contrib/sdk
OBJ="$HERE/obj"
mkdir -p "$OBJ"

# PDF only. #if-style features are set to 0; #ifdef-style OUTPUT features
# (DOCX/ODT/OCR) must stay UNDEFINED to be disabled - do not pass them.
# TOFU (+SIL) drops all non-CJK Noto fonts; TOFU_CJK/SYMBOL/EMOJI drop the rest,
# leaving only the base14 URW fonts we actually ship.
DEFS="-DFZ_ENABLE_XPS=0 -DFZ_ENABLE_SVG=0 -DFZ_ENABLE_CBZ=0 -DFZ_ENABLE_IMG=0 \
-DFZ_ENABLE_HTML=0 -DFZ_ENABLE_EPUB=0 -DFZ_ENABLE_ICC=0 -DFZ_ENABLE_JPX=0 \
-DFZ_ENABLE_JS=0 -DTOFU -DTOFU_CJK -DTOFU_SYMBOL -DTOFU_EMOJI -DTOFU_SIL"

INC="-I $SRC/include -I $SRC/source/fitz -I $SRC/generated -I $HERE/shim \
-I $SDK/sources/newlib/libc/include \
-I $SDK/sources/freetype/include \
-I $SRC/thirdparty/zlib \
-I $SRC/thirdparty/jbig2dec \
-I $SRC/thirdparty/libjpeg \
-I $SRC/scripts/libjpeg \
-I $SRC/thirdparty/gumbo-parser/src"

CFLAGS="-c -fno-ident -O2 -fomit-frame-pointer -U__WIN32__ -U_WIN32 -U__MINGW32__ -UWIN32"

# sources we do NOT want:
#   harfbuzz.c   - no harfbuzz (PDF needs no text shaping)
#   output-docx.c - DOCX writer, needs the 'extract' thirdparty (disabled)
SKIP="harfbuzz.c output-docx.c"

ok=0; fail=0
: > "$HERE/fail.log"
for dir in fitz pdf; do
	for f in "$SRC/source/$dir"/*.c; do
		base=$(basename "$f")
		case " $SKIP " in *" $base "*) continue;; esac
		if kos32-gcc $CFLAGS $DEFS $INC -o "$OBJ/${dir}_${base%.c}.o" "$f" 2>>"$HERE/fail.log"; then
			ok=$((ok+1))
		else
			fail=$((fail+1))
			echo "FAIL $dir/$base" | tee -a "$HERE/fail.log" >/dev/null
			echo "FAIL $dir/$base"
		fi
	done
done

# gumbo-parser (HTML5 parser used by fitz/xml.c) - pure C, 11 files
GINC="-I $SRC/thirdparty/gumbo-parser/src -I $SDK/sources/newlib/libc/include"
for f in "$SRC/thirdparty/gumbo-parser/src"/*.c; do
	base=$(basename "$f")
	if kos32-gcc $CFLAGS $GINC -o "$OBJ/gumbo_${base%.c}.o" "$f" 2>>"$HERE/fail.log"; then
		ok=$((ok+1))
	else
		fail=$((fail+1)); echo "FAIL gumbo/$base"
	fi
done

echo "=== compiled OK: $ok, FAILED: $fail ==="