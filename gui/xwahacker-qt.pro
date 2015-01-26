QT += widgets
CONFIG += release
static {
QMAKE_LFLAGS += -static-libgcc -static-libstdc++
}

HEADERS = xwahacker-qt.h
SOURCES = main.cpp xwahacker-qt.cpp
