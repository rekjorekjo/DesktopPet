@echo off
setlocal enabledelayedexpansion

rem ============================================================
rem  DesktopPet Release Package Script
rem ============================================================
rem  Output: DesktopPet-vX.Y.Z-release.zip
rem ============================================================

rem === Version Configuration ===
set "APP_NAME=DesktopPet"
set "APP_VERSION=0.30.3"
set "RELEASE_DIR_NAME=%APP_NAME%-v%APP_VERSION%-release"
set "ZIP_NAME=%APP_NAME%-v%APP_VERSION%-release.zip"

rem === Path Configuration ===
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%..\.."
cd /d "%PROJECT_ROOT%"

set "QT_DIR=F:\Qt\6.11.0\msvc2022_64"
set "WINDEPLOYQT=%QT_DIR%\bin\windeployqt.exe"

set "BUILD_EXE=%PROJECT_ROOT%\build\Desktop_Qt_6_11_0_MSVC2022_64bit-Release\%APP_NAME%.exe"
set "BUILD_RESIZE_EXE=%PROJECT_ROOT%\build\Desktop_Qt_6_11_0_MSVC2022_64bit-Release\%APP_NAME%-resize.exe"

set "DIST_ROOT=%PROJECT_ROOT%\dist"
set "DIST_DIR=%DIST_ROOT%\%RELEASE_DIR_NAME%"
set "ZIP_PATH=%DIST_ROOT%\%ZIP_NAME%"

set "RESIZE_BUILT=0"

rem ============================================================
echo ============================================
echo  DesktopPet Release Package Script
echo ============================================
echo.
echo  Version:       %APP_VERSION%
echo  Project root:  %PROJECT_ROOT%
echo  Output dir:    %DIST_DIR%
echo  Output zip:    %ZIP_PATH%
echo.

rem ============================================================
echo [1/6] Checking prerequisites...
rem ============================================================

if not exist "%WINDEPLOYQT%" (
    echo ERROR: windeployqt.exe not found.
    echo        QT_DIR: %QT_DIR%
    echo        Expected: %WINDEPLOYQT%
    goto error
)
echo   windeployqt: OK

if not exist "%BUILD_EXE%" (
    echo ERROR: Release exe not found.
    echo        Please build Release in Qt Creator first.
    echo        Expected: %BUILD_EXE%
    goto error
)
echo   Release exe: OK

if exist "%BUILD_RESIZE_EXE%" (
    set "RESIZE_BUILT=1"
    echo   Resize tool: OK
) else (
    echo   Resize tool: [NOT FOUND] - will be skipped
)

rem ============================================================
echo.
echo [2/6] Preparing dist directory...
rem ============================================================

if exist "%DIST_DIR%" (
    echo   Removing old release folder...
    rmdir /s /q "%DIST_DIR%"
)

if exist "%ZIP_PATH%" (
    echo   Removing old zip package...
    del /q "%ZIP_PATH%"
)

if not exist "%DIST_ROOT%" mkdir "%DIST_ROOT%"
mkdir "%DIST_DIR%"

echo   Created: %DIST_DIR%

rem ============================================================
echo.
echo [3/6] Copying executable...
rem ============================================================

copy "%BUILD_EXE%" "%DIST_DIR%\%APP_NAME%.exe" >nul
if errorlevel 1 (
    echo ERROR: Failed to copy executable.
    goto error
)
echo   Copied: %APP_NAME%.exe

rem ============================================================
echo.
echo [4/6] Running windeployqt...
rem ============================================================

"%WINDEPLOYQT%" --release --compiler-runtime "%DIST_DIR%\%APP_NAME%.exe"
if errorlevel 1 (
    echo ERROR: windeployqt failed.
    goto error
)
echo   Qt dependencies deployed.

rem ============================================================
echo.
echo [5/6] Copying resources...
rem ============================================================

if exist "%PROJECT_ROOT%\resources" (
    echo   Copying resources...
    robocopy "%PROJECT_ROOT%\resources" "%DIST_DIR%\resources" /e /njh /njs /ndl /nc /ns >nul
    if errorlevel 8 (
        echo   WARNING: Failed to copy resources directory.
    ) else (
        echo   Resources: OK
    )
)

if exist "%PROJECT_ROOT%\config" (
    echo   Copying config...
    robocopy "%PROJECT_ROOT%\config" "%DIST_DIR%\config" /e /njh /njs /ndl /nc /ns >nul
    if errorlevel 8 (
        echo   WARNING: Failed to copy config directory.
    ) else (
        echo   Config: OK
    )
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
    copy "%BUILD_RESIZE_EXE%" "%DIST_DIR%\%APP_NAME%-resize.exe" >nul
    if errorlevel 1 (
        echo   WARNING: Failed to copy %APP_NAME%-resize.exe.
    ) else (
        echo   Copied: %APP_NAME%-resize.exe
    )
)

rem ============================================================
echo.
echo [6/6] Creating ZIP package...
rem ============================================================

powershell -NoProfile -ExecutionPolicy Bypass -Command "Compress-Archive -Path '%DIST_DIR%\*' -DestinationPath '%ZIP_PATH%' -Force"
if errorlevel 1 (
    echo ERROR: Failed to create ZIP package.
    goto error
)
echo   Created: %ZIP_NAME%

rem ============================================================
echo.
echo ============================================
echo  Package completed successfully!
echo ============================================
echo.
echo  Version:         %APP_VERSION%
echo  Output folder:   %DIST_DIR%
echo  Main executable: %DIST_DIR%\%APP_NAME%.exe
if "!RESIZE_BUILT!"=="1" (
    echo  Resize tool:    %DIST_DIR%\%APP_NAME%-resize.exe [INCLUDED]
) else (
    echo  Resize tool:    [NOT INCLUDED]
)
echo  Zip package:     %ZIP_PATH%
echo.

goto end

:error
echo.
echo ============================================
echo  Package FAILED!
echo ============================================
endlocal
exit /b 1

:end
endlocal
exit /b 0
