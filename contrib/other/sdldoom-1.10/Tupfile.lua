if tup.getconfig("NO_GCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  if tup.getconfig("NO_NASM") == "full" then return end -- required for SDL compilation
  HELPERDIR = "../../../programs"
end
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")
tup.include(HELPERDIR .. "/use_sdl_newlib.lua")
CFLAGS = CFLAGS .. " -UWIN32 -U_WIN32 -U__WIN32__ -U__MINGW32__ -D_KOLIBRI"

LDFLAGS = LDFLAGS .. " --subsystem native"

-- DOOM source files (all *.c, as in the Makefile)
compile_gcc{
    "am_map.c", "d_items.c", "d_main.c", "d_net.c", "doomdef.c", "doomstat.c", "dstrings.c",
    "f_finale.c", "f_wipe.c", "g_game.c", "hu_lib.c", "hu_stuff.c", "i_main.c", "i_net.c",
    "i_sound.c", "i_system.c", "i_video.c", "info.c", "m_argv.c", "m_bbox.c", "m_cheat.c",
    "m_fixed.c", "m_menu.c", "m_misc.c", "m_random.c", "m_swap.c", "p_ceilng.c", "p_doors.c",
    "p_enemy.c", "p_floor.c", "p_inter.c", "p_lights.c", "p_map.c", "p_maputl.c", "p_mobj.c",
    "p_plats.c", "p_pspr.c", "p_saveg.c", "p_setup.c", "p_sight.c", "p_spec.c", "p_switch.c",
    "p_telept.c", "p_tick.c", "p_user.c", "r_bsp.c", "r_data.c", "r_draw.c", "r_main.c",
    "r_plane.c", "r_segs.c", "r_sky.c", "r_things.c", "s_sound.c", "sounds.c", "st_lib.c",
    "st_stuff.c", "tables.c", "v_video.c", "w_wad.c", "wi_stuff.c", "z_zone.c",
}

link_gcc("doom")
