if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

SDK_DIR = "../../../sdk"

CFLAGS = CFLAGS .. " -std=c99 -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32"
INCLUDES = INCLUDES .. " -I . -I " .. SDK_DIR .. "/sources/freetype/include -I " .. SDK_DIR .. "/sources/libjpeg -I " .. SDK_DIR .. "/sources/zlib -I " .. SDK_DIR .. "/sources/libopenjpeg -I " .. SDK_DIR .. "/sources/libjbig2dec"

compile_gcc{"base_error.c", "base_geometry.c", "base_getopt.c", "base_hash.c", "base_memory.c", "base_object.c", "base_string.c", "base_time.c", "crypt_aes.c", "crypt_arc4.c", "crypt_md5.c", "crypt_sha2.c", "dev_bbox.c", "dev_list.c", "dev_null.c", "dev_text.c", "dev_trace.c", "filt_basic.c", "filt_dctd.c", "filt_faxd.c", "filt_flate.c", "filt_jbig2d.c", "filt_jpxd.c", "filt_lzwd.c", "filt_predict.c", "lrintf.c", "obj_print.c", "res_bitmap.c", "res_colorspace.c", "res_font.c", "res_halftone.c", "res_path.c", "res_pixmap.c", "res_shade.c", "res_text.c", "stm_buffer.c", "stm_open.c", "stm_read.c", "strtoll.c", "strtoull.c"}
tup.rule(OBJS, "kos32-ar rcs %o %f", {"../lib/libfitz.a", "<../lib/libfitz>"})
