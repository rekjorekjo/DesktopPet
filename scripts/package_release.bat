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

rem resize.exe build failure: set to 1 to abort main packaging on failure
set "ABORT_ON_RESIZE_FAIL=0"

set "BUILD_RESIZE_EXE=%PROJECT_ROOT%\build\Desktop_Qt_6_11_0_MSVC2022_64bit-Release\DesktopPet-resize.exe"
set "RESIZE_EXE=%DIST_DIR%\DesktopPet-resize.exe"
set "RESIZE_BUILT=0"

echo ============================================
echo  DesktopPet Release Package Script
echo ============================================
echo.
echo Project root: %PROJECT_ROOT%
echo.

echo [1/7] Checking prerequisites...

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
echo [2/7] Checking resize tool...

if exist "%BUILD_RESIZE_EXE%" (
    set "RESIZE_BUILT=1"
    echo   DesktopPet-resize.exe: OK
) else (
    echo [WARN] DesktopPet-resize.exe not found at: %BUILD_RESIZE_EXE%
    echo   Please build Release in Qt Creator first (DesktopPet-resize target).
    if "!ABORT_ON_RESIZE_FAIL!"=="1" (
        echo   ABORT_ON_RESIZE_FAIL is set, aborting.
        goto error
    )
    echo   Skipping resize tool, continuing with main package.
)

echo.
echo [3/7] Preparing dist directory...

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
echo [4/7] Copying executable...

copy "%BUILD_EXE%" "%DIST_DIR%\%APP_NAME%.exe" >nul
if errorlevel 1 (
    echo ERROR: Failed to copy executable.
    goto error
)
echo   Copied: %APP_NAME%.exe

echo.
echo [5/7] Running windeployqt...

"%WINDEPLOYQT%" --release --compiler-runtime "%DIST_DIR%\%APP_NAME%.exe"
if errorlevel 1 (
    echo ERROR: windeployqt failed.
    goto error
)
echo   Qt dependencies deployed.

echo.
echo [6/7] Copying resources...

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

if "!RESIZE_BUILT!"=="1" (
    copy "%BUILD_RESIZE_EXE%" "%RESIZE_EXE%" >nul
    if errorlevel 1 (
        echo WARNING: Failed to copy DesktopPet-resize.exe.
    ) else (
        echo   Copied: DesktopPet-resize.exe
    )
)

echo.
echo [7/7] Creating ZIP package...

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
echo  Output folder:   %DIST_DIR%
echo  Main executable: %DIST_DIR%\%APP_NAME%.exe
if "!RESIZE_BUILT!"=="1" (
    echo  Resize tool:    %DIST_DIR%\DesktopPet-resize.exe [INCLUDED]
) else (
    echo  Resize tool:    [NOT INCLUDED]
)
echo  Zip package:     %ZIP_PATH%
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
