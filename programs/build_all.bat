@echo off

set languages=en ru ge et
cls
echo Build KolibriOS apps
echo Enter valide languege
echo     [%languages%]

set /P res=">

@erase lang.inc
echo lang fix %res% > lang.inc

echo *
echo Building system
echo *
@fasm system\calendar\trunk\calendar.asm calendar
@fasm system\board\trunk\board.asm board
@fasm system\cpu\trunk\cpu.asm cpu 
@fasm system\cpuid\trunk\cpuid.asm cpuid
@fasm system\desktop\trunk\desktop.asm desktop
@fasm system\docpack\trunk\docpack.asm docpack
@fasm system\end\trunk\end.asm end
@fasm system\gmon\gmon.asm gmon
@fasm system\icon\trunk\icon.asm icon
@fasm system\kbd\trunk\kbd.ASM kbd
@fasm system\launcher\trunk\launcher.asm launcher
@fasm system\menu\trunk\menu.asm menu
@fasm system\PANEL\trunk\@PANEL.ASM @PANEL
@fasm system\pcidev\trunk\pcidev.asm pcidev
@fasm system\RB\trunk\@RB.ASM @RB
@fasm system\rdsave\trunk\rdsave.asm rdsave
@fasm system\run\trunk\run.asm run
@fasm system\setup\trunk\setup.asm setup
@fasm system\skinsel\skinsel.asm skinsel
@fasm system\vrr\trunk\vrr.asm vrr
@fasm system\vrr_m\trunk\vrr_m.asm vrr_m

echo *
echo Building develop
echo *
@fasm develop\cmd\trunk\cmd.asm cmd
@fasm develop\fasm\trunk\fasm.asm fasm
@fasm develop\h2d2b\trunk\h2d2b.asm h2d2b
@fasm develop\heed\trunk\heed.asm heed
@fasm develop\hexview\trunk\hexview.asm hexview
@fasm develop\keyascii\trunk\keyascii.asm keyascii
@fasm develop\mtdbg\mtdbg.asm mtdbg
@fasm develop\param\trunk\param.asm param
@fasm develop\scancode\trunk\scancode.asm scancode
@fasm develop\tinypad\trunk\tinypad.asm tinypad

echo *
echo Building fs
echo *
@fasm fs\copy2\trunk\copy2.asm cmd
@fasm fs\copyr\trunk\copyr.asm copyr
@fasm fs\kfar\trunk\kfar.asm kfar
@fasm fs\mfar\trunk\mfar.asm mfar
@fasm fs\sysxtree\trunk\sysxtree.asm sysxtree

echo *
echo Building network
echo *
@fasm network\airc\trunk\airc.asm airc
@fasm network\arpstat\trunk\arpstat.asm arpstat
@fasm network\autodhcp\trunk\autodhcp.asm autodhcp
@fasm network\dhcp\trunk\dhcp.asm dhcp
@fasm network\dnsr\trunk\dnsr.asm dnsr
@fasm network\ethstat\trunk\ethstat.asm ethstat
@fasm network\httpc\trunk\httpc.asm httpc
@fasm network\https\trunk\https.asm https
@fasm network\ipc\trunk\ipc.asm ipc
@fasm network\local\trunk\local.asm local
@fasm network\netsendc\trunk\netsendc.asm netsendc
@fasm network\netsends\trunk\netsends.asm netsends
@fasm network\nntpc\trunk\nntpc.asm nntpc
@fasm network\popc\trunk\popc.asm popc
@fasm network\ppp\trunk\ppp.asm ppp
@fasm network\rccc\trunk\rccc.asm rccc
@fasm network\rccs\trunk\rccs.asm rccs
@fasm network\remote\trunk\remote.asm remote
@fasm network\smtps\trunk\smtps.asm smtps
@fasm network\stackcfg\trunk\stackcfg.asm stackcfg
@fasm network\telnet\trunk\telnet.asm telnet
@fasm network\terminal\trunk\terminal.asm terminal
@fasm network\tftpa\trunk\tftpa.asm tftpa
@fasm network\tftpc\trunk\tftpc.asm tftpc
@fasm network\VNCclient\VNCclient.asm VNCclient
@fasm network\ym\trunk\ym.asm ym

echo *
echo Building other
echo *
@fasm other\archer\trunk\@rcher.asm @rcher
@fasm other\calc\trunk\calc.asm calc
@fasm other\mhc\trunk\mhc.asm mhc
@fasm other\period\trunk\period.asm period
@fasm other\rtfread\trunk\rtfread.asm rtfread

echo *
echo Building media
echo *
rem @fasm media\ac97snd\trunk\ac97snd.asm ac97snd
@fasm media\animage\trunk\animage.asm animage
@fasm media\cdp\trunk\cdp.asm cdp
@fasm media\gifview\trunk\gifview.asm gifview
@fasm media\iconedit\trunk\iconedit.asm iconedit
@fasm media\jpegview\trunk\jpegview.asm jpegview
@fasm media\midamp\trunk\midamp.asm midamp
@fasm media\midiplay\trunk\midiplay.asm midiplay
@fasm media\mixer\trunk\mixer.asm mixer
@fasm media\mp3s\trunk\mp3s.asm mp3s
@fasm media\mv\trunk\mv.asm mv
@fasm media\pic4\trunk\pic4.asm pic4
@fasm media\sb\trunk\sb.asm sb
@fasm media\scrshoot\scrshoot.asm scrshoot

echo *
echo Building games
echo *
@fasm games\15\trunk\15.asm 15
@fasm games\arcanii\trunk\arcanii.asm arcanii
@fasm games\arcanoid\trunk\arcanoid.asm arcanoid
rem @fasm games\c4\trunk\c4.asm c4
@fasm games\chess\trunk\chess.asm chess
@fasm games\freecell\freecell.asm freecell
@fasm games\mblocks\trunk\mblocks.asm mblocks
@fasm games\phenix\trunk\phenix.asm phenix
@fasm games\pipes\pipes.asm pipes
@fasm games\pong\trunk\pong.asm pong
@fasm games\pong3\trunk\pong3.asm pong3
@fasm games\tanks\trunk\tanks.asm tanks
@fasm games\tetris\trunk\tetris.asm tetris

echo *
echo Building demos
echo *
@fasm demos\3dcube2\trunk\3dcube2.asm 3dcube2
@fasm demos\3detx60b\trunk\3detx60b.asm 3detx60b
@fasm demos\3dtcub10\trunk\3dtcub10.asm 3dtcub10
rem @fasm demos\aclock\trunk\aclock.asm aclock
@fasm demos\bcdclk\bcdclk\bcdclk.asm bcdclk
@fasm demos\bgitest\trunk\bgitest.asm bgitest
@fasm demos\colorref\trunk\colorref.asm colorref
@fasm demos\crownscr\trunk\crownscr.asm crownscr
@fasm demos\cslide\trunk\cslide.asm cslide
@fasm demos\eyes\trunk\eyes.asm eyes
@fasm demos\fire\trunk\fire.asm fire
@fasm demos\fire2\trunk\fire2.asm fire2
@fasm demos\free3d04\trunk\free3d04.asm free3d04
@fasm demos\magnify\trunk\magnify.asm magnify
@fasm demos\movback\trunk\movback.asm movback
@fasm demos\plasma\trunk\plasma.asm plasma
@fasm demos\ss\trunk\@ss.asm @ss
@fasm demos\timer\trunk\timer.asm timer
@fasm demos\tinyfrac\trunk\tinyfrac.asm tinyfrac
@fasm demos\transp\trunk\transp.asm transp
@fasm demos\trantest\trunk\trantest.asm trantest
@fasm demos\tube\trunk\tube.asm tube

@erase lang.inc

echo *
echo Finished building 
echo *
pause
