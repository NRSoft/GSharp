
QT -= core gui

TARGET = gsharp
TEMPLATE = lib
CONFIG += staticlib

DESTDIR = ../lib

DEFINES += GSHARP_LIBRARY

INCLUDEPATH += ../include

SOURCES += gsharp.cpp\
	gsharp_parser.cpp\
	gsharp_program.cpp

HEADERS += gsharp_except.h\
        gsharp_program.h\
        version.h\
        ../include/gsharp.h\
        ../include/gsharp_extra.h

unix {
    target.path = /usr/lib
    INSTALLS += target
    CONFIG   += c++11
}
