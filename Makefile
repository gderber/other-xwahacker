CC=gcc
DIET=diet gcc -m32
CROSS_CC=i686-w64-mingw32-gcc
CFLAGS=-Wall -Wdeclaration-after-statement -Wpointer-arith -Wredundant-decls -Wcast-qual -Wwrite-strings -g -Os
CFLAGS+=-std=c99 -D_XOPEN_SOURCE=500
LDFLAGS=-lm
VERSION=2.3

all: xwahacker.unsigned.exe xwareplacer.unsigned.exe xwahacker.static xwareplacer.static

%: %.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

%.static: %.c
	$(DIET) $(CFLAGS) $^ $(LDFLAGS) -s -o $@

%.unsigned.exe: %.c
	$(CROSS_CC) -static $(CFLAGS) $^ $(LDFLAGS) -o $@

%.exe: %.unsigned.exe
	strip $^
	if [ -n "$(SIGNKEY)" ] ; then osslsigncode sign -ts http://www.startssl.com/timestamp -certs $(SIGNKEY).crt -key $(SIGNKEY).key -in $^ -out $@ ; chmod +x $@ ; else cp $^ $@ ; fi

release: xwahacker-${VERSION}.zip

xwahacker-${VERSION}.zip: *.bat xwahacker.exe xwareplacer.exe xwahacker.static xwareplacer.static readme.txt readme-linux.txt readme-xwareplacer.txt LICENSE xwahacker.c xwareplacer.c
	7z a -mx=9 $@ $^

upload: xwahacker-${VERSION}.zip readme.txt
	scp $^ $(SFUSER),xwahacker@frs.sourceforge.net:/home/frs/project/x/xw/xwahacker

clean:
	rm -rf xwahacker xwahacker.exe xwahacker.unsigned.exe xwahacker.static xwahacker*.zip xwareplacer xwareplacer.unsigned.exe xwareplacer.exe xwareplacer.static

.PHONY: all clean release upload
