QT       += widgets testlib
TEMPLATE  = app
TARGET    = tst_calculator

CONFIG   += c++17 release console
CONFIG   -= debug_and_release debug app_bundle

# 直接编译工程源码，保证测的就是真实程序，而不是另一份复制品
INCLUDEPATH += ..
HEADERS     += ../calckey.h \
               ../calculatorengine.h \
               ../mainwindow.h
SOURCES     += tst_calculator.cpp \
               ../calckey.cpp \
               ../calculatorengine.cpp \
               ../mainwindow.cpp
FORMS       += ../mainwindow.ui
RESOURCES   += ../resources.qrc
