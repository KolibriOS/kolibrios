if tup.getconfig("NO_GCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  if tup.getconfig("NO_NASM") == "full" then return end -- required for SDL compilation
  HELPERDIR = "../../../programs"
end
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")
--use_dynamic_stack() -- default 64K are not sufficient
tup.include(HELPERDIR .. "/use_sdl_newlib.lua")
CFLAGS = CFLAGS .. " -UWIN32 -U_WIN32 -U__WIN32__ -D_KOLIBRI"

LDFLAGS = LDFLAGS .. " --subsystem native"

-- Game src files --
compile_gcc{ 
    "wl_cloudsky.cpp", "wl_debug.cpp", "id_sd.cpp", "wl_play.cpp", "id_vl.cpp", "wl_act2.cpp", "wl_floorceiling.cpp", "wl_dir3dspr.cpp", 
    "wl_state.cpp", "wl_atmos.cpp", "id_in.cpp", "signon.cpp", "wl_parallax.cpp", "wl_agent.cpp", "sdl_winmain.cpp", "wl_inter.cpp", "wl_text.cpp", 
    "id_pm.cpp",  "wl_draw.cpp",  "wl_menu.cpp", "wl_game.cpp", "wl_act1.cpp", "wl_main.cpp", "wl_shade.cpp", "id_us_1.cpp", "id_vh.cpp", "id_ca.cpp", 
    "joystick_stub.cpp", "kolibri.cpp", "mame/fmopl.cpp",
}

-- SDL_mixer stubs -- 
compile_gcc{ 
  "SDL_mixer/mixer.c", "SDL_mixer/music.c", "SDL_mixer/load_aiff.c", "SDL_mixer/load_voc.c",
  "SDL_mixer/effects_internal.c", "SDL_mixer/effect_position.c",
}

link_gcc("wolf3d")
