@echo off
rem Launches Virtual CDJ. Prefers a Release build if one exists, otherwise
rem falls back to Debug. Any arguments passed to this script (e.g. an audio
rem file path) are forwarded to the application.
setlocal

set "EXE_RELEASE=%~dp0build\src\VirtualCDJ_artefacts\Release\Virtual CDJ.exe"
set "EXE_DEBUG=%~dp0build\src\VirtualCDJ_artefacts\Debug\Virtual CDJ.exe"

if exist "%EXE_RELEASE%" (
    start "" "%EXE_RELEASE%" %*
    goto :eof
)

if exist "%EXE_DEBUG%" (
    start "" "%EXE_DEBUG%" %*
    goto :eof
)

echo Virtual CDJ.exe nao foi encontrado.
echo Compile o projeto primeiro, por exemplo:
echo.
echo   cmake -S . -B build -G "Visual Studio 17 2022" -A x64
echo   cmake --build build --config Release --target VirtualCDJ
echo.
pause
