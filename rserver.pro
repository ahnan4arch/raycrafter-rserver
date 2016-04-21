TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
QT += core gui widgets

SOURCES += src/main.cpp \
    src/WebsocketServer.cpp \
    src/jsmn/jsmn.c \
    src/RenderServer.cpp

include(deployment.pri)
qtcAddDeployment()

INCLUDEPATH += include

HEADERS += \
    include/WebsocketServer.h \
    include/jsmn/jsmn.h \
    include/RenderServer.h \
    include/jsmn/jsmn_util.h



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
