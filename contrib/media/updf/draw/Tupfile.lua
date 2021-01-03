if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

SDK_DIR = "../../../sdk"

CFLAGS = CFLAGS .. " -std=c99"
INCLUDES = INCLUDES .. " -I ../fitz"

compile_gcc{"arch_arm.c", "arch_port.c", "draw_affine.c", "draw_blend.c", "draw_device.c", "draw_edge.c", "draw_glyph.c", "draw_mesh.c", "draw_paint.c", "draw_path.c", "draw_scale.c", "draw_unpack.c"}
tup.rule(OBJS, "kos32-ar rcs %o %f", {"../lib/libdraw.a", "<../lib/libdraw>"})
