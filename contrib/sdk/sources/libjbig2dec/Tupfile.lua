if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")
 
CFLAGS = CFLAGS .. " -std=c99 -DHAVE_CONFIG_H"
INCLUDES = INCLUDES .. " -I ."
 
compile_gcc{"jbig2_arith.c", "jbig2_arith_iaid.c", "jbig2_arith_int.c", "jbig2.c", "jbig2dec.c", "jbig2_generic.c", "jbig2_halftone.c", "jbig2_huffman.c", "jbig2_image.c", "jbig2_image_pbm.c", "jbig2_metadata.c", "jbig2_mmr.c", "jbig2_page.c", "jbig2_refinement.c", "jbig2_segment.c", "jbig2_symbol_dict.c", "jbig2_text.c", "memcmp.c", "sha1.c"}
tup.rule(OBJS, "kos32-ar rcs %o %f", {"../../lib/libjbig2dec.a", "<../../lib/libjbig2dec>"})
