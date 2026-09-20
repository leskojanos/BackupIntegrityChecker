@echo off
setlocal enabledelayedexpansion

echo ==========================================================
echo        BACKUP INTEGRITY CHECKER - Automata Build
echo ==========================================================

if "%VCPKG_ROOT%"=="" (
    echo [FIGYELMEZTETES] A VCPKG_ROOT valtozo nincs beallitva!
    set TOOLCHAIN_ARG=
) else (
    set TOOLCHAIN_ARG=-DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
)

if not exist "build" mkdir build

echo [+] CMake konfiguracio...
cmake -B build -S . !TOOLCHAIN_ARG!
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

echo.
echo [+] Forditas Release modban...
cmake --build build --config Release
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

echo.
if exist "build\Release\backup_integrity_checker.exe" (
    copy /Y "build\Release\backup_integrity_checker.exe" "backup_integrity_checker.exe" >nul
    echo [SIKER] .\backup_integrity_checker.exe elkeszult!
) else if exist "build\backup_integrity_checker.exe" (
    copy /Y "build\backup_integrity_checker.exe" "backup_integrity_checker.exe" >nul
    echo [SIKER] .\backup_integrity_checker.exe elkeszult!
)
endlocal
