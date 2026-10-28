QT       += widgets
TEMPLATE  = app
TARGET    = Calculator

CONFIG   += c++17 release
CONFIG   -= debug_and_release debug

HEADERS  += mainwindow.h
SOURCES  += main.cpp \
            mainwindow.cpp
FORMS    += mainwindow.ui
