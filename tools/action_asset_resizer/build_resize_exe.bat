@echo off
REM DesktopPet Action Asset Resizer - 打包脚本
REM 将 action_asset_resizer.py 打包为 resize.exe

echo ============================================
echo  DesktopPet Action Asset Resizer - Builder
echo ============================================
echo.

cd /d "%~dp0"

echo [1/3] 检查 PyInstaller ...
pyinstaller --version >nul 2>&1
if errorlevel 1 (
    echo [错误] 未找到 PyInstaller，请先安装：
    echo   pip install -r requirements.txt
    pause
    exit /b 1
)
echo      PyInstaller 已就绪。

echo.
echo [2/3] 开始打包 ...
pyinstaller --noconsole --onefile --name resize action_asset_resizer.py
if errorlevel 1 (
    echo [错误] 打包失败。
    pause
    exit /b 1
)

echo.
echo [3/3] 打包完成！
echo      输出文件: dist\resize.exe
echo.
echo ============================================
pause
