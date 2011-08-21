@echo off
xwahacker xwingalliance.exe -f
echo Enter new maximum fps limit
set /p fps=
xwahacker xwingalliance.exe -f %fps%
pause
exit
