if tup.getconfig("NO_GCC") ~= "" or tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

CFLAGS = " -c -O2 -fno-ident -std=c99 -fomit-frame-pointer -mno-ms-bitfields -U_Win32 -U_WIN32 -U__MINGW32__ -DPACKAGE_NAME='\"Fplay-vaapi\"' -DDEBUG=1 -D_GNU_SOURCE -DHAVE_VAAPI -I../../sdk/sources/newlib/libc/include -I../../sdk/sources/ffmpeg/ffmpeg-2.8 -I../../sdk/sources/freetype/include -I../../sdk/sources/vaapi/libva-1.6.2 "

LDFLAGS = " -static --subsystem native --stack 0x200000 -Map fplay.map -Tapp-dynamic.lds --image-base 0 "

LIBS= " -lavdevice.dll -lavformat.dll -lavcodec.dll -lavutil.dll -lswscale.dll -lswresample.dll -lsound -lpixlib3 -lfreetype.dll -lva.dll -lgcc -lc.dll "

compile_gcc{
    "audio.c",
    "decoder.c",
    "fplay.c",
    "vaapi.c",
    "video.c",
    "utils.c",
    "winlib/button.c",
    "winlib/caption.c",
    "winlib/fontlib.c",
    "winlib/frame.c",
    "winlib/panel.c",
    "winlib/window.c",
}
tup.rule("opendial.asm", "fasm %f %o ", "opendial.o")
tup.rule("skin/skin.asm", "fasm %f %o ", "skin/skin.o")

tup.append_table(OBJS, {"opendial.o", "skin/skin.o"})

link_gcc("Fplay")
