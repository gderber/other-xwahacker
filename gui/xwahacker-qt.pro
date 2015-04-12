QT += widgets
CONFIG += release
CONFIG -= exceptions rtti
static {
QMAKE_LFLAGS += -static-libgcc -static-libstdc++
}
win32: QMAKE_LFLAGS += -Wl,--dynamicbase -Wl,--nxcompat

HEADERS = xwahacker-qt.h
SOURCES = main.cpp xwahacker-qt.cpp
