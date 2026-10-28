@echo off
setlocal
rem ---- 如本机 Qt 安装位置不同，修改下面一行 ----
set QTDIR=E:\Qt\6.7.2\mingw_64
set PATH=%QTDIR%\bin;%PATH%

if not exist "%~dp0build\Calculator.exe" (
    echo 尚未编译，请先运行 build.bat
    exit /b 1
)
start "" "%~dp0build\Calculator.exe"
