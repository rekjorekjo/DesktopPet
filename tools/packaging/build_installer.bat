@echo off
setlocal enabledelayedexpansion

rem ============================================================
rem  DesktopPet Inno Setup Build Script
rem ============================================================

set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%..\.."
cd /d "%PROJECT_ROOT%"

set /p APP_VERSION=<"%PROJECT_ROOT%\VERSION.txt"
if "%APP_VERSION%"=="" (
    echo ERROR: VERSION.txt file not found or empty.
    goto error
)

set "DIST_DIR=%PROJECT_ROOT%\dist\DesktopPet-v%APP_VERSION%-release"
set "ISS_FILE=%SCRIPT_DIR%DesktopPet.iss"
set "ISCC=F:\Inno Setup 6\ISCC.exe"
set "OUTPUT_EXE=%PROJECT_ROOT%\dist\DesktopPet_Setup_v%APP_VERSION%.exe"

echo ============================================
echo  DesktopPet Inno Setup Build Script
echo ============================================
echo.
echo  Version:    %APP_VERSION%
echo  Dist dir:   %DIST_DIR%
echo  ISS file:   %ISS_FILE%
echo  Output:     %OUTPUT_EXE%
echo.

rem ============================================================
echo [1/4] Checking prerequisites...
rem ============================================================

if not exist "%ISCC%" (
    echo ERROR: ISCC.exe not found: %ISCC%
    echo Please edit ISCC path in tools\packaging\build_installer.bat.
    goto error
)
echo   ISCC: OK

if not exist "F:\Inno Setup 6\Languages\ChineseSimplified.isl" (
    echo ERROR: ChineseSimplified.isl not found.
    echo Please install or copy ChineseSimplified.isl to:
    echo F:\Inno Setup 6\Languages\ChineseSimplified.isl
    goto error
)
echo   ChineseSimplified.isl: OK

if not exist "%DIST_DIR%\DesktopPet.exe" (
    echo ERROR: DesktopPet.exe not found in dist directory.
    echo        Please run package_release.bat first:
    echo        tools\packaging\package_release.bat
    goto error
)
echo   DesktopPet.exe: OK

if not exist "%DIST_DIR%\DesktopPet-resize.exe" (
    echo ERROR: DesktopPet-resize.exe not found in dist directory.
    echo        Please run package_release.bat first:
    echo        tools\packaging\package_release.bat
    goto error
)
echo   DesktopPet-resize.exe: OK

rem ============================================================
echo.
echo [2/4] Checking for user data in dist...
rem ============================================================

set "HAS_WARNING=0"

if exist "%DIST_DIR%\pets" (
    echo   WARNING: pets\ folder found in dist - will be excluded from installer.
    set "HAS_WARNING=1"
)

if exist "%DIST_DIR%\actions" (
    echo   WARNING: actions\ folder found in dist - will be excluded from installer.
    set "HAS_WARNING=1"
)

if "!HAS_WARNING!"=="0" (
    echo   No user data found in dist. OK.
)

rem ============================================================
echo.
echo [3/4] Compiling installer...
rem ============================================================

"%ISCC%" "%PROJECT_ROOT%\tools\packaging\DesktopPet.iss" /O"%PROJECT_ROOT%\dist" /DMyAppVersion="%APP_VERSION%"
if errorlevel 1 (
    echo ERROR: Inno Setup compilation failed.
    goto error
)

rem ============================================================
echo.
echo [4/4] Generating latest.json manifest...
rem ============================================================

set "LATEST_JSON=%PROJECT_ROOT%\dist\latest.json"

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "$v = '%APP_VERSION%'; " ^
    "$out = $env:LATEST_JSON; " ^
    "$now = [DateTime]::UtcNow.ToString('yyyy-MM-ddTHH:mm:ssZ'); " ^
    "$data = [ordered]@{ " ^
    "    version = $v; " ^
    "    tag = ('v' + $v); " ^
    "    releaseUrl = ('https://github.com/rekjorekjo/DesktopPet/releases/tag/v' + $v); " ^
    "    installerUrl = ('https://github.com/rekjorekjo/DesktopPet/releases/download/v' + $v + '/DesktopPet_Setup_v' + $v + '.exe'); " ^
    "    installerName = ('DesktopPet_Setup_v' + $v + '.exe'); " ^
    "    publishedAt = $now; " ^
    "    notes = 'Stability fixes' " ^
    "}; " ^
    "$json = $data | ConvertTo-Json -Depth 3; " ^
    "$utf8NoBom = New-Object System.Text.UTF8Encoding($false); " ^
    "[System.IO.File]::WriteAllText($out, $json, $utf8NoBom)"

if errorlevel 1 (
    echo ERROR: Failed to generate latest.json.
    goto error
)

if not exist "%LATEST_JSON%" (
    echo ERROR: latest.json was not created.
    goto error
)

echo   latest.json: OK

echo.
echo ============================================
echo  Build completed successfully!
echo ============================================
echo.
echo  Installer: %OUTPUT_EXE%
echo  Manifest:  %LATEST_JSON%
echo.

goto end

:error
echo.
echo ============================================
echo  Build FAILED!
echo ============================================
endlocal
exit /b 1

:end
endlocal
exit /b 0
