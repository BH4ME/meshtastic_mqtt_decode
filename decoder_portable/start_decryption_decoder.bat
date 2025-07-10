@echo off
echo ==========================================
echo   Meshtastic MQTT Decoder v2.0
echo       WITH DECRYPTION SUPPORT
echo ==========================================
echo This version attempts to decrypt message content!
echo ==========================================
echo.

if exist "mqtt_decoder_with_decryption.exe" (
    mqtt_decoder_with_decryption.exe
) else (
    echo ERROR: mqtt_decoder_with_decryption.exe not found!
    echo Please run build_with_decryption.bat to compile first.
    pause
    exit /b 1
)

echo.
echo Decoder finished.
pause 