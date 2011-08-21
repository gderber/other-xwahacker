CC=gcc
CROSS_CC=i686-w64-mingw32-gcc
CFLAGS=-Wall -Wdeclaration-after-statement -Wpointer-arith -Wredundant-decls -Wcast-qual -Wwrite-strings -g -Os
CFLAGS+=-std=c99 -D_XOPEN_SOURCE=500

all: xwahacker.exe

xwahacker: xwahacker.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

xwahacker.exe: xwahacker.c
	$(CROSS_CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

release: xwahacker.zip

xwahacker.zip: *.bat xwahacker.exe readme.txt otherfixes.txt LICENSE xwahacker.c
	strip xwahacker.exe
	7z a -mx=9 $@ $^

upload: xwahacker.zip
	scp $^ $(SFUSER),xwahacker@frs.sourceforge.net:/home/frs/project/x/xw/xwahacker

clean:
	rm -rf xwahacker xwahacker.exe xwahacker.zip

.PHONY: all clean release upload
