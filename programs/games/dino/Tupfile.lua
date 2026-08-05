if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

-- The floppy image carries no libc.dll, so link newlib statically:
-- rebuild LDFLAGS from scratch with app.lds instead of use_newlib's app-dynamic.lds
LDFLAGS = "-static -nostdlib -n --file-alignment=16 --section-alignment=16 -L" .. tup.getvariantdir()
LDFLAGS = LDFLAGS .. " -T" .. NEWLIB_BASE .. "/app.lds --image-base 0 --subsystem native -L" .. TOOLCHAIN_LIBPATH
LIBS = "-lc -lm -lgcc"
LIBDEPS = {}

compile_gcc{
    "cloud.c",
    "game_over_panel.c",
    "horizon.c",
    "main.c",
    "obstacle.c",
    "trex.c",
    "distance_meter.c",
    "graphics.c",
    "horizon_line.c",
    "misc.c",
    "runner.c"
}

link_gcc("dino")
