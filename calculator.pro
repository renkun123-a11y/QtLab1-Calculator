QT       += widgets
TEMPLATE  = app
TARGET    = Calculator

CONFIG   += c++17 release
CONFIG   -= debug_and_release debug

HEADERS  += calckey.h \
            calculatorengine.h \
            mainwindow.h
SOURCES  += calckey.cpp \
            calculatorengine.cpp \
            main.cpp \
            mainwindow.cpp
FORMS    += mainwindow.ui
RESOURCES += resources.qrc
