@echo off
rem Build uPDF for KolibriOS: third-party libs from the SDK, the MuPDF
rem libraries (fitz, pdf, draw) and the application itself.
rem Needs only the kos32-gcc toolchain, no make.
rem
rem Usage: build_libs.bat        - build missing libraries, then the app
rem        build_libs.bat all    - force-rebuild everything

setlocal
cd /d "%~dp0"

rem kos32-gcc toolchain location (override with the KOS32_TOOL variable)
if "%KOS32_TOOL%"=="" set KOS32_TOOL=C:\MinGW\msys\1.0\home\autobuild\tools\win32
set PATH=%KOS32_TOOL%\bin;%PATH%
where kos32-gcc >nul 2>nul
if errorlevel 1 (
	echo error: kos32-gcc not found, set KOS32_TOOL
	exit /b 1
)

set UPDF=%CD%
for %%i in ("%CD%\..\..\sdk") do set SDK=%%~fi
set CFLAGS=-c -fno-ident -O2 -fomit-frame-pointer -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32
set FORCE=%1

set INC=-DHAVE_CONFIG_H -I "%SDK%\sources\newlib\libc\include" -I "%SDK%\sources\freetype\include" -I "%SDK%\sources\libpng" -I "%SDK%\sources\zlib" -I .
call :lib "%SDK%\sources\libjbig2dec" libjbig2dec.a "%SDK%\lib"
if errorlevel 1 exit /b 1

set INC=-I "%SDK%\sources\newlib\libc\include" -I "%SDK%\sources\freetype\include" -I "%SDK%\sources\zlib" -I .
call :lib "%SDK%\sources\libopenjpeg" libopenjpeg.a "%SDK%\lib"
if errorlevel 1 exit /b 1

set INC=-I "%SDK%\sources\newlib\libc\include" -I "%SDK%\sources\freetype\include" -I "%SDK%\sources\libjpeg" -I "%SDK%\sources\zlib" -I "%SDK%\sources\libopenjpeg" -I "%SDK%\sources\libjbig2dec"
call :lib "%UPDF%\fitz" libfitz.a "%UPDF%\lib"
if errorlevel 1 exit /b 1

set INC=-I "%SDK%\sources\newlib\libc\include" -I "%UPDF%\fitz" -I "%SDK%\sources\freetype\include"
call :lib "%UPDF%\pdf" libmupdf.a "%UPDF%\lib"
if errorlevel 1 exit /b 1

set INC=-I "%SDK%\sources\newlib\libc\include" -I "%UPDF%\fitz"
call :lib "%UPDF%\draw" libdraw.a "%UPDF%\lib"
if errorlevel 1 exit /b 1

echo building updf...
cd /d "%UPDF%\apps"
del /q *.o updf 2>nul
for %%f in (kolibri.c kos_main.c pdfapp.c) do (
	kos32-gcc %CFLAGS% -I "%SDK%\sources\newlib\libc\include" -I "%SDK%\sources\freetype\include" -I "%SDK%\sources\zlib" -I ..\fitz -I ..\pdf -o "%%~nf.o" "%%f"
	if errorlevel 1 exit /b 1
)
kos32-ld -static -nostdlib -T "%SDK%\sources\newlib\app.lds" --image-base 0 -L "%SDK%\lib" -L "%KOS32_TOOL%\lib" -L ..\lib --subsystem native -o updf kolibri.o pdfapp.o kos_main.o -lmupdf -lfitz -lgcc -lfitz -ldraw -ljpeg -ljbig2dec -lfreetype -lopenjpeg -lz.dll -lc.dll
if errorlevel 1 exit /b 1
kos32-objcopy updf -O binary
del /q *.o
echo done: apps\updf
exit /b 0

:lib
rem %1 = source dir, %2 = library name, %3 = destination dir; uses %INC%
if not "%FORCE%"=="all" if exist "%~3\%~2" (
	echo skip %~2 ^(already in %~3^)
	exit /b 0
)
echo building %~2...
pushd "%~1"
del /q *.o 2>nul
for %%f in (*.c) do (
	kos32-gcc %CFLAGS% %INC% -o "%%~nf.o" "%%f"
	if errorlevel 1 (
		popd
		exit /b 1
	)
)
kos32-ar rcs %~2 *.o
if errorlevel 1 (
	popd
	exit /b 1
)
del /q *.o
if not exist "%~3" mkdir "%~3"
move /y "%~2" "%~3\" >nul
popd
exit /b 0
