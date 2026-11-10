@echo off
rem Ejecuta la ROM de Gens Invaders en BlastEm.
rem Uso: run.bat          -> solo corre out\rom.bin
rem      run.bat build    -> compila con SGDK y luego corre
setlocal
set BLASTEM=C:\Tools\BlastEm\blastem-win32-0.6.2\blastem.exe
if "%GDK%"=="" set GDK=C:\tools\SGDK
cd /d "%~dp0"
if /i "%~1"=="build" (
    "%GDK%\bin\make.exe" -f "%GDK%\makefile.gen" || exit /b 1
)
if not exist "out\rom.bin" (
    echo No existe out\rom.bin. Corre: run.bat build
    exit /b 1
)
start "" "%BLASTEM%" "%~dp0out\rom.bin"
