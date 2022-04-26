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
CFLAGS = CFLAGS  .. [[ -DTYRIAN_DIR="\"."\" -D_GNU_SOURCE=1 -D_REENTRANT -DNDEBUG -Wno-missing-field-initializers ]]

LDFLAGS = LDFLAGS .. " --subsystem native"

-- Game src files --
compile_gcc{
	"./src/scroller.c",
	"./src/config.c",
	"./src/game_menu.c",
	"./src/file.c",
	"./src/opentyr.c",
	"./src/sndmast.c",
	"./src/sizebuf.c",
	"./src/video_scale.c",
	"./src/loudness.c",
	"./src/palette.c",
	"./src/joystick.c",
	"./src/lds_play.c",
	"./src/font.c",
	"./src/config_file.c",
	"./src/network.c",
	"./src/helptext.c",
	"./src/xmas.c",
	"./src/keyboard.c",
	"./src/jukebox.c",
	"./src/picload.c",
	"./src/shots.c",
	"./src/setup.c",
	"./src/mouse.c",
	"./src/musmast.c",
	"./src/nortvars.c",
	"./src/backgrnd.c",
	"./src/destruct.c",
	"./src/lvllib.c",
	"./src/video_scale_hqNx.c",
	"./src/std_support.c",
	"./src/mtrand.c",
	"./src/sprite.c",
	"./src/episodes.c",
	"./src/arg_parse.c",
	"./src/opl.c",
	"./src/video.c",
	"./src/editship.c",
	"./src/vga_palette.c",
	"./src/pcxload.c",
	"./src/fonthand.c",
	"./src/mainint.c",
	"./src/tyrian2.c",
	"./src/lvlmast.c",
	"./src/animlib.c",
	"./src/pcxmast.c",
	"./src/menus.c",
	"./src/starlib.c",
	"./src/player.c",
	"./src/nortsong.c",
	"./src/vga256d.c",
	"./src/varz.c",
	"./src/params.c",
	"./SDL/joystick_stub.c",
	"./SDL/SDL_wave.c",
	"./SDL/SDL_audiocvt.c",
}

link_gcc("opentyrian")
