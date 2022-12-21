QT       += core gui printsupport serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH +=  \
    qcustomplot \
    least_square_method \
    collect_data \
    start_serial \
    D:/eigen-3.3.8 \


SOURCES += \
    about.cpp \
    collect_data/collectpanel.cpp \
    least_square_method/bll_leastssquare.cpp \
    least_square_method/fitchart.cpp \
    main.cpp \
    start_serial/bll_serial.cpp \
    start_serial/bll_serialrecvanalyse.cpp \
    start_serial/startcommunication.cpp\
    mainwindow.cpp \
    qcustomplot/charttracer.cpp \
    qcustomplot/interactchart.cpp \
    qcustomplot/qcustomplot.cpp \

HEADERS += \
    about.h \
    collect_data/collectpanel.h \
    least_square_method/bll_leastssquare.h \
    least_square_method/fitchart.h \
    mainwindow.h \
    qcustomplot/charttracer.h \
    qcustomplot/interactchart.h \
    qcustomplot/qcustomplot.h \
    start_serial/bll_serial.h \
    start_serial/bll_serialrecvanalyse.h \
    start_serial/startcommunication.h

FORMS += \
    about.ui \
    collect_data/collectpanel.ui \
    mainwindow.ui \
    start_serial/startcommunication.ui

TRANSLATIONS += \
    TemperatureSensorCalibration_zh_CN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    main.qrc
