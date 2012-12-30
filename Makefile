CC=gcc
CROSS_CC=i686-w64-mingw32-gcc
CFLAGS=-Wall -Wdeclaration-after-statement -Wpointer-arith -Wredundant-decls -Wcast-qual -Wwrite-strings -g -Os
CFLAGS+=-std=c99 -D_XOPEN_SOURCE=500
LDFLAGS=-lm
VERSION=2.1

all: xwahacker.exe xwareplacer.exe

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

%.exe: %.c
	$(CROSS_CC) -static $(CFLAGS) $(LDFLAGS) $^ -o $@

release: xwahacker-${VERSION}.zip

xwahacker-${VERSION}.zip: *.bat xwahacker.exe xwareplacer.exe readme.txt LICENSE xwahacker.c xwareplacer.c
	strip xwahacker.exe
	strip xwareplacer.exe
	7z a -mx=9 $@ $^

upload: xwahacker-${VERSION}.zip readme.txt
	scp $^ $(SFUSER),xwahacker@frs.sourceforge.net:/home/frs/project/x/xw/xwahacker

clean:
	rm -rf xwahacker xwahacker.exe xwahacker*.zip xwareplacer xwareplacer.exe

.PHONY: all clean release upload
