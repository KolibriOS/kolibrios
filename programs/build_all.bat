@echo off
cls
goto MAIN


rem
rem %1 - variable name
rem %2-x - allowed values
rem
:input_value
   set __var_name=%1
   set __values=
   set __value=
   set __res=
   shift
   
  :__allowed
   set __values=%1 %__values%
   shift
   if not "%1"=="" goto __allowed
   
   set /P __res=">
  :Check_Value
   for %%a in (%__values%) do if %%a==%__res% set __value=%__res%
   if defined __value goto :__input_value_end

   echo Value '%__res%' is incorrect
   echo Enter valid value from [ %__values% ]:

   set /P __res=">
   goto Check_Value
   :__input_value_end
   set %__var_name%=%__value%
goto :eof




:MAIN
set languages=en ru ge et
set __CPU_type=p5 p6 k6
set BIN=bin

echo Build KolibriOS apps
echo Enter valid language
echo     [%languages%]
call :input_value res %languages%
echo lang fix %res% > lang.inc

echo Enter CPU_type ("p5" for interrupt, "p6" for SYSENTER, "k6" for SYSCALL)
call :input_value res %__CPU_type%
echo __CPU_type fix %res% > config.inc

for %%i in (%BIN% %BIN%\demos %BIN%\develop %BIN%\lib %BIN%\games %BIN%\network %BIN%\3d %BIN%\fonts %BIN%\nightbuild) do if not exist %%i mkdir %%i

echo *
echo Building system
echo *
fasm system\calendar\trunk\calendar.asm %BIN%\calendar
fasm system\board\trunk\board.asm %BIN%\develop\board
fasm system\commouse\trunk\commouse.asm %BIN%\commouse
fasm system\cpu\trunk\cpu.asm %BIN%\cpu 
fasm system\cpuid\trunk\cpuid.asm %BIN%\cpuid
fasm system\skincfg\trunk\skincfg.asm %BIN%\skincfg
fasm system\docpack\trunk\docpack.asm %BIN%\docpack
fasm system\end\trunk\end.asm %BIN%\end
fasm system\gmon\gmon.asm %BIN%\gmon
fasm system\icon\trunk\icon.asm %BIN%\icon
fasm system\kbd\trunk\kbd.ASM %BIN%\kbd
fasm system\launcher\trunk\launcher.asm %BIN%\launcher
fasm system\menu\trunk\menu.asm %BIN%\@menu
fasm system\mgb\trunk\mgb.asm %BIN%\mgb
fasm system\mousemul\trunk\mousemul.asm %BIN%\mousemul
fasm system\PANEL\trunk\@TASKBAR.ASM %BIN%\@TASKBAR
fasm system\pcidev\trunk\pcidev.asm %BIN%\pcidev
fasm system\RB\trunk\@RB.ASM %BIN%\@RB
fasm system\rdsave\trunk\rdsave.asm %BIN%\rdsave
fasm system\run\trunk\run.asm %BIN%\run
fasm system\setup\trunk\setup.asm %BIN%\setup
fasm system\skinsel\skinsel.asm %BIN%\skinsel
fasm system\ss\trunk\@ss.asm %BIN%\@ss
fasm system\vrr\trunk\vrr.asm %BIN%\vrr
fasm system\vrr_m\trunk\vrr_m.asm %BIN%\vrr_m
fasm system\zkey\trunk\zkey.asm %BIN%\zkey

echo *
echo Building develop
echo *
fasm develop\cmd\trunk\cmd.asm %BIN%\cmd
fasm develop\fasm\trunk\fasm.asm %BIN%\develop\fasm
fasm develop\h2d2b\trunk\h2d2b.asm %BIN%\develop\h2d2b
fasm develop\heed\trunk\heed.asm %BIN%\demos\heed
rem fasm develop\hexview\trunk\hexview.asm hexview
fasm develop\keyascii\trunk\keyascii.asm %BIN%\develop\keyascii
fasm develop\mtdbg\mtdbg.asm %BIN%\develop\mtdbg
rem fasm develop\param\trunk\param.asm param
fasm develop\scancode\trunk\scancode.asm %BIN%\develop\scancode
fasm develop\tinypad\trunk\tinypad.asm %BIN%\tinypad
fasm develop\cObj\trunk\cObj.asm %BIN%\develop\cObj

echo *
echo Building systems libraries
echo *
fasm develop\libraries\box_lib\trunk\box_lib.asm %BIN%\lib\box_lib.obj
fasm develop\libraries\console\console.asm %BIN%\lib\console.obj
fasm develop\libraries\libs-dev\libgfx\libgfx.asm %BIN%\lib\libgfx.obj
fasm develop\libraries\libs-dev\libimg\libimg.asm %BIN%\lib\libimg.obj
fasm develop\libraries\libs-dev\libini\libini.asm %BIN%\lib\libini.obj
fasm develop\libraries\libs-dev\libio\libio.asm %BIN%\lib\libio.obj

echo *
echo Building fs
echo *
fasm fs\copy2\trunk\copy2.asm %BIN%\copy2
fasm fs\copyr\trunk\copyr.asm %BIN%\copyr
fasm fs\kfar\trunk\kfar.asm %BIN%\kfar
rem fasm fs\mfar\trunk\mfar.asm %BIN%\mfar
fasm fs\sysxtree\trunk\sysxtree.asm %BIN%\sysxtree

echo *
echo Building network
echo *
fasm network\airc\trunk\airc.asm %BIN%\network\airc
fasm network\arpstat\trunk\arpstat.asm %BIN%\network\arpstat
fasm network\autodhcp\trunk\autodhcp.asm %BIN%\network\autodhcp
fasm network\chess\trunk\chess.asm %BIN%\network\chess
fasm network\dhcp\trunk\dhcp.asm %BIN%\network\dhcp
fasm network\dnsr\trunk\dnsr.asm %BIN%\network\dnsr
fasm network\ethstat\trunk\ethstat.asm %BIN%\network\ethstat
fasm network\ftps\trunk\ftps.asm %BIN%\network\ftps
fasm network\httpc\trunk\httpc.asm %BIN%\network\httpc
fasm network\https\trunk\https.asm %BIN%\network\https
fasm network\ipc\trunk\ipc.asm %BIN%\network\ipc
fasm network\local\trunk\local.asm %BIN%\network\local
fasm network\mp3s\trunk\mp3s.asm %BIN%\network\mp3s
fasm network\netsendc\trunk\netsendc.asm %BIN%\network\netsendc
fasm network\netsends\trunk\netsends.asm %BIN%\network\netsends
fasm network\nntpc\trunk\nntpc.asm %BIN%\network\nntpc
fasm network\popc\trunk\popc.asm %BIN%\network\popc
fasm network\ppp\trunk\ppp.asm %BIN%\network\ppp
fasm network\rccc\trunk\rccc.asm %BIN%\network\rccc
fasm network\rccs\trunk\rccs.asm %BIN%\network\rccs
fasm network\remote\trunk\remote.asm %BIN%\network\remote
fasm network\smtps\trunk\smtps.asm %BIN%\network\smtps
fasm network\stackcfg\trunk\stackcfg.asm %BIN%\network\stackcfg
fasm network\telnet\trunk\telnet.asm %BIN%\network\telnet
fasm network\terminal\trunk\terminal.asm %BIN%\network\terminal
fasm network\tftpa\trunk\tftpa.asm %BIN%\network\tftpa
fasm network\tftpc\trunk\tftpc.asm %BIN%\network\tftpc
fasm network\VNCclient\VNCclient.asm %BIN%\network\VNCclient
fasm network\ym\trunk\ym.asm %BIN%\network\ym

echo *
echo Building other
echo *
rem fasm other\archer\trunk\@rcher.asm %BIN%\@rcher
fasm other\calc\trunk\calc.asm %BIN%\calc
fasm other\mhc\trunk\mhc.asm %BIN%\mhc
fasm other\period\trunk\period.asm %BIN%\period
fasm other\rtfread\trunk\rtfread.asm %BIN%\rtfread

echo *
echo Building media
echo *
rem media\ac97snd\trunk\ac97snd.asm ac97snd
fasm media\animage\trunk\animage.asm %BIN%\animage
fasm media\cdp\trunk\cdp.asm %BIN%\cdp
fasm media\gifview\trunk\gifview.asm %BIN%\gifview
fasm media\iconedit\trunk\iconedit.asm %BIN%\iconedit
fasm media\jpegview\trunk\jpegview.asm %BIN%\jpegview
fasm media\midamp\trunk\midamp.asm %BIN%\midamp
fasm media\midiplay\trunk\midiplay.asm %BIN%\midiplay
fasm media\mixer\trunk\mixer.asm %BIN%\mixer

fasm media\mv\trunk\mv.asm %BIN%\mv
fasm media\pic4\trunk\pic4.asm %BIN%\pic4
fasm media\sb\trunk\sb.asm %BIN%\sb
fasm media\scrshoot\scrshoot.asm %BIN%\scrshoot

echo *
echo Building games
echo *
fasm games\15\trunk\15.asm %BIN%\games\15
fasm games\arcanii\trunk\arcanii.asm %BIN%\games\arcanii
fasm games\arcanoid\trunk\arcanoid.asm %BIN%\games\arcanoid
cd games\c4\trunk\
nasmw -f bin -o ..\..\..\%BIN%\games\c4 c4.asm
cd ..\..\..
fasm games\freecell\freecell.asm %BIN%\games\freecell
fasm games\mblocks\trunk\mblocks.asm %BIN%\games\mblocks
fasm games\phenix\trunk\phenix.asm %BIN%\games\phenix
fasm games\pipes\pipes.asm %BIN%\games\pipes
fasm games\pong\trunk\pong.asm %BIN%\games\pong
fasm games\pong3\trunk\pong3.asm %BIN%\games\pong3
fasm games\snake\snake.asm %BIN%\games\snake
copy games\snake\snake.ini %BIN%\games\snake.ini
fasm games\tanks\trunk\tanks.asm %BIN%\games\tanks
fasm games\tetris\trunk\tetris.asm %BIN%\games\tetris
rem fasm games\hunter\trunk\hunter.asm %BIN%\games\hunter

echo *
echo Building demos
echo *
fasm demos\3dcube2\trunk\3dcube2.asm %BIN%\3d\3dcube2
rem fasm demos\3detx60b\trunk\3detx60b.asm %BIN%\3d\3detx60b
fasm demos\3dtcub10\trunk\3dtcub10.asm %BIN%\3d\3dtcub10
cd demos\aclock\trunk\
nasmw -t -f bin -o ..\..\..\%BIN%\demos\aclock aclock.asm
cd ..\..\..
fasm demos\bcdclk\trunk\bcdclk.asm %BIN%\demos\bcdclk
fasm demos\bgitest\trunk\bgitest.asm %BIN%\fonts\bgitest
fasm demos\colorref\trunk\colorref.asm %BIN%\demos\colorref
fasm demos\crownscr\trunk\crownscr.asm %BIN%\3d\crownscr
fasm demos\cslide\trunk\cslide.asm %BIN%\demos\cslide
fasm demos\eyes\trunk\eyes.asm %BIN%\demos\eyes
fasm demos\fire\trunk\fire.asm %BIN%\demos\fire
fasm demos\fire2\trunk\fire2.asm %BIN%\demos\fire2
fasm demos\firework\trunk\firework.asm %BIN%\demos\firework
fasm demos\free3d04\trunk\free3d04.asm %BIN%\3d\free3d04
fasm demos\magnify\trunk\magnify.asm %BIN%\magnify
fasm demos\movback\trunk\movback.asm %BIN%\demos\movback
fasm demos\plasma\trunk\plasma.asm %BIN%\demos\plasma
fasm demos\timer\trunk\timer.asm %BIN%\demos\timer
fasm demos\tinyfrac\trunk\tinyfrac.asm %BIN%\demos\tinyfrac
fasm demos\transp\trunk\transp.asm %BIN%\demos\transp
fasm demos\trantest\trunk\trantest.asm %BIN%\demos\trantest
fasm demos\tube\trunk\tube.asm %BIN%\demos\tube
fasm demos\unvwater\trunk\unvwater.asm %BIN%\demos\unvwater

echo *
echo Building depend application for fdd's nightbuild
echo *
echo __nightbuild fix yes >> config.inc
fasm media\kiv\trunk\kiv.asm %BIN%\nightbuild\kiv
fasm media\scrshoot\scrshoot.asm %BIN%\nightbuild\scrshoot
fasm media\animage\trunk\animage.asm %BIN%\nightbuild\animage
fasm media\midamp\trunk\midamp.asm %BIN%\nightbuild\midamp
fasm develop\heed\trunk\heed.asm %BIN%\nightbuild\heed
fasm develop\tinypad\trunk\tinypad.asm %BIN%\nightbuild\tinypad
fasm system\skincfg\trunk\skincfg.asm %BIN%\nightbuild\skincfg
fasm system\hdd_info\trunk\hdd_info.asm %BIN%\nightbuild\hdd_info
fasm system\mgb\trunk\mgb.asm %BIN%\nightbuild\mgb
fasm system\rdsave\trunk\rdsave.asm %BIN%\nightbuild\rdsave
fasm other\kpack\trunk\kpack.asm %BIN%\nightbuild\kpack
fasm other\rtfread\trunk\rtfread.asm %BIN%\nightbuild\rtfread
;restore
echo __CPU_type fix %res% > config.inc
erase lang.inc

echo *
echo Finished building 
echo *


kpack /nologo 2> nul
if "%errorlevel%"=="9009" (
echo   *** NOTICE ***
echo If you want to pack all applications you may 
echo place "kpack" in accessible directory.
echo You can download that tool from http://diamondz.land.ru/
goto END
)

echo Kpack KolibriOS apps?
echo     

set /P res=[y/n]?

if "%res%"=="y" (

echo *
echo Compressing system
echo *
kpack %BIN%\calendar
kpack %BIN%\develop\board
kpack %BIN%\cpu 
kpack %BIN%\cpuid
kpack %BIN%\skincfg
kpack %BIN%\docpack
kpack %BIN%\end
kpack %BIN%\gmon
kpack %BIN%\icon
kpack %BIN%\kbd
kpack %BIN%\launcher
kpack %BIN%\menu
kpack %BIN%\mgb
kpack %BIN%\@TASKBAR
kpack %BIN%\pcidev
kpack %BIN%\@RB
kpack %BIN%\rdsave
kpack %BIN%\run
kpack %BIN%\setup
kpack %BIN%\skinsel
kpack %BIN%\@ss
kpack %BIN%\vrr
kpack %BIN%\vrr_m
kpack %BIN%\zkey

echo *
echo Compressing develop
echo *

kpack %BIN%\cmd
kpack %BIN%\develop\fasm
kpack %BIN%\develop\h2d2b
kpack %BIN%\demos\heed
kpack %BIN%\develop\keyascii
kpack %BIN%\develop\mtdbg
kpack %BIN%\develop\scancode

echo *
echo Compressing libraries
echo *

kpack %BIN%\lib\box_lib.obj
kpack %BIN%\lib\console.obj
kpack %BIN%\lib\libsgfx.obj
kpack %BIN%\lib\libimg.obj
kpack %BIN%\lib\libini.obj
kpack %BIN%\lib\libio.obj


echo *
echo Compressing fs
echo *

kpack %BIN%\copy2
kpack %BIN%\copyr
kpack %BIN%\kfar
kpack %BIN%\sysxtree

echo *
echo Compressing network
echo *

kpack %BIN%\network\airc
kpack %BIN%\network\arpstat
kpack %BIN%\network\autodhcp
kpack %BIN%\network\chess
kpack %BIN%\network\dhcp
kpack %BIN%\network\dnsr
kpack %BIN%\network\ethstat
kpack %BIN%\network\httpc
kpack %BIN%\network\https
kpack %BIN%\network\ipc
kpack %BIN%\network\local
kpack %BIN%\network\netsendc
kpack %BIN%\network\netsends
kpack %BIN%\network\nntpc
kpack %BIN%\network\popc
kpack %BIN%\network\ppp
kpack %BIN%\network\rccc
kpack %BIN%\network\rccs
kpack %BIN%\network\remote
kpack %BIN%\network\smtps
kpack %BIN%\network\stackcfg
kpack %BIN%\network\telnet
kpack %BIN%\network\terminal
kpack %BIN%\network\tftpa
kpack %BIN%\network\tftpc
kpack %BIN%\network\VNCclient
kpack %BIN%\network\ym

echo *
echo Compressing other
echo *

rem kpack %BIN%\@rcher
kpack %BIN%\calc
kpack %BIN%\mhc
kpack %BIN%\period
kpack %BIN%\rtfread

echo *
echo Compressing media
echo *

kpack %BIN%\animage
kpack %BIN%\cdp
kpack %BIN%\gifview
kpack %BIN%\iconedit
kpack %BIN%\jpegview
kpack %BIN%\midamp
kpack %BIN%\midiplay
kpack %BIN%\mixer
kpack %BIN%\mp3s
kpack %BIN%\mv
kpack %BIN%\pic4
kpack %BIN%\sb
kpack %BIN%\scrshoot

echo *
echo Compressing games
echo *

kpack %BIN%\games\15
kpack %BIN%\games\arcanii
kpack %BIN%\games\arcanoid
kpack %BIN%\games\freecell
kpack %BIN%\games\mblocks
kpack %BIN%\games\phenix
kpack %BIN%\games\pipes
kpack %BIN%\games\pong
kpack %BIN%\games\pong3
kpack %BIN%\games\tanks
kpack %BIN%\games\tetris

echo *
echo Compressing demos
echo *

kpack %BIN%\3d\3dcube2
kpack %BIN%\3d\3dtcub10
kpack %BIN%\demos\aclock
kpack %BIN%\demos\bcdclk
kpack %BIN%\fonts\bgitest
kpack %BIN%\demos\colorref
kpack %BIN%\3d\crownscr
kpack %BIN%\demos\cslide
kpack %BIN%\demos\eyes
kpack %BIN%\demos\fire
kpack %BIN%\demos\fire2
kpack %BIN%\demos\firework
kpack %BIN%\3d\free3d04
kpack %BIN%\magnify
kpack %BIN%\demos\movback
kpack %BIN%\demos\plasma
kpack %BIN%\demos\timer
kpack %BIN%\demos\tinyfrac
kpack %BIN%\demos\transp
kpack %BIN%\demos\trantest
kpack %BIN%\demos\tube
kpack %BIN%\demos\unvwater

echo *
echo Compressing nightbuild
echo *
kpack %BIN%\nightbuild\kiv
kpack %BIN%\nightbuild\scrshoot
kpack %BIN%\nightbuild\animage
kpack %BIN%\nightbuild\midamp
kpack %BIN%\nightbuild\heed
kpack %BIN%\nightbuild\tinypad
kpack %BIN%\nightbuild\skincfg
kpack %BIN%\nightbuild\hdd_info
kpack %BIN%\nightbuild\mgb
kpack %BIN%\nightbuild\rdsave
kpack %BIN%\nightbuild\kpack
kpack %BIN%\nightbuild\rtfread

echo *
echo Compressing complete
echo *
)

:END
echo *
echo Done. Thanks for your choice ;)
echo *
pause
