@echo off
setlocal

rem Tool source directory (where this bat lives)
set "TOOL_DIR=%~dp0"

rem Project root: tools\action_asset_resizer\..\.. -> project root
set "PROJECT_ROOT=%TOOL_DIR%..\.."

rem Resolve to absolute paths
pushd "%TOOL_DIR%"
set "TOOL_DIR=%CD%\"
popd
pushd "%PROJECT_ROOT%"
set "PROJECT_ROOT=%CD%"
popd

set "DIST_DIR=%PROJECT_ROOT%\dist"
set "BUILD_DIR=%TOOL_DIR%.pyinstaller_build\build"
set "SPEC_DIR=%TOOL_DIR%.pyinstaller_build"
set "OUTPUT_EXE=%DIST_DIR%\DesktopPet-resize.exe"

echo ============================================
echo DesktopPet Action Asset Resizer - Builder
echo ============================================
echo.
echo Tool dir:    %TOOL_DIR%
echo Project root: %PROJECT_ROOT%
echo Output:      %OUTPUT_EXE%
echo.

echo [1/3] Installing requirements...
python -m pip install -r "%TOOL_DIR%requirements.txt"
if errorlevel 1 (
    echo [ERROR] Failed to install requirements.
    exit /b 1
)

echo.
echo [2/3] Checking PyInstaller...
python -m PyInstaller --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] PyInstaller is not available.
    echo Please run: python -m pip install pyinstaller
    exit /b 1
)

echo.
echo [3/3] Building DesktopPet-resize.exe...

if not exist "%DIST_DIR%" mkdir "%DIST_DIR%"

python -m PyInstaller ^
    --noconsole ^
    --onefile ^
    --name DesktopPet-resize ^
    --distpath "%DIST_DIR%" ^
    --workpath "%BUILD_DIR%" ^
    --specpath "%SPEC_DIR%" ^
    "%TOOL_DIR%action_asset_resizer.py"
if errorlevel 1 (
    echo [ERROR] Build failed.
    exit /b 1
)

echo.
echo ============================================
echo Build complete.
echo Output: %OUTPUT_EXE%
echo ============================================
endlocal
