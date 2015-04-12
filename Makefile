CC=gcc
DIET=diet gcc -m32
CROSS_CC=i686-w64-mingw32-gcc
CFLAGS=-Wall -Wdeclaration-after-statement -Wpointer-arith -Wredundant-decls -Wcast-qual -Wwrite-strings -g -Os
# is supposedly the default, but obviously not on MinGW-w64
CFLAGS+=-Qn
CFLAGS+=-fomit-frame-pointer
CFLAGS+=-std=c99 -D_XOPEN_SOURCE=500
LDFLAGS=-lm
VERSION=2.5
GUI_VERSION=0.25

all: xwahacker.unsigned.exe xwareplacer.unsigned.exe xwahacker.static xwareplacer.static

%: %.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

%.static: %.c
	$(DIET) $(CFLAGS) $^ $(LDFLAGS) -s -o $@

# Simpler, safer but larger code build command:
#$(CROSS_CC) -static $(CFLAGS) -Wl,--nxcompat -Wl,--no-seh -Wl,--dynamicbase $^ $(LDFLAGS) -o $@
%.unsigned.exe: %.c
	$(CROSS_CC) $(CFLAGS) -Wl,--nxcompat -Wl,--no-seh -Wl,--dynamicbase -DNDEBUG -U_XOPEN_SOURCE -D__NO_ISOCEXT -nostdlib maincrtstartup.c $^ -lmsvcrt -lkernel32 -o $@

xwahacker-qt.unsigned.exe: gui/release/xwahacker-qt.exe
	cp $< $@

%.exe: %.unsigned.exe
	strip $^
	if [ -n "$(SIGNKEY)" ] ; then osslsigncode sign -ts http://www.startssl.com/timestamp -certs $(SIGNKEY).crt -key $(SIGNKEY).key -in $^ -out $@ ; chmod +x $@ ; else cp $^ $@ ; fi

release: xwahacker-${VERSION}.zip xwahacker-gui-${GUI_VERSION}-win.zip

xwahacker-gui-${GUI_VERSION}-win.zip: xwahacker-qt.exe LICENSE
	7z a -mx=9 $@ $^

xwahacker-${VERSION}.zip: *.bat xwahacker.exe xwareplacer.exe xwahacker.static xwareplacer.static readme.txt readme-linux.txt readme-xwareplacer.txt LICENSE xwahacker.c xwareplacer.c
	7z a -mx=9 $@ $^

upload: xwahacker-${VERSION}.zip readme.txt
	scp $^ $(SFUSER),xwahacker@frs.sourceforge.net:/home/frs/project/x/xw/xwahacker

gui_upload: xwahacker-gui-${GUI_VERSION}-win.zip
	scp $^ $(SFUSER),xwahacker@frs.sourceforge.net:/home/frs/project/x/xw/xwahacker

clean:
	rm -rf xwahacker xwahacker.exe xwahacker.unsigned.exe xwahacker.static xwahacker*.zip xwareplacer xwareplacer.unsigned.exe xwareplacer.exe xwareplacer.static xwahacker-qt.unsigned.exe xwahacker-qt.exe

.PHONY: all clean release upload
