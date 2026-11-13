@echo off
rem Ejecuta la ROM de Gens Invaders en BlastEm.
rem Uso: run.bat          -> solo corre out\rom.bin
rem      run.bat build    -> genera src\version.h, compila con SGDK y luego corre
setlocal
set BLASTEM=C:\Tools\BlastEm\blastem-win32-0.6.2\blastem.exe
if "%GDK%"=="" set GDK=C:\tools\SGDK
cd /d "%~dp0"
if /i "%~1"=="build" (
    call :version
    rem Varios modulos incluyen GAME_VERSION via game.h; hay que borrar todos los .o.
    del /q "out\src\*.o" >nul 2>&1
    "%GDK%\bin\make.exe" -f "%GDK%\makefile.gen" || exit /b 1
)
if not exist "out\rom.bin" (
    echo No existe out\rom.bin. Corre: run.bat build
    exit /b 1
)
start "" "%BLASTEM%" "%~dp0out\rom.bin"
exit /b 0

:version
rem Version = V0.3.xx, xx = numero de commits del repo (2 digitos minimo).
set BUILD=00
for /f %%c in ('git rev-list --count HEAD 2^>nul') do set BUILD=%%c
if %BUILD% LSS 10 set BUILD=0%BUILD%
rem Python evita el espacio final que deja echo y escribe CRLF exacto.
python -c "from pathlib import Path; Path('src/version.h').write_bytes('#define GAME_VERSION \"V0.3.%BUILD%\"\r\n'.encode())"
echo Version: V0.3.%BUILD%
exit /b 0
