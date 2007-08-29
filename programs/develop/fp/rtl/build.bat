@echo off

set FPRTL={path to original freepascal rtl source code, example ... \fp\src\rtl}
set INCS=-Fi%FPRTL%\inc;%FPRTL%\i386;%FPRTL%\objpas;%FPRTL%\objpas\sysutils;%FPRTL%\objpas\classes
set UNTS=-Fu%FPRTL%\inc;%FPRTL%\i386;%FPRTL%\objpas
set FPCARGS=-Twin32 -Se5 -Sg -n -O3pPENTIUM3 -CfSSE -di386 -FU..\units %INCS% %UNTS%

fpc system.pp -Us %FPCARGS%
if errorlevel 1 goto error

fpc %FPRTL%\objpas\objpas.pp %FPCARGS%
if errorlevel 1 goto error

fpc buildrtl.pp %FPCARGS%
if errorlevel 0 goto end

:error
echo An error occured while building RTL

:end
