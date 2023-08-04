QT += widgets core gui

#TEMPLATE = lib
TEMPLATE = app

DEFINES += WBSIGNALDETECTWIDGET_LIBRARY

CONFIG += c++17

SOURCES += \
    ../src/disturbnoisetableview.cpp \
    ../src/mainwindow.cpp \
    ../src/manmadenoisetableview.cpp \
    ../src/signaldetecttableview.cpp \
    ../src/testmain.cpp \
    ../src/wbsignaldetectmodel.cpp \
    ../src/wbsignaldetectwidget.cpp

HEADERS += \
    ../inc/disturbnoisetableview.h \
    ../inc/mainwindow.h \
    ../inc/manmadenoisetableview.h \
    ../inc/signaldetecttableview.h \
    ../inc/wbsignaldetectmodel.h \
    ../inc/WBSignalDetectWidget_global.h \
    ../inc/wbsignaldetectwidget.h

DESTDIR = ../bin/

INCLUDEPATH += \
    ../inc/  \
    ../tmp/ \
    'C:/Program Files (x86)/Intel/oneAPI/ipp/2021.9.0/include' \
    'C:/Program Files (x86)/Intel/oneAPI/ipp/2021.9.0/lib/intel64'

DEPENDPATH += 'C:/Program Files (x86)/Intel/oneAPI/ipp/2021.9.0/lib/intel64'

LIBS += -L'C:/Program Files (x86)/Intel/oneAPI/ipp/2021.9.0/lib/intel64/' -lippcore -lipps -lippvm
PRE_TARGETDEPS += 'C:/Program Files (x86)/Intel/oneAPI/ipp/2021.9.0/lib/intel64/ippcore.lib'    \
    'C:/Program Files (x86)/Intel/oneAPI/ipp/2021.9.0/lib/intel64/ipps.lib' \
    'C:/Program Files (x86)/Intel/oneAPI/ipp/2021.9.0/lib/intel64/ippvm.lib'

FORMS += \
    ../ui/mainwindow.ui \
    ../ui/wbsignaldetectwidget.ui

