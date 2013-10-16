Sorry, no proper documentation yet.
To compile, just type "make xwahacker".
To enable the fix for keys not working in the hangar in Wine, use
./xwahacker path/to/xwingalliance.exe -p 71

Other various improvements:
32 bit graphics (corrupts load screen):
./xwahacker path/to/xwingalliance.exe -c 1
Undo that:
./xwahacker path/to/xwingalliance.exe -c 0
Replace resolution, for example play in 1920x1080 if 800x600 was selected in menu:
./xwahacker path/to/xwingalliance.exe -r 1 1920 1080
