@echo off
setlocal
cd /d "%~dp0"
set "VERSION=1.3"
set "EXE=tetris-kingdom-%VERSION%.exe"
if not exist build mkdir build
where cl >nul 2>&1
if not errorlevel 1 goto msvc
where g++ >nul 2>&1
if not errorlevel 1 (
    set "CXX=g++"
    goto mingw
)
if exist "C:\msys64\ucrt64\bin\g++.exe" (
    set "CXX=C:\msys64\ucrt64\bin\g++.exe"
    set "PATH=C:\msys64\ucrt64\bin;%PATH%"
    goto mingw
)
echo Install a C++17 compiler or open a Visual Studio Developer Command Prompt.
exit /b 1

:mingw
if /I "%~1"=="test" goto mingw_tests
"%CXX%" -std=c++17 -DTETRIS_VERSION=%VERSION% -Wall -Wextra -Wpedantic -O2 -static main.cpp game.cpp mechanics.cpp board.cpp blocks.cpp render.cpp console.cpp -o "build\%EXE%"
exit /b %ERRORLEVEL%

:mingw_tests
"%CXX%" -std=c++17 -Wall -Wextra -Wpedantic -g -static tests\board_tests.cpp mechanics.cpp board.cpp blocks.cpp -o build\board_tests.exe
if errorlevel 1 exit /b 1
"%CXX%" -std=c++17 -DTETRIS_VERSION=%VERSION% -Wall -Wextra -Wpedantic -g -static tests\game_smoke.cpp game.cpp mechanics.cpp board.cpp blocks.cpp render.cpp -o build\game_smoke.exe
if errorlevel 1 exit /b 1
goto run_tests

:msvc
pushd build
if /I "%~1"=="test" goto msvc_tests
cl /nologo /EHsc /std:c++17 /DTETRIS_VERSION=%VERSION% /W4 /utf-8 ..\main.cpp ..\game.cpp ..\mechanics.cpp ..\board.cpp ..\blocks.cpp ..\render.cpp ..\console.cpp /Fe:"%EXE%"
set "RESULT=%ERRORLEVEL%"
popd
exit /b %RESULT%

:msvc_tests
cl /nologo /EHsc /std:c++17 /W4 /utf-8 ..\tests\board_tests.cpp ..\mechanics.cpp ..\board.cpp ..\blocks.cpp /Fe:board_tests.exe
if errorlevel 1 goto test_failure
cl /nologo /EHsc /std:c++17 /DTETRIS_VERSION=%VERSION% /W4 /utf-8 ..\tests\game_smoke.cpp ..\game.cpp ..\mechanics.cpp ..\board.cpp ..\blocks.cpp ..\render.cpp /Fe:game_smoke.exe
if errorlevel 1 goto test_failure
popd
goto run_tests
:test_failure
popd
exit /b 1

:run_tests
build\board_tests.exe
if errorlevel 1 exit /b 1
build\game_smoke.exe
exit /b %ERRORLEVEL%
