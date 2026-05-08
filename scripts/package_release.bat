@echo off
setlocal enabledelayedexpansion

set "APP_NAME=DesktopPet"

rem Always switch to project root: scripts\..
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."
cd /d "%PROJECT_ROOT%"

set "QT_DIR=F:\Qt\6.11.0\msvc2022_64"
set "WINDEPLOYQT=%QT_DIR%\bin\windeployqt.exe"

set "BUILD_EXE=%PROJECT_ROOT%\build\Desktop_Qt_6_11_0_MSVC2022_64bit-Release\DesktopPet.exe"

set "DIST_ROOT=%PROJECT_ROOT%\dist"
set "DIST_DIR=%DIST_ROOT%\DesktopPet"
set "ZIP_NAME=DesktopPet-release.zip"
set "ZIP_PATH=%DIST_ROOT%\%ZIP_NAME%"

echo ============================================
echo  DesktopPet Release Package Script
echo ============================================
echo.
echo Project root: %PROJECT_ROOT%
echo.

echo [1/6] Checking prerequisites...

if not exist "%WINDEPLOYQT%" (
    echo ERROR: windeployqt.exe not found.
    echo Current QT_DIR: %QT_DIR%
    goto error
)
echo   windeployqt: OK

if not exist "%BUILD_EXE%" (
    echo ERROR: Release exe not found.
    echo Please build Release in Qt Creator first.
    echo Current BUILD_EXE: %BUILD_EXE%
    goto error
)
echo   Release exe: OK

echo.
echo [2/6] Preparing dist directory...

if exist "%DIST_DIR%" (
    echo   Removing old dist folder...
    rmdir /s /q "%DIST_DIR%"
)

if exist "%ZIP_PATH%" (
    echo   Removing old zip package...
    del /q "%ZIP_PATH%"
)

if not exist "%DIST_ROOT%" mkdir "%DIST_ROOT%"
mkdir "%DIST_DIR%"

echo.
echo [3/6] Copying executable...

copy "%BUILD_EXE%" "%DIST_DIR%\%APP_NAME%.exe" >nul
if errorlevel 1 (
    echo ERROR: Failed to copy executable.
    goto error
)
echo   Copied: %APP_NAME%.exe

echo.
echo [4/6] Running windeployqt...

"%WINDEPLOYQT%" --release --compiler-runtime "%DIST_DIR%\%APP_NAME%.exe"
if errorlevel 1 (
    echo ERROR: windeployqt failed.
    goto error
)
echo   Qt dependencies deployed.

echo.
echo [5/6] Copying resources...

if exist "%PROJECT_ROOT%\resources" (
    echo   Copying resources...
    robocopy "%PROJECT_ROOT%\resources" "%DIST_DIR%\resources" /e /njh /njs /ndl /nc /ns >nul
    if errorlevel 8 echo WARNING: Failed to copy resources directory.
)

if exist "%PROJECT_ROOT%\config" (
    echo   Copying config...
    robocopy "%PROJECT_ROOT%\config" "%DIST_DIR%\config" /e /njh /njs /ndl /nc /ns >nul
    if errorlevel 8 echo WARNING: Failed to copy config directory.
)

if exist "%PROJECT_ROOT%\README.md" (
    copy "%PROJECT_ROOT%\README.md" "%DIST_DIR%\README.md" >nul
    echo   Copied: README.md
)

if exist "%PROJECT_ROOT%\LICENSE" (
    copy "%PROJECT_ROOT%\LICENSE" "%DIST_DIR%\LICENSE" >nul
    echo   Copied: LICENSE
)

echo.
echo [6/6] Creating ZIP package...

powershell -NoProfile -ExecutionPolicy Bypass -Command "Compress-Archive -Path '%DIST_DIR%\*' -DestinationPath '%ZIP_PATH%' -Force"
if errorlevel 1 (
    echo ERROR: Failed to create ZIP package.
    goto error
)

echo.
echo ============================================
echo  Package completed.
echo ============================================
echo.
echo  Output folder: %DIST_DIR%
echo  Zip package:   %ZIP_PATH%
echo.

goto end

:error
echo.
echo ============================================
echo  Package FAILED.
echo ============================================
endlocal
exit /b 1

:end
endlocal
exit /b 0