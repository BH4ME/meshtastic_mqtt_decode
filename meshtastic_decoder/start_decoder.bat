@echo off
chcp 65001 >nul
echo ================================
echo    Meshtastic MQTT 解码器
echo ================================
echo.
echo 正在启动解码器...
echo 如果出现入口点错误，请确保系统有足够权限
echo.
mqtt_decoder.exe
if errorlevel 1 (
    echo.
    echo 程序运行出错，请检查：
    echo 1. 是否有杀毒软件阻止运行
    echo 2. 是否有足够的系统权限
    echo 3. 文件是否完整
)
pause 