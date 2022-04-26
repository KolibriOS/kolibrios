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
CFLAGS = CFLAGS .. " -DSDL -UWIN32 -U_WIN32 -U__WIN32__ -D_KOLIBRI"
CFLAGS = CFLAGS .. " -DUSE_ASM"

LDFLAGS = LDFLAGS .. " --subsystem native --stack 0x200000"

-- CFLAGS = CFLAGS:gsub("-Os", "-O2")
compile_gcc{
  "chase.c", "cl_demo.c", "cl_input.c", "cl_main.c", "cl_parse.c", "cl_tent.c",
  "cmd.c", "common.c", "console.c", "crc.c", "cvar.c", "d_edge.c", "d_init.c",
  "d_modech.c", "d_part.c", "d_polyse.c", "d_scan.c", "d_sky.c", "d_sprite.c",
  "d_surf.c", "draw.c", "host.c", "host_cmd.c", "keys.c", "mathlib.c",
  "menu.c", "model.c", "net_loop.c", "net_main.c", "net_vcr.c", "pr_cmds.c",
  "pr_edict.c", "pr_exec.c", "r_aclip.c", "r_alias.c", "r_bsp.c", "r_draw.c",
  "r_edge.c", "r_efrag.c", "r_light.c", "r_main.c", "r_misc.c", "r_part.c",
  "r_sky.c", "r_sprite.c", "r_surf.c", "sbar.c", "screen.c", "snd_dma.c",
  "snd_mem.c", "snd_mix.c", "sv_main.c", "sv_move.c", "sv_phys.c", "sv_user.c",
  "view.c", "wad.c", "world.c", "zone.c"
}
-- asm vs c
--[[compile_gcc{
  "d_vars.c", "r_vars.c", "nonintel.c"
}]]
compile_gcc{
  "d_draw.S", "d_draw16.S", "d_parta.S", "d_polysa.S", "d_scana.S", "d_spr8.S",
  "d_varsa.S", "math.S", "r_aclipa.S", "r_aliasa.S", "r_drawa.S", "r_edgea.S",
  "r_varsa.S", "snd_mixa.S", "surf8.S", "surf16.S", "sys_wina.S", "worlda.S"
}
-- select variants
compile_gcc{"sys_sdl.c"} -- SDL frontend
compile_gcc{"vid_sdl.c"} -- video and mouse from SDL
compile_gcc{"cd_null.c"} -- no CD audio
compile_gcc{"snd_sdl.c"} -- sound from SDL
compile_gcc{"net_none.c"} -- no network
link_gcc("sdlquake")
