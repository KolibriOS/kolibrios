set BINDIR=C:\Users\Кирилл\Desktop\cmm\_bin 
md %BINDIR%
cd %BINDIR%
rem FOR %%A in (*.*) do del \Q "%%A"

cd ..\browser
cls
call compile.bat
copy /Y htmlv %BINDIR%

cd ..\copyf
cls
call compile.bat
copy /Y copyf %BINDIR%

cd ..\dicty
cls
call compile.bat
copy /Y dicty %BINDIR%

cd ..\eolite
cls
call compile.bat
copy /Y eolite %BINDIR%

cd ..\example
cls
call compile.bat

cd ..\installer
cls
call compile.bat
copy /Y installer.kex %BINDIR%

cd ..\liza
cls
call compile.bat
copy /Y liza_mail %BINDIR%

cd ..\end
cls
call compile.bat
copy /Y end %BINDIR%

cd ..\notify
cls
call compile.bat
copy /Y @notify %BINDIR%

cd ..\rb
cls
call compile_rus.bat
copy /Y @rb %BINDIR%

cd ..\skinsel
cls
call compile.bat
copy /Y skinsel %BINDIR%

cd ..\tmpdisk
cls
call compile_ru.bat
copy /Y tmpdisk %BINDIR%

cd %BINDIR%
FOR %%A in (*.*) do ..\c--\kpack "%%A"
pause