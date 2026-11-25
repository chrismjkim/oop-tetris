@echo off
setlocal
if not "%~1"=="" (
    echo Usage: build.bat
    exit /b 2
)
cd /d "%~dp0"
set "VERSION=1.4"
set "EXE=tetris-kingdom-%VERSION%.exe"
if not exist build mkdir build
set "CORE=..\Augment.cpp ..\Block.cpp ..\BlockManager.cpp ..\BlockSpec.cpp ..\board.cpp ..\Card.cpp ..\CardInfo.cpp ..\Cell.cpp ..\DestroyedBlock.cpp ..\Landing.cpp ..\LineClear.cpp ..\PlacedBlock.cpp ..\Point.cpp ..\RemovedCell.cpp ..\StageManager.cpp ..\StageRules.cpp ..\Trigger.cpp"
set "UI=..\GameManager.cpp ..\DisplayManager.cpp"
set "CONSOLE=..\InputManager.cpp ..\ConsoleScreen.cpp"
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
pushd build
"%CXX%" -std=c++17 -DTETRIS_VERSION=%VERSION% -Wall -Wextra -Wpedantic -O2 -static ..\main.cpp %CORE% %UI% %CONSOLE% -o "%EXE%"
goto done

:msvc
pushd build
cl /nologo /EHsc /std:c++17 /DTETRIS_VERSION=%VERSION% /W4 /utf-8 ..\main.cpp %CORE% %UI% %CONSOLE% /Fe:"%EXE%"

:done
set "RESULT=%ERRORLEVEL%"
popd
exit /b %RESULT%
