@echo off
setlocal
echo ====================================================
echo  Starting C++ Car Rental Management System
echo ====================================================

if not exist "server.exe" (
    echo server.exe not found! Running build.bat first...
    call build.bat
    if errorlevel 1 (
        echo Build failed! Exiting.
        pause
        exit /b 1
    )
)

echo Starting Crow server at http://localhost:18080...
start "" "http://localhost:18080"
server.exe
