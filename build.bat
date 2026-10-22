@echo off
setlocal
echo ====================================================
echo  Building C++ OOP Car Rental Server (Crow + MinGW-w64)
echo ====================================================

set "WINGET_GCC=C:\Users\dnyan\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin"
if exist "%WINGET_GCC%\g++.exe" (
    set "PATH=%WINGET_GCC%;%PATH%"
)

g++ --version
if errorlevel 1 (
    echo Error: g++ compiler not found!
    exit /b 1
)

echo Compiling src/RentalSystem.cpp and src/main.cpp...
g++ -std=c++17 -O2 -static -I./include src/RentalSystem.cpp src/main.cpp -o server.exe -lws2_32 -lmswsock
if errorlevel 1 (
    echo [ERROR] Compilation failed!
    exit /b 1
)

echo [SUCCESS] server.exe built successfully!
exit /b 0
