@echo off

REM This file was generated by Ember Desktop from the following template:
REM   app/ncp/meta-inf/template/em3xx/em3xx-postbuild.bat
REM Please do not edit it directly.

REM Post Build processing for IAR Workbench.

SET TARGET_BPATH=%*%

echo " "
echo "This converts S37 to Ember Bootload File format if a bootloader has been selected in AppBuilder"
echo " "
@echo on
cmd /C ""%ISA3_UTILS_DIR%\em3xx_convert.exe" $--em3xxConvertFlags--$ "%TARGET_BPATH%.s37" "%TARGET_BPATH%.ebl" > "%TARGET_BPATH%-em3xx-convert-output.txt""
@echo off
type "%TARGET_BPATH%-em3xx-convert-output.txt"