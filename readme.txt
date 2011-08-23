To patch X-Wing Alliance, copy the .bat and xwahacker.exe files into the
X-Wing Alliance installation directory (the one that contains
xwingalliance.exe).
Then run the appropriate .bat file to apply or revert a patch,
see below for a list of what which one does.
Only version 2.02 of xwingalliance.exe is supported.
If you are running Windows Vista (R), you may have to choose
"Run as Administrator" or in some other way work around UAC restrictions.

The file otherfixes.txt contains patch instructions for other
games like TIE95, XWing95 and X-Wing vs. TIE/Balance of Power,
though .bat files for those that automate the patching are included
as well.

Interpreting the output:
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

changefps.bat              Allows changing the maximum FPS limit

changeres.bat              Allows replacing one of the resolutions by another

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

For X-Wing versus TIE Fighter
(tested against German version after installing BoP):
xvtfixedclear.bat          Fixes disappearing objects
xvtnofixedclear.bat        Reverts changes from xvtfixedclear.bat

For Balance of Power:
(needs to be run in XwingTie\BalanceOfPower)
bopfixedclear.bat          Fixes disappearing objects
bopnofixedclear.bat        Reverts changes from bopfixedclear.bat
