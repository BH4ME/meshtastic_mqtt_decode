@echo off
echo ==========================================
echo   Meshtastic MQTT Decoder v2.0
echo       WITH DECRYPTION SUPPORT
echo ==========================================
echo.

echo Building decryption-enabled version...
g++ -static -static-libgcc -static-libstdc++ -o mqtt_decoder_with_decryption.exe src\mqtt_decoder_with_decryption.cpp
if errorlevel 1 (
    echo Failed to build decryption version!
    pause
    exit /b 1
)

echo.
echo ✅ Build completed successfully!
echo.
echo Run mqtt_decoder_with_decryption.exe to start the decoder.
echo This version will attempt to decrypt message content!
echo.
pause 