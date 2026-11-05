@echo off
setlocal
cd /d "%~dp0"
where cl >nul 2>&1
if errorlevel 1 (
    echo Open the Developer Command Prompt for Visual Studio, then run build.bat.
    exit /b 1
)
if not exist build mkdir build
pushd build
cl /nologo /EHsc /std:c++17 /W4 /utf-8 ..\main.cpp ..\game.cpp ..\board.cpp ..\blocks.cpp ..\render.cpp ..\console.cpp /Fe:tetris.exe
set "BUILD_RESULT=%ERRORLEVEL%"
popd
exit /b %BUILD_RESULT%
