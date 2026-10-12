@echo off
cd /d "%~dp0"
set "ADB=C:\Users\Hugo W\AppData\Local\Android\Sdk\platform-tools\adb.exe"
"%ADB%" devices
"%ADB%" uninstall com.MODART.Uproject
"%ADB%" install -r Uproject-arm64.apk
"%ADB%" shell mkdir -p /sdcard/Android/obb/com.MODART.Uproject
for %%f in (*.obb) do "%ADB%" push "%%f" /sdcard/Android/obb/com.MODART.Uproject/
"%ADB%" shell ls -l /sdcard/Android/obb/com.MODART.Uproject/
pause