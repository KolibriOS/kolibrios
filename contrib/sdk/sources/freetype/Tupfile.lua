if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

CFLAGS = CFLAGS .. " -U_Win32 -U_WIN32 -U__MINGW32__ -DFT_CONFIG_CONFIG_H=\"<ftconfig.h>\" -DFT_CONFIG_MODULES_H=\"<ftmodule.h>\" -DFT2_BUILD_LIBRARY"
INCLUDES = INCLUDES .. " -I./include -I./include/freetype/config"

compile_gcc{"src/autofit/autofit.c", "src/base/ftbase.c", "src/base/ftbbox.c", "src/base/ftbdf.c", "src/base/ftbitmap.c", "src/base/ftcid.c", "src/base/ftdebug.c", "src/base/ftfstype.c", "src/base/ftgasp.c", "src/base/ftglyph.c", "src/base/ftgxval.c", "src/base/ftinit.c", "src/base/ftlcdfil.c", "src/base/ftmm.c", "src/base/ftotval.c", "src/base/ftpatent.c", "src/base/ftpfr.c", "src/base/ftstroke.c", "src/base/ftsynth.c", "src/base/ftsystem.c", "src/base/fttype1.c", "src/bdf/bdf.c", "src/bzip2/ftbzip2.c", "src/cache/ftcache.c", "src/cff/cff.c", "src/cid/type1cid.c", "src/gzip/ftgzip.c", "src/lzw/ftlzw.c", "src/otvalid/otvalid.c", "src/pcf/pcf.c", "src/pfr/pfr.c", "src/psaux/psaux.c", "src/pshinter/pshinter.c", "src/psnames/psnames.c", "src/raster/raster.c", "src/sfnt/sfnt.c", "src/smooth/smooth.c", "src/truetype/truetype.c", "src/type1/type1.c", "src/type42/type42.c", "src/winfonts/winfnt.c"}
tup.rule(OBJS, "kos32-ar rcs %o %f", {"../../lib/libfreetype.a", "../../lib/<libfreetype>"})
