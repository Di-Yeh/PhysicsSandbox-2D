@echo off
chcp 65001
setlocal enabledelayedexpansion

:: 检查 build 目录
if not exist build (
    mkdir build
    set NEED_CMAKE=Y
) else (
    echo ===========================================
    set /p NEED_CMAKE="是否需要重新配置 CMake? (修改了 CMakeLists 或增加了文件才选 Y) [Y/N]: "
)

cd build

:: 根据选择运行 CMake 配置
if /I "!NEED_CMAKE!"=="Y" (
    echo === 正在重新配置 CMake (请耐心等待) ===
    cmake ..
    if %errorlevel% neq 0 ( pause & exit /b )
)

:: 选择构建模式
echo.
echo ===========================================
echo 请选择构建模式: [1] Debug  [2] Release
set /p choice="输入数字: "

if "%choice%"=="1" ( set CONFIG=Debug ) else ( set CONFIG=Release )

:: 仅执行编译 (增量编译)
echo === 正在快速编译 !CONFIG! 版本 ===
cmake --build . --config !CONFIG!
if %errorlevel% neq 0 ( pause & exit /b )

:: 同步资源 (xcopy 只会复制更新过的文件)
if exist "..\assets" (
    xcopy /y /e /i /d "..\assets" "!CONFIG!\assets" > nul
)

:: 运行
echo === 启动程序: !CONFIG!\PhysicsSandbox.exe ===
cd !CONFIG!
start PhysicsSandbox.exe
exit /b