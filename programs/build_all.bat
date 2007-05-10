@echo off

set languages=en ru ge et
set __CPU_type=p5 p6 k6
set kpack=y n
cls
echo Build KolibriOS apps
echo Enter valide language
echo     [%languages%]

set /P res=">

@erase lang.inc
echo lang fix %res% > lang.inc

echo Enter CPU_type ("p5" for interrupt, "p6" for SYSENTER, "k6" for SYSCALL)

set /p res=">

@erase config.inc
echo __CPU_type fix %res% > config.inc


if not exist bin mkdir bin
if not exist bin\demos mkdir bin\demos
if not exist bin\develop mkdir bin\develop
if not exist bin\games mkdir bin\games
if not exist bin\network mkdir bin\network
if not exist bin\3d mkdir bin\3d
if not exist bin\fonts mkdir bin\fonts

echo *
echo Building system
echo *
@fasm system\calendar\trunk\calendar.asm bin\calendar
@fasm system\board\trunk\board.asm bin\develop\board
@fasm system\cpu\trunk\cpu.asm bin\cpu 
@fasm system\cpuid\trunk\cpuid.asm bin\cpuid
@fasm system\desktop\trunk\desktop.asm bin\desktop
@fasm system\docpack\trunk\docpack.asm bin\docpack
@fasm system\end\trunk\end.asm bin\end
@fasm system\gmon\gmon.asm bin\gmon
@fasm system\icon\trunk\icon.asm bin\icon
@fasm system\kbd\trunk\kbd.ASM bin\kbd
@fasm system\launcher\trunk\launcher.asm bin\launcher
@fasm system\menu\trunk\menu.asm bin\@menu
@fasm system\mgb\trunk\mgb.asm bin\mgb
@fasm system\PANEL\trunk\@PANEL.ASM bin\@PANEL
@fasm system\pcidev\trunk\pcidev.asm bin\pcidev
@fasm system\RB\trunk\@RB.ASM bin\@RB
@fasm system\rdsave\trunk\rdsave.asm bin\rdsave
@fasm system\run\trunk\run.asm bin\run
@fasm system\setup\trunk\setup.asm bin\setup
@fasm system\skinsel\skinsel.asm bin\skinsel
@fasm system\vrr\trunk\vrr.asm bin\vrr
@fasm system\vrr_m\trunk\vrr_m.asm bin\vrr_m

echo *
echo Building develop
echo *
@fasm develop\cmd\trunk\cmd.asm bin\cmd
@fasm develop\fasm\trunk\fasm.asm bin\develop\fasm
@fasm develop\h2d2b\trunk\h2d2b.asm bin\develop\h2d2b
@fasm develop\heed\trunk\heed.asm bin\demos\heed
rem @fasm develop\hexview\trunk\hexview.asm hexview
@fasm develop\keyascii\trunk\keyascii.asm bin\develop\keyascii
@fasm develop\mtdbg\mtdbg.asm bin\develop\mtdbg
rem @fasm develop\param\trunk\param.asm param
@fasm develop\scancode\trunk\scancode.asm bin\develop\scancode
@fasm develop\tinypad\trunk\tinypad.asm bin\tinypad

echo *
echo Building fs
echo *
@fasm fs\copy2\trunk\copy2.asm bin\copy2
@fasm fs\copyr\trunk\copyr.asm bin\copyr
@fasm fs\kfar\trunk\kfar.asm bin\kfar
rem @fasm fs\mfar\trunk\mfar.asm bin\mfar
@fasm fs\sysxtree\trunk\sysxtree.asm bin\sysxtree

echo *
echo Building network
echo *
@fasm network\airc\trunk\airc.asm bin\network\airc
@fasm network\arpstat\trunk\arpstat.asm bin\network\arpstat
@fasm network\autodhcp\trunk\autodhcp.asm bin\network\autodhcp
@fasm network\dhcp\trunk\dhcp.asm bin\network\dhcp
@fasm network\dnsr\trunk\dnsr.asm bin\network\dnsr
@fasm network\ethstat\trunk\ethstat.asm bin\network\ethstat
@fasm network\ftps\trunk\https.asm bin\network\ftps
@fasm network\httpc\trunk\httpc.asm bin\network\httpc
@fasm network\https\trunk\https.asm bin\network\https
@fasm network\ipc\trunk\ipc.asm bin\network\ipc
@fasm network\local\trunk\local.asm bin\network\local
@fasm network\mp3s\trunk\mp3s.asm bin\network\mp3s
@fasm network\netsendc\trunk\netsendc.asm bin\network\netsendc
@fasm network\netsends\trunk\netsends.asm bin\network\netsends
@fasm network\nntpc\trunk\nntpc.asm bin\network\nntpc
@fasm network\popc\trunk\popc.asm bin\network\popc
@fasm network\ppp\trunk\ppp.asm bin\network\ppp
@fasm network\rccc\trunk\rccc.asm bin\network\rccc
@fasm network\rccs\trunk\rccs.asm bin\network\rccs
@fasm network\remote\trunk\remote.asm bin\network\remote
@fasm network\smtps\trunk\smtps.asm bin\network\smtps
@fasm network\stackcfg\trunk\stackcfg.asm bin\network\stackcfg
@fasm network\telnet\trunk\telnet.asm bin\network\telnet
@fasm network\terminal\trunk\terminal.asm bin\network\terminal
@fasm network\tftpa\trunk\tftpa.asm bin\network\tftpa
@fasm network\tftpc\trunk\tftpc.asm bin\network\tftpc
@fasm network\VNCclient\VNCclient.asm bin\network\VNCclient
@fasm network\ym\trunk\ym.asm bin\network\ym

echo *
echo Building other
echo *
rem @fasm other\archer\trunk\@rcher.asm bin\@rcher
@fasm other\calc\trunk\calc.asm bin\calc
@fasm other\mhc\trunk\mhc.asm bin\mhc
@fasm other\period\trunk\period.asm bin\period
@fasm other\rtfread\trunk\rtfread.asm bin\rtfread

echo *
echo Building media
echo *
rem media\ac97snd\trunk\ac97snd.asm ac97snd
@fasm media\animage\trunk\animage.asm bin\animage
@fasm media\cdp\trunk\cdp.asm bin\cdp
@fasm media\gifview\trunk\gifview.asm bin\gifview
@fasm media\iconedit\trunk\iconedit.asm bin\iconedit
@fasm media\jpegview\trunk\jpegview.asm bin\jpegview
@fasm media\midamp\trunk\midamp.asm bin\midamp
@fasm media\midiplay\trunk\midiplay.asm bin\midiplay
@fasm media\mixer\trunk\mixer.asm bin\mixer

@fasm media\mv\trunk\mv.asm bin\mv
@fasm media\pic4\trunk\pic4.asm bin\pic4
@fasm media\sb\trunk\sb.asm bin\sb
@fasm media\scrshoot\scrshoot.asm bin\scrshoot

echo *
echo Building games
echo *
@fasm games\15\trunk\15.asm bin\games\15
@fasm games\arcanii\trunk\arcanii.asm bin\games\arcanii
@fasm games\arcanoid\trunk\arcanoid.asm bin\games\arcanoid
cd games\c4\trunk\
@nasmw -f bin -o ..\..\..\bin\games\c4 c4.asm
cd ..\..\..
@fasm games\chess\trunk\chess.asm bin\games\chess
@fasm games\freecell\freecell.asm bin\games\freecell
@fasm games\mblocks\trunk\mblocks.asm bin\games\mblocks
@fasm games\phenix\trunk\phenix.asm bin\games\phenix
@fasm games\pipes\pipes.asm bin\games\pipes
@fasm games\pong\trunk\pong.asm bin\games\pong
@fasm games\pong3\trunk\pong3.asm bin\games\pong3
@fasm games\tanks\trunk\tanks.asm bin\games\tanks
@fasm games\tetris\trunk\tetris.asm bin\games\tetris
rem @fasm games\hunter\trunk\hunter.asm bin\games\hunter

echo *
echo Building demos
echo *
@fasm demos\3dcube2\trunk\3dcube2.asm bin\3d\3dcube2
rem @fasm demos\3detx60b\trunk\3detx60b.asm bin\3d\3detx60b
@fasm demos\3dtcub10\trunk\3dtcub10.asm bin\3d\3dtcub10
cd demos\aclock\trunk\
@nasmw -t -f bin -o ..\..\..\bin\demos\aclock aclock.asm
cd ..\..\..
@fasm demos\bcdclk\bcdclk\bcdclk.asm bin\demos\bcdclk
@fasm demos\bgitest\trunk\bgitest.asm bin\fonts\bgitest
@fasm demos\colorref\trunk\colorref.asm bin\demos\colorref
@fasm demos\crownscr\trunk\crownscr.asm bin\3d\crownscr
@fasm demos\cslide\trunk\cslide.asm bin\demos\cslide
@fasm demos\eyes\trunk\eyes.asm bin\demos\eyes
@fasm demos\fire\trunk\fire.asm bin\demos\fire
@fasm demos\fire2\trunk\fire2.asm bin\demos\fire2
@fasm demos\free3d04\trunk\free3d04.asm bin\3d\free3d04
@fasm demos\magnify\trunk\magnify.asm bin\magnify
@fasm demos\movback\trunk\movback.asm bin\demos\movback
@fasm demos\plasma\trunk\plasma.asm bin\demos\plasma
@fasm demos\ss\trunk\@ss.asm bin\@ss
@fasm demos\timer\trunk\timer.asm bin\demos\timer
@fasm demos\tinyfrac\trunk\tinyfrac.asm bin\demos\tinyfrac
@fasm demos\transp\trunk\transp.asm bin\demos\transp
@fasm demos\trantest\trunk\trantest.asm bin\demos\trantest
@fasm demos\tube\trunk\tube.asm bin\demos\tube

@erase lang.inc

echo *
echo Finished building 
echo *

echo Kpack KolibriOS apps?
echo     [%kpack%]

set /P res=">

if "%res%"=="y" (

echo *
echo Compressing system
echo *
@kpack bin\calendar
@kpack bin\develop\board
@kpack bin\cpu 
@kpack bin\cpuid
@kpack bin\desktop
@kpack bin\docpack
@kpack bin\end
@kpack bin\gmon
@kpack bin\icon
@kpack bin\kbd
@kpack bin\launcher
@kpack bin\menu
@kpack bin\mgb
@kpack bin\@PANEL
@kpack bin\pcidev
@kpack bin\@RB
@kpack bin\rdsave
@kpack bin\run
@kpack bin\setup
@kpack bin\skinsel
@kpack bin\vrr
@kpack bin\vrr_m

echo *
echo Compressing develop
echo *

@kpack bin\cmd
@kpack bin\develop\fasm
@kpack bin\develop\h2d2b
@kpack bin\demos\heed
@kpack bin\develop\keyascii
@kpack bin\develop\mtdbg
@kpack bin\develop\scancode

echo *
echo Compressing fs
echo *

@kpack bin\copy2
@kpack bin\copyr
@kpack bin\kfar
@kpack bin\sysxtree

echo *
echo Compressing network
echo *

@kpack bin\network\airc
@kpack bin\network\arpstat
@kpack bin\network\autodhcp
@kpack bin\network\dhcp
@kpack bin\network\dnsr
@kpack bin\network\ethstat
@kpack bin\network\httpc
@kpack bin\network\https
@kpack bin\network\ipc
@kpack bin\network\local
@kpack bin\network\netsendc
@kpack bin\network\netsends
@kpack bin\network\nntpc
@kpack bin\network\popc
@kpack bin\network\ppp
@kpack bin\network\rccc
@kpack bin\network\rccs
@kpack bin\network\remote
@kpack bin\network\smtps
@kpack bin\network\stackcfg
@kpack bin\network\telnet
@kpack bin\network\terminal
@kpack bin\network\tftpa
@kpack bin\network\tftpc
@kpack bin\network\VNCclient
@kpack bin\network\ym

echo *
echo Compressing other
echo *

rem @kpack bin\@rcher
@kpack bin\calc
@kpack bin\mhc
@kpack bin\period
@kpack bin\rtfread

echo *
echo Compressing media
echo *

@kpack bin\animage
@kpack bin\cdp
@kpack bin\gifview
@kpack bin\iconedit
@kpack bin\jpegview
@kpack bin\midamp
@kpack bin\midiplay
@kpack bin\mixer
@kpack bin\mp3s
@kpack bin\mv
@kpack bin\pic4
@kpack bin\sb
@kpack bin\scrshoot

echo *
echo Compressing games
echo *

@kpack bin\games\15
@kpack bin\games\arcanii
@kpack bin\games\arcanoid
@kpack bin\games\chess
@kpack bin\games\freecell
@kpack bin\games\mblocks
@kpack bin\games\phenix
@kpack bin\games\pipes
@kpack bin\games\pong
@kpack bin\games\pong3
@kpack bin\games\tanks
@kpack bin\games\tetris

echo *
echo Compressing demos
echo *

@kpack bin\3d\3dcube2
@kpack bin\3d\3dtcub10
@kpack bin\demos\aclock
@kpack bin\demos\bcdclk
@kpack bin\fonts\bgitest
@kpack bin\demos\colorref
@kpack bin\3d\crownscr
@kpack bin\demos\cslide
@kpack bin\demos\eyes
@kpack bin\demos\fire
@kpack bin\demos\fire2
@kpack bin\3d\free3d04
@kpack bin\magnify
@kpack bin\demos\movback
@kpack bin\demos\plasma
@kpack bin\@ss
@kpack bin\demos\timer
@kpack bin\demos\tinyfrac
@kpack bin\demos\transp
@kpack bin\demos\trantest
@kpack bin\demos\tube

echo *
echo Compressing complete
echo *
)

pause
