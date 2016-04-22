#-------------------------------------------------
#
# Project created by QtCreator 2016-04-22T17:53:48
#
#-------------------------------------------------

QT       -= core gui

TARGET = rserver
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    src/jsmn/jsmn.c \
    src/main.cpp \
    src/RenderServer.cpp \
    src/WebsocketServer.cpp \
    src/jsmn/jsmn_util.cpp

INCLUDEPATH += include

HEADERS += \
    include/jsmn/jsmn.h \
    include/jsmn/jsmn_util.h \
    include/rserver/RenderServer.h \
    include/rserver/WebsocketServer.h

# websocketpp ---
INCLUDEPATH += c:/libs/websocketpp
INCLUDEPATH += c:/libs/asio-1.10.6/include

# libjpeg ---
INCLUDEPATH += c:/libs-install-msvc2013/libjpeg/include
QMAKE_LIBDIR += "c:/libs-install-msvc2013/libjpeg/lib"
LIBS += jpeg.lib


win32
{
	DEFINES += NOMINMAX
}

