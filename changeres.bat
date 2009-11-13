@echo off
xwahacker xwingalliance.exe -r
set res=-1
echo Select resolution to change (0, 1, 2 or 3, anything else to abort).
set /p res=
if %res% == 0 goto good
if %res% == 1 goto good
if %res% == 2 goto good
if %res% == 3 goto good
goto bad
:good
echo Enter new horizontal resolution
set /p xres=
echo Enter new vertical resolution
set /p yres=
xwahacker xwingalliance.exe -r %res% %xres% %yres%
pause
exit

:bad
echo Not a valid resolution number.
pause
exit