To patch X-Wing Alliance, extract the .bat and xwahacker.exe files
somewhere and then copy xwingalliance.exe from the X-Wing Alliance
installation directory there.
Then run the appropriate .bat file to apply or revert a patch,
see below for a list of what which one does.
When done, copy the xwingalliance.exe back, overwriting the original
(preferably after making a backup).
Only version 2.02 of xwingalliance.exe is supported.

The process is similar for other games like TIE95, XWing95 and
X-Wing vs. TIE/Balance of Power, though except that the file
to copy/replace is called TIE95.EXE, XWING95.EXE or Z_XVT__.EXE.

Interpreting the output:
> 'xwahacker' is not recognized as an internal or external command, operable program or batch file.

Means: the .bat file could not start the xwahacker.exe file.
Unless you forgot to extract that file to the same location
as the .bat files, I have no explanation for this.

> Could not detect file, assuming it is ...

Means: the file you are trying to patch is not supported at all

> Detected file as ...
> Could not find the previous patch state in patch group, no changes made

Means: the file you are trying to patch either is not supported or has been
modified in a way that xwahacker does not support.
To avoid breaking the file beyond repair, xwahacker aborted.


For X-Wing Alliance:
fixedclear.bat             Fixes disappearing objects in X-Wing Alliance
nofixedclear.bat           Reverts changes from fixedclear.bat

32bitmode.bat              Changes rendering to use 32 bit mode.
                           Allows using anti-aliasing, but breaks in-flight
                           menu and the briefing summary while loading the
                           mission.
                           Might also fix performance issues with some
                           graphics drivers.
16bitmode.bat              Reverts changes from 32bitmode.bat

changefps.bat              Allows changing the maximum FPS limit.
                           Useless since it only affects FPS in the main menu,
                           not for gameplay.

changeres.bat              Allows replacing one of the resolutions by another.
                           Note: this now also updates the HUD size and field
                           of view, by using xwahacker.exe directly you can
                           also change these any way you want.

force800mode.bat           Should force X-Wing Alliance to always use the
                           800x600 mode (the actual resolution can be changed
                           with changeres.bat). Not well tested, might not
                           work.
noforce800mode.bat         Reverts changes from force800mode.bat


For TIE Fighter 95 (for Windows):
tie95fixedclear.bat        Fixes disappearing objects in TIE Fighter for Windows
tie95nofixedclear.bat      Reverts changes from tie95fixedclear.bat


For X-Wing 95 (for Windows):
xwing95fixedclear.bat      Fixes disappearing objects in X-Wing for Windows
xwing95nofixedclear.bat    Reverts changes from xwing95fixedclear.bat

For X-Wing versus TIE Fighter, Balance of Power:
Tested against German version after installing BoP.
Patch z_xvt__.exe in XwingTie for XvT, patch the one in
XwingTie\BalanceOfPower for BoP.
xvtfixedclear.bat          Fixes disappearing objects
xvtnofixedclear.bat        Reverts changes from xvtfixedclear.bat
