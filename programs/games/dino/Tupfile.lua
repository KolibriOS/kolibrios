if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

-- The floppy image carries no libc.dll, so link newlib statically
LDFLAGS = LDFLAGS:gsub("app%-dynamic%.lds", "app.lds") .. " -L" .. TOOLCHAIN_LIBPATH .. " --subsystem native"
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
