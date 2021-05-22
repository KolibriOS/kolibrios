if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

SDK_DIR = "../../../sdk"

CFLAGS = CFLAGS .. " -std=c99"
INCLUDES = INCLUDES .. " -I . -I ../fitz -I " .. SDK_DIR .. "/sources/freetype/include"

compile_gcc{"pdf_annot.c", "pdf_cmap.c", "pdf_cmap_load.c", "pdf_cmap_parse.c", "pdf_cmap_table.c", "pdf_colorspace.c", "pdf_crypt.c", "pdf_encoding.c", "pdf_font.c", "pdf_fontfile.c", "pdf_function.c", "pdf_image.c", "pdf_interpret.c", "pdf_lex.c", "pdf_metrics.c", "pdf_nametree.c", "pdf_outline.c", "pdf_page.c", "pdf_parse.c", "pdf_pattern.c", "pdf_repair.c", "pdf_shade.c", "pdf_store.c", "pdf_stream.c", "pdf_type3.c", "pdf_unicode.c", "pdf_xobject.c", "pdf_xref.c", "snprintf.c"}
tup.rule(OBJS, "kos32-ar rcs %o %f", {"../lib/libmupdf.a", "<../lib/libmupdf>"})
