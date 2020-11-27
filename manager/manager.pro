
QT  += core gui widgets x11extras

TARGET = ukui-display-switch
TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4)

include(display-switch/display-switch.pri)
include(common/common.pri)

QMAKE_CXXFLAGS += -Wdeprecated-declarations

INCLUDEPATH += \
    common/ \
    display-switch/

SOURCES +=  \
    main.cpp

HEADERS  += 

CONFIG += c++11 debug link_pkgconfig

PKGCONFIG += xrandr x11 xtst

target.path = /usr/sbin/

INSTALLS += target

RESOURCES += \
    image.qrc

DISTFILES += \
    ukui-display-switch.qss
