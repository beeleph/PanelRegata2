QT       += core gui
QT       +=  serialbus
QT       += sql
QT       += qml quick

qtConfig(modbus-serialport): QT += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    errorconnectiondialog.cpp \
    main.cpp \
    mainwindow.cpp \
    qledlabel.cpp \
    sample.cpp \
    samplejournal.cpp

HEADERS += \
    errorconnectiondialog.h \
    mainwindow.h \
    qledlabel.h \
    sample.h \
    samplejournal.h

FORMS += \
    errorconnectiondialog.ui \
    mainwindow.ui \
    qledlabel.ui \
    samplejournal.ui

TRANSLATIONS += \
    PanelRegata2_ru_RU.ts \
    PanelRegata2_ru_RU.qm

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
