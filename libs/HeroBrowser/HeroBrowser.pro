TEMPLATE = lib
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

include(Hero.pri)

QT += webkit opengl network

win32:CONFIG(debug,debug|release) {
  TARGET = HeroBrowserd
}

RCC_DIR     = $$PWD/.rcc
UI_DIR      = $$PWD/.ui
MOC_DIR     = $$PWD/.moc
OBJECTS_DIR = $$PWD/.obj

win32 {
    DEFINES += _WINDOWS
}

# Input
HEADERS += HeroBrowser.h WebKitAdapter.h
SOURCES += HeroBrowser.cpp WebKitAdapter.cpp