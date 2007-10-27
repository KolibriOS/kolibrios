@echo off

set FPRTL={FreePascal RTL source code, example c:\fp\src\rtl}
set INCS=-Fi%FPRTL%\inc;%FPRTL%\i386;%FPRTL%\objpas;%FPRTL%\objpas\sysutils;%FPRTL%\objpas\classes
set UNTS=-Fu%FPRTL%\inc;%FPRTL%\i386;%FPRTL%\objpas
set BUILDPATH=..\units
set FPCARGS=-n -Twin32 -Sge5 -O3pPENTIUM3 -CfSSE -di386 -FU%BUILDPATH% %INCS% %UNTS%

fpc system.pp -Us %FPCARGS%
if errorlevel 1 goto error

fpc %FPRTL%\objpas\objpas.pp %FPCARGS%
if errorlevel 1 goto error

fpc buildrtl.pp %FPCARGS%
if errorlevel 0 goto end

:error
echo An error occured while building RTL

:end
